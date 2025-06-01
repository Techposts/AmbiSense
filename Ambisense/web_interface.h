#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ESPmDNS.h>

// Global WebServer object
extern WebServer server;

/**
 * Initializes the web server
 */
void setupWebServer();

/**
 * Generate HTML for different pages
 */
String getMainHTML();
String getAdvancedHTML();
String getEffectsHTML();
String getMeshHTML();
String getNetworkHTML();

/**
 * Route handlers for web pages
 */
void handleRoot();
void handleAdvanced();
void handleEffects();
void handleMesh();
void handleNetwork();
void handleSimplePage();

/**
 * API endpoint handlers
 */
void handleDistance();
void handleSettings();
void handleSet();
void handleSetLightMode();
void handleSetDirectionalLight();
void handleSetCenterShift();
void handleSetTrailLength();
void handleSetBackgroundMode();
void handleSetMotionSmoothing();
void handleSetMotionSmoothingParam();
void handleSetEffectSpeed();
void handleSetEffectIntensity();
void handleSetSensorPriorityMode();  // Handler for sensor priority mode
void handleResetDistanceValues();  // Handler for resetting distance values

/**
 * LED testing and configuration handlers
 */
void handleTestLEDs();
void handleReinitLEDs();

/**
 * LED Distribution handlers
 */
void handleSetLEDSegmentMode();
void handleGetLEDSegmentInfo();
void handleSetLEDSegmentInfo();

/**
 * ESP-NOW handlers
 */
void handleGetDeviceInfo();
void handleSetDeviceRole();
void handleScanForSlaves();
void handleAddSlave();
void handleRemoveSlave();
void handleSetMasterMac();
void handleGetSensorData();
void handleDiagnostics();

/**
 * WiFi management handlers
 */
void handleNetworkPost();
void handleResetWifi();
void handleScanNetworks();

#endif // WEB_INTERFACE_H