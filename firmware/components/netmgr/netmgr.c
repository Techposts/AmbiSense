#include "netmgr.h"

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mdns.h"

#include "settings.h"

static const char *TAG = "netmgr";

#define STA_RETRY_MAX     3
#define STA_RETRY_BACKOFF_MS  3000
/* Grace period after STA gets an IP before AUTO policy tears the AP
 * interface down. Lets a phone that's still on the captive portal
 * finish polling /api/wifi and read the device's new STA IP / hostname
 * before the AP disappears underneath it. The frontend's polling
 * cadence is 1.5 s so the success modal flips within ~3 s of GOT_IP;
 * 8 s grace gives the user 5+ s to read the URL — and the modal
 * persists in the browser even after the AP drops, so they can
 * continue to copy the URL without a live network. */
#define AP_TEARDOWN_GRACE_US (8ULL * 1000ULL * 1000ULL)

static struct {
    netmgr_state_t state;
    netmgr_state_cb_t cb;
    void *cb_ctx;
    EventGroupHandle_t evt;
    int sta_retry;
    esp_netif_t *sta_netif;
    esp_netif_t *ap_netif;
    char hostname[33];
    bool inited;
    bool dns_running;
    bool ap_active;        /* true while we are broadcasting an SSID */
    bool sta_configured;   /* true if NVS has stored creds */
    netmgr_ap_mode_t ap_mode;
    TaskHandle_t dns_task;
    /* Deferred work — both timers run on the esp_timer task, NOT the
     * Wi-Fi event loop, so they can safely call esp_wifi_connect() and
     * esp_wifi_set_mode() which would deadlock if called from inside an
     * event handler. */
    esp_timer_handle_t sta_retry_timer;   /* fires STA_RETRY_BACKOFF_MS after disconnect */
    esp_timer_handle_t ap_teardown_timer; /* fires AP_TEARDOWN_GRACE_US after STA gets IP */
} s_net;

/* Decide whether the AP interface should be on right now.
 *   AUTO / STA_ONLY: AP up unless STA is currently connected.
 *   ALWAYS:           AP up unconditionally.
 *   No STA configured at all: AP up regardless of mode (otherwise the
 *                             user has no way to reach the device). */
static bool ap_should_be_on(void) {
    if (!s_net.sta_configured) return true;
    if (s_net.ap_mode == NETMGR_AP_ALWAYS) return true;
    return s_net.state != NETMGR_STATE_STA_CONNECTED;
}

/* Forward decls used in transition helpers below */
static esp_err_t configure_ap(void);
static void start_captive_dns(void);
static void stop_captive_dns_now(void);

/* Switch the radio to APSTA / STA_ONLY without restarting Wi-Fi.
 * `target_ap_on` is what we want; we pick the IDF mode accordingly. */
static void apply_ap_state(bool target_ap_on) {
    if (target_ap_on == s_net.ap_active) return;
    wifi_mode_t want = target_ap_on ? WIFI_MODE_APSTA : WIFI_MODE_STA;
    esp_err_t err = esp_wifi_set_mode(want);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "esp_wifi_set_mode(%d) failed: 0x%x", want, err);
        return;
    }
    if (target_ap_on) {
        configure_ap();
        start_captive_dns();
        ESP_LOGI(TAG, "AP brought up (mode=%d)", s_net.ap_mode);
    } else {
        stop_captive_dns_now();
        ESP_LOGI(TAG, "AP brought down (STA owns the radio)");
    }
    s_net.ap_active = target_ap_on;
}

#define EVT_GOT_IP   BIT0
#define EVT_FAIL     BIT1

static void notify_state(netmgr_state_t st) {
    s_net.state = st;
    if (s_net.cb) s_net.cb(st, s_net.cb_ctx);
}

/* Sanitize a free-form name into an mDNS-safe hostname:
 *   lowercase, alnum + hyphen, leading "ambisense-" if not present.
 * Falls back to "ambisense-XXXX" using the last 16 bits of MAC. */
