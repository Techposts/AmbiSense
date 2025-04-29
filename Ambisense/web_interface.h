#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

/**
 * Initialize the web server
 */
void setupWebServer();

/**
 * Handle the root page
 */
void handleRoot(AsyncWebServerRequest *request);

/**
 * Handle setting updates
 */
void handleSet(AsyncWebServerRequest *request);

/**
 * Handle distance API endpoint
 */
void handleDistance(AsyncWebServerRequest *request);

/**
 * Handle settings API endpoint
 */
void handleSettings(AsyncWebServerRequest *request);

/**
 * Handle light mode setting
 */
void handleSetLightMode(AsyncWebServerRequest *request);

/**
 * Handle directional light setting
 */
void handleSetDirectionalLight(AsyncWebServerRequest *request);

/**
 * Handle center shift setting
 */
void handleSetCenterShift(AsyncWebServerRequest *request);

/**
 * Handle trail length setting
 */
void handleSetTrailLength(AsyncWebServerRequest *request);

/**
 * Handle background mode setting
 */
void handleSetBackgroundMode(AsyncWebServerRequest *request);

/**
 * Handle network settings page
 */
void handleNetworkSettings(AsyncWebServerRequest *request);

/**
 * Handle network scan API
 */
void handleScanNetworks(AsyncWebServerRequest *request);

/**
 * Handle WiFi reset
 */
void handleResetWifi(AsyncWebServerRequest *request);

/**
 * Handle motion smoothing toggle
 */
void handleSetMotionSmoothing(AsyncWebServerRequest *request);

/**
 * Handle motion smoothing parameter changes
 */
void handleSetMotionSmoothingParam(AsyncWebServerRequest *request);

/**
 * Handle effect speed setting
 */
void handleSetEffectSpeed(AsyncWebServerRequest *request);

/**
 * Handle effect intensity setting
 */
void handleSetEffectIntensity(AsyncWebServerRequest *request);

#endif // WEB_INTERFACE_H