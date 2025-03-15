#include <WiFi.h>
#include "config.h"
#include "wifi_manager.h"

void setupWiFi() {
  WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
  Serial.println("Wi-Fi Started | Connect to: " + String(WIFI_AP_SSID));
  Serial.println("Visit http://192.168.4.1 to adjust settings");
}