static void sanitize_hostname(const char *in, char *out, size_t max) {
    char buf[33] = {0};
    size_t bi = 0;
    if (in) {
        for (size_t i = 0; in[i] && bi < sizeof(buf) - 1; ++i) {
            char c = in[i];
            if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
            if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') {
                buf[bi++] = c;
            } else if (c == ' ' || c == '.' || c == '_') {
                buf[bi++] = '-';
            }
        }
    }
    if (bi == 0 || strncmp(buf, "ambisense", 9) != 0) {
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_WIFI_STA);
        snprintf(out, max, "ambisense-%02x%02x", mac[4], mac[5]);
    } else {
        snprintf(out, max, "%s", buf);
    }
}

static esp_err_t bring_up_mdns(void) {
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "mdns_init: 0x%x", err);
        return err;
    }
    mdns_hostname_set(s_net.hostname);
    mdns_instance_name_set("AmbiSense");
    mdns_service_add("_ambisense", "_http", "_tcp", 80, NULL, 0);
    ESP_LOGI(TAG, "mDNS up: %s.local", s_net.hostname);
    return ESP_OK;
}

/* Captive-portal DNS responder. Resolves every query to our AP IP so any
 * hostname a phone tries (apple.com/library/test/success.html, msftconnecttest,
 * captive.apple.com, ...) gets steered at the device. iOS / Android / Win11
 * detect this and pop the setup page automatically. */
static void dns_task(void *arg) {
    (void)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { ESP_LOGE(TAG, "dns: socket"); vTaskDelete(NULL); }

    struct sockaddr_in srv = {
        .sin_family = AF_INET,
        .sin_port = htons(53),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(sock, (struct sockaddr *)&srv, sizeof(srv)) < 0) {
        ESP_LOGE(TAG, "dns: bind 53"); close(sock); vTaskDelete(NULL);
    }

    /* Our AP IP. Default IDF AP is 192.168.4.1. We'll fetch live. */
    esp_netif_ip_info_t ip;
    esp_netif_get_ip_info(s_net.ap_netif, &ip);
    uint32_t ap_ip = ip.ip.addr;  /* network-order */

    uint8_t buf[512];
    while (s_net.dns_running) {
        struct sockaddr_in src;
        socklen_t slen = sizeof(src);
        int n = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&src, &slen);
        if (n < 12) continue;
        /* Build a minimal answer: copy the question, set QR=1, AA=1, ANCOUNT=1,
         * append an A-record pointing to ap_ip, TTL 60. */
        buf[2] |= 0x80;     /* QR */
        buf[2] |= 0x04;     /* AA */
        buf[3] = 0x80;      /* RA + RCODE=0 */
        buf[6] = 0; buf[7] = 1;   /* ANCOUNT = 1 */
        buf[8] = 0; buf[9] = 0;   /* NSCOUNT */
        buf[10] = 0; buf[11] = 0; /* ARCOUNT */

        /* Find end of question (NUL-terminated label sequence + 4 bytes type/class). */
        int p = 12;
        while (p < n && buf[p] != 0) p += buf[p] + 1;
        p += 1 + 4;  /* skip null label + qtype + qclass */
        if (p + 16 > (int)sizeof(buf)) continue;

        /* Answer: pointer to question name (0xC00C), TYPE=A, CLASS=IN, TTL=60, RDLEN=4, IP. */
        buf[p++] = 0xC0; buf[p++] = 0x0C;
        buf[p++] = 0x00; buf[p++] = 0x01;
        buf[p++] = 0x00; buf[p++] = 0x01;
        buf[p++] = 0x00; buf[p++] = 0x00; buf[p++] = 0x00; buf[p++] = 0x3C;
        buf[p++] = 0x00; buf[p++] = 0x04;
        buf[p++] = (ap_ip >>  0) & 0xFF;
        buf[p++] = (ap_ip >>  8) & 0xFF;
        buf[p++] = (ap_ip >> 16) & 0xFF;
        buf[p++] = (ap_ip >> 24) & 0xFF;

        sendto(sock, buf, p, 0, (struct sockaddr *)&src, slen);
    }
    close(sock);
    vTaskDelete(NULL);
}

