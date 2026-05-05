#pragma once

/*
 * AmbiSense v6 — auth.
 *
 * Off by default. When a password is set, every /api/... endpoint requires
 * a valid session cookie. Login takes a password, hashes it with PBKDF2-
 * SHA256 (250k rounds), compares against the stored hash, and issues a
 * 32-byte random session token returned as `Set-Cookie: ambisense=...`.
 *
 * Session storage is in-RAM (8 slots, 24 h TTL). Restart wipes sessions —
 * acceptable for a hobbyist device.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AUTH_TOKEN_LEN 32
#define AUTH_TOKEN_HEX_LEN (AUTH_TOKEN_LEN * 2 + 1)

esp_err_t auth_init(void);

/* True if a password is configured (auth required for /api/...). */
bool auth_is_enabled(void);

/* Set the admin password. Empty/NULL clears it (disables auth). */
esp_err_t auth_set_password(const char *plaintext);

/* Verify a plaintext password against the stored hash. */
bool auth_check_password(const char *plaintext);

/* Issue a session token (returned in token_hex_out, NUL-terminated). */
esp_err_t auth_issue_session(char token_hex_out[AUTH_TOKEN_HEX_LEN]);

/* Validate a token string. Returns true if active session exists. */
bool auth_check_session(const char *token_hex);

/* Revoke a single session (logout) or all (factory reset). */
void auth_revoke(const char *token_hex);
void auth_revoke_all(void);

#ifdef __cplusplus
}
#endif
