#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H
#include <ESPmDNS.h>



/**
 * Initialize the web server
 */
void setupWebServer();

/**
 * Process web server requests
 * Should be called regularly in the main loop
 */
void handleWebServer();

/**
 * Handle the root page
 */
void handleRoot();

/**
 * Handle setting updates
 */
void handleSet();

/**
 * Handle distance API endpoint
 */
void handleDistance();

/**
 * Handle settings API endpoint
 */
void handleSettings();

/**
 * Handle light mode setting
 */
void handleSetLightMode();

/**
 * Handle directional light setting
 */
void handleSetDirectionalLight();

/**
 * Handle center shift setting
 */
void handleSetCenterShift();

/**
 * Handle trail length setting
 */
void handleSetTrailLength();

/**
 * Handle background mode setting
 */
void handleSetBackgroundMode();

/**
 * Handle network settings page
 */
void handleNetworkSettings();

/**
 * Handle network scan API
 */
void handleScanNetworks();

/**
 * Handle WiFi reset
 */
void handleResetWifi();

/**
 * Handle motion smoothing toggle
 */
void handleSetMotionSmoothing();

/**
 * Handle motion smoothing parameter changes
 */
void handleSetMotionSmoothingParam();


void handleSetEffectSpeed();
void handleSetEffectIntensity();

#endif // WEB_INTERFACE_H