static void start_captive_dns(void) {
    if (s_net.dns_running) return;
    s_net.dns_running = true;
    xTaskCreate(dns_task, "captive_dns", 3072, NULL, 3, &s_net.dns_task);
}

static void stop_captive_dns_now(void) {
    /* The dns_task observes s_net.dns_running and exits at next packet/recv
     * timeout. We don't force-kill the task; it self-terminates. */
    s_net.dns_running = false;
}

/* esp_timer callbacks — these run on the esp_timer task, OUTSIDE the
 * Wi-Fi event loop, so they may safely call esp_wifi_* functions. */
static void sta_retry_timer_cb(void *arg) {
    (void)arg;
    if (s_net.state == NETMGR_STATE_STA_CONNECTING) {
        ESP_LOGI(TAG, "STA retry timer firing");
        esp_wifi_connect();
    }
}

static void ap_teardown_timer_cb(void *arg) {
    (void)arg;
    /* Re-check policy at fire time — STA might have dropped during the
     * grace period, in which case we leave the AP up. */
    if (s_net.state == NETMGR_STATE_STA_CONNECTED && !ap_should_be_on()) {
        ESP_LOGI(TAG, "AP teardown grace period elapsed; powering AP down per policy");
        apply_ap_state(false);
    }
}

static void on_wifi_event(void *arg, esp_event_base_t base, int32_t id, void *data) {
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        bool was_connected = (s_net.state == NETMGR_STATE_STA_CONNECTED);
        s_net.state = NETMGR_STATE_STA_CONNECTING;
        /* Cancel any pending AP teardown — STA isn't connected right now. */
        if (s_net.ap_teardown_timer) esp_timer_stop(s_net.ap_teardown_timer);
        if (was_connected) {
            ESP_LOGW(TAG, "STA dropped after being connected — bringing AP back up while we retry");
            apply_ap_state(ap_should_be_on());
        }
        if (s_net.sta_retry < STA_RETRY_MAX) {
            s_net.sta_retry++;
            ESP_LOGW(TAG, "STA disconnected; retry %d/%d in %d ms",
                     s_net.sta_retry, STA_RETRY_MAX, STA_RETRY_BACKOFF_MS);
            /* Schedule the retry on the esp_timer task — calling
             * vTaskDelay() inside the Wi-Fi event loop blocks every
             * subsequent Wi-Fi event for the duration, which under
             * reconnect storms causes the stack to fall behind. */
            if (s_net.sta_retry_timer) {
                esp_timer_stop(s_net.sta_retry_timer);
                esp_timer_start_once(s_net.sta_retry_timer,
                                     (uint64_t)STA_RETRY_BACKOFF_MS * 1000ULL);
            }
        } else {
            ESP_LOGW(TAG, "STA failed after %d retries; AP fallback active", STA_RETRY_MAX);
            xEventGroupSetBits(s_net.evt, EVT_FAIL);
            s_net.state = NETMGR_STATE_AP_FALLBACK;
            apply_ap_state(true);  /* No matter the mode, fail-soft to AP. */
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "STA got IP: " IPSTR, IP2STR(&e->ip_info.ip));
        s_net.sta_retry = 0;
        s_net.state = NETMGR_STATE_STA_CONNECTED;
        if (s_net.sta_retry_timer) esp_timer_stop(s_net.sta_retry_timer);
        xEventGroupSetBits(s_net.evt, EVT_GOT_IP);
        /* AUTO/STA_ONLY: AP should come down. But don't tear it down
         * immediately — a phone that's still on the captive portal
         * needs a polling window to read /api/wifi and learn the new
         * STA IP / hostname. Schedule the teardown 30 s out. */
        if (s_net.ap_active && !ap_should_be_on() && s_net.ap_teardown_timer) {
            esp_timer_stop(s_net.ap_teardown_timer);
            esp_timer_start_once(s_net.ap_teardown_timer, AP_TEARDOWN_GRACE_US);
            ESP_LOGI(TAG, "AP teardown scheduled in 8 s (captive-portal grace window)");
        }
    } else if (base == WIFI_EVENT && id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t *e = (wifi_event_ap_staconnected_t *)data;
        ESP_LOGI(TAG, "AP client joined: " MACSTR, MAC2STR(e->mac));
    }
}

/* Configure the AP interface. AP visibility is governed by the
 * netmgr_ap_mode_t policy (AUTO / ALWAYS / STA_ONLY). For first-setup
 * the AP starts open so the captive portal pops the setup page; the
 * user can lock it down via /api/wifi { ap_password: "..." }. */
static esp_err_t configure_ap(void) {
    char ap_ssid[32];
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(ap_ssid, sizeof(ap_ssid), "AmbiSense-%02X%02X", mac[4], mac[5]);

    /* Optional WPA2 password from NVS; default is open for first-setup. */
    char ap_pass[64] = {0};
    settings_get_str("wifi", "ap_pass", ap_pass, sizeof(ap_pass));

    /* AP channel: NVS override, otherwise default 6. */
    uint8_t channel = 6;
    uint8_t saved_ch = 0;
    if (settings_get_u8("wifi", "ap_ch", &saved_ch) == ESP_OK && saved_ch >= 1 && saved_ch <= 13) {
        channel = saved_ch;
    }

    wifi_config_t cfg = {0};
    snprintf((char *)cfg.ap.ssid, sizeof(cfg.ap.ssid), "%s", ap_ssid);
    cfg.ap.ssid_len = strlen(ap_ssid);
    cfg.ap.channel = channel;
    cfg.ap.max_connection = 4;  /* a phone or two; we don't need more */
    if (ap_pass[0] && strlen(ap_pass) >= 8) {
        snprintf((char *)cfg.ap.password, sizeof(cfg.ap.password), "%s", ap_pass);
        cfg.ap.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        cfg.ap.authmode = WIFI_AUTH_OPEN;
    }
    cfg.ap.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &cfg));
    ESP_LOGI(TAG, "AP configured: SSID=%s%s channel=%u", ap_ssid,
             cfg.ap.authmode == WIFI_AUTH_OPEN ? " (open)" : " (wpa2)", channel);
    return ESP_OK;
}

static esp_err_t configure_sta(const char *ssid, const char *pass) {
    wifi_config_t cfg = {0};
    snprintf((char *)cfg.sta.ssid, sizeof(cfg.sta.ssid), "%s", ssid);
    if (pass && pass[0]) snprintf((char *)cfg.sta.password, sizeof(cfg.sta.password), "%s", pass);
    cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    /* WIFI_FAST_SCAN relies on a cached BSSID/channel from a previous
     * successful connect. On a fresh device or after credentials change
     * the cache is empty and FAST_SCAN can give up before finding the
     * SSID — manifests to users as "first attempt fails, second
     * succeeds". ALL_CHANNEL_SCAN takes ~3 s extra but is reliable on
     * the very first try, which is the only attempt that matters during
     * onboarding. */
    cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    cfg.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    cfg.sta.pmf_cfg.capable = true;
    return esp_wifi_set_config(WIFI_IF_STA, &cfg);
}

esp_err_t netmgr_init(void) {
    if (s_net.inited) return ESP_OK;

    s_net.evt = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_net.sta_netif = esp_netif_create_default_wifi_sta();
    s_net.ap_netif  = esp_netif_create_default_wifi_ap();

    /* Hostname: NVS wifi.host > derived from MAC. */
    char raw[33] = {0};
    settings_get_hostname(raw, sizeof(raw));
    sanitize_hostname(raw, s_net.hostname, sizeof(s_net.hostname));
    esp_netif_set_hostname(s_net.sta_netif, s_net.hostname);

    wifi_init_config_t wcfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wcfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &on_wifi_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_event, NULL));

    /* Create the deferred-work timers. They get armed/disarmed from the
     * Wi-Fi event handler; their callbacks run on the esp_timer task,
     * keeping the event loop unblocked. */
    const esp_timer_create_args_t retry_args = {
        .callback = sta_retry_timer_cb, .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK, .name = "sta_retry",
    };
    ESP_ERROR_CHECK(esp_timer_create(&retry_args, &s_net.sta_retry_timer));
    const esp_timer_create_args_t teardown_args = {
        .callback = ap_teardown_timer_cb, .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK, .name = "ap_teardown",
    };
    ESP_ERROR_CHECK(esp_timer_create(&teardown_args, &s_net.ap_teardown_timer));

    /* Read AP-mode policy from NVS; default AUTO (AP only when STA is
     * down, or always when no STA configured). */
    uint8_t apmode = NETMGR_AP_AUTO;
    settings_get_u8("wifi", "ap_mode", &apmode);
    if (apmode > NETMGR_AP_STA_ONLY) apmode = NETMGR_AP_AUTO;
    s_net.ap_mode = (netmgr_ap_mode_t)apmode;

    char ssid[33] = {0}, pass[65] = {0};
    settings_get_wifi_ssid(ssid, sizeof(ssid));
    settings_get_wifi_pass(pass, sizeof(pass));
    s_net.sta_configured = ssid[0] != 0;

    /* Decide initial mode. If we have STA creds, start in APSTA so STA
     * can come up while AP is reachable; the AP will be torn down by
     * the IP_EVENT_STA_GOT_IP handler if policy allows. If no creds,
     * AP-only is the right answer. */
    bool ap_at_boot = ap_should_be_on();
    ESP_ERROR_CHECK(esp_wifi_set_mode(ap_at_boot ? WIFI_MODE_APSTA : WIFI_MODE_STA));
    if (ap_at_boot) configure_ap();

    if (s_net.sta_configured) {
        configure_sta(ssid, pass);
        s_net.sta_retry = 0;
        xEventGroupClearBits(s_net.evt, EVT_GOT_IP | EVT_FAIL);
        notify_state(NETMGR_STATE_STA_CONNECTING);
    } else {
        notify_state(NETMGR_STATE_AP_FALLBACK);
    }

    ESP_ERROR_CHECK(esp_wifi_start());
    s_net.ap_active = ap_at_boot;
    ESP_LOGI(TAG, "Wi-Fi up: ap=%s sta=%s host=%s mode=%s",
             ap_at_boot ? "yes" : "no",
             s_net.sta_configured ? ssid : "(none)",
             s_net.hostname,
             s_net.ap_mode == NETMGR_AP_ALWAYS ? "always" :
             s_net.ap_mode == NETMGR_AP_STA_ONLY ? "sta_only" : "auto");

    if (ap_at_boot) start_captive_dns();
    bring_up_mdns();

    /* If we tried STA, wait briefly so callers see a settled state. */
    if (s_net.sta_configured) {
        EventBits_t bits = xEventGroupWaitBits(
            s_net.evt, EVT_GOT_IP | EVT_FAIL, pdFALSE, pdFALSE,
            pdMS_TO_TICKS(15000));
        if (bits & EVT_GOT_IP) {
            /* event handler already adjusted AP state per policy */
        } else {
            notify_state(NETMGR_STATE_AP_FALLBACK);
            apply_ap_state(true);  /* AP is the user's only way back in */
        }
    }

    s_net.inited = true;
    return ESP_OK;
}

netmgr_ap_mode_t netmgr_get_ap_mode(void) {
    return s_net.ap_mode;
}

esp_err_t netmgr_set_ap_mode(netmgr_ap_mode_t mode) {
    if (mode > NETMGR_AP_STA_ONLY) return ESP_ERR_INVALID_ARG;
    s_net.ap_mode = mode;
    settings_set_u8("wifi", "ap_mode", (uint8_t)mode);
    /* Apply immediately so the user sees the effect without a reboot. */
    apply_ap_state(ap_should_be_on());
    ESP_LOGI(TAG, "AP mode set to %s", mode == NETMGR_AP_ALWAYS ? "always" :
                                       mode == NETMGR_AP_STA_ONLY ? "sta_only" : "auto");
    return ESP_OK;
}

bool netmgr_is_ap_active(void) {
    return s_net.ap_active;
}

esp_err_t netmgr_set_ap_password(const char *pass) {
    if (!pass) pass = "";
    settings_set_str("wifi", "ap_pass", pass);
    /* If AP is currently up, re-apply config so new password takes effect. */
    if (s_net.ap_active) {
        configure_ap();
        ESP_LOGI(TAG, "AP password updated; re-applied to running AP");
    }
    return ESP_OK;
}

esp_err_t netmgr_set_credentials(const char *ssid, const char *pass) {
    if (!ssid || !ssid[0]) {
        settings_set_wifi_ssid("");
        settings_set_wifi_pass("");
        /* CRITICAL: clear in-memory flag too, otherwise ap_should_be_on()
         * still thinks we're configured and keeps the AP up incorrectly. */
        s_net.sta_configured = false;
        esp_wifi_disconnect();
        notify_state(NETMGR_STATE_AP_FALLBACK);
        ESP_LOGI(TAG, "Cleared STA creds; AP remains up");
        return ESP_OK;
    }
    settings_set_wifi_ssid(ssid);
    settings_set_wifi_pass(pass ? pass : "");
    /* CRITICAL: mark STA configured BEFORE the connection attempt. The
     * IP_EVENT_STA_GOT_IP handler reads ap_should_be_on() which depends
     * on this flag — without setting it here, fresh-NVS onboarding
     * leaves the AP up forever even after STA succeeds, because
     * ap_should_be_on() returns true unconditionally when no STA is
     * configured. (Reproduced 2026-05-07 on a freshly-erased C3.) */
    s_net.sta_configured = true;

    /* AP keeps running throughout. Just retarget STA. */
    esp_wifi_disconnect();
    s_net.sta_retry = 0;
    xEventGroupClearBits(s_net.evt, EVT_GOT_IP | EVT_FAIL);
    configure_sta(ssid, pass);
    notify_state(NETMGR_STATE_STA_CONNECTING);
    esp_wifi_connect();

    EventBits_t bits = xEventGroupWaitBits(
        s_net.evt, EVT_GOT_IP | EVT_FAIL, pdFALSE, pdFALSE,
        pdMS_TO_TICKS(15000));
    if (bits & EVT_GOT_IP) {
        notify_state(NETMGR_STATE_STA_CONNECTED);
        return ESP_OK;
    }
    notify_state(NETMGR_STATE_AP_FALLBACK);
    return ESP_FAIL;
}

bool netmgr_is_sta_connected(void) {
    return s_net.state == NETMGR_STATE_STA_CONNECTED;
}

esp_err_t netmgr_get_ip(char *out, size_t max) {
    if (!out || max == 0) return ESP_ERR_INVALID_ARG;
    esp_netif_ip_info_t ip;
    esp_netif_t *if_ = (s_net.state == NETMGR_STATE_STA_CONNECTED) ? s_net.sta_netif : s_net.ap_netif;
    esp_err_t err = esp_netif_get_ip_info(if_, &ip);
    if (err != ESP_OK) return err;
    snprintf(out, max, IPSTR, IP2STR(&ip.ip));
    return ESP_OK;
}

esp_err_t netmgr_get_hostname(char *out, size_t max) {
    if (!out || max == 0) return ESP_ERR_INVALID_ARG;
    snprintf(out, max, "%s", s_net.hostname);
    return ESP_OK;
}

int8_t netmgr_get_rssi(void) {
    if (!netmgr_is_sta_connected()) return 0;
    wifi_ap_record_t info = {0};
    if (esp_wifi_sta_get_ap_info(&info) != ESP_OK) return 0;
    return info.rssi;
}

void netmgr_on_state_change(netmgr_state_cb_t cb, void *ctx) {
    s_net.cb = cb;
    s_net.cb_ctx = ctx;
}

esp_err_t netmgr_set_hostname(const char *name) {
    if (!name) return ESP_ERR_INVALID_ARG;
    sanitize_hostname(name, s_net.hostname, sizeof(s_net.hostname));
    settings_set_hostname(s_net.hostname);
    if (s_net.sta_netif) esp_netif_set_hostname(s_net.sta_netif, s_net.hostname);
    mdns_hostname_set(s_net.hostname);
    return ESP_OK;
}
