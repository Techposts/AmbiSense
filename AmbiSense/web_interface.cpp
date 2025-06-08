#include <WebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>
#include <EEPROM.h>

#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "wifi_manager.h"
#include "espnow_manager.h"

// Create WebServer object on port 80
WebServer server(WEB_SERVER_PORT);

// Full-featured compressed HTML templates stored in PROGMEM to save RAM
#include "compressed_html_full.h"

// Function to generate main page HTML
String getMainHTML() {
  return buildFullPage(basic_tab_full, "basic", FPSTR(basic_tab_js));
}

// Function to generate advanced settings HTML
String getAdvancedHTML() {
  return buildFullPage(advanced_tab_full, "advanced", nullptr);
}

// Function to generate effects settings HTML
String getEffectsHTML() {
  return buildFullPage(effects_tab_full, "effects", nullptr);
}

// Function to generate mesh settings HTML
String getMeshHTML() {
  return buildFullPage(mesh_tab_full, "mesh", FPSTR(mesh_tab_js));
}
void handleDiagnostics() {
  // Real-time diagnostics page with live updates
  String html = R"(<!DOCTYPE html>
<html><head>
<meta charset="UTF-8">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>AmbiSense Real-Time Diagnostics</title>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<style>
:root{--primary:#4361ee;--success:#4cc9f0;--bg-dark:#121212;--bg-card:#1e1e1e;--text:#ffffff;--text-secondary:#b0b0b0}
body{font-family:'Segoe UI',Tahoma,Geneva,Verdana,sans-serif;background:var(--bg-dark);color:var(--text);padding:20px;margin:0}
.container{max-width:800px;margin:0 auto;background:var(--bg-card);padding:25px;border-radius:12px;box-shadow:0 10px 20px rgba(0,0,0,0.3)}
.header{background:linear-gradient(135deg,var(--primary) 0%,#4895ef 100%);padding:20px;margin:-25px -25px 25px -25px;border-radius:12px 12px 0 0;text-align:center}
.header h1{margin:0;font-size:24px;text-shadow:0 2px 5px rgba(0,0,0,0.2)}
.menu{margin:20px 0;text-align:center}
.menu a{color:var(--primary);margin:0 15px;text-decoration:none;padding:8px 16px;border-radius:6px;background:#333;transition:all 0.3s ease}
.menu a:hover{background:var(--primary);color:white}
.status-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px;margin:20px 0}
.status{background:#333;padding:20px;border-radius:10px;border-left:4px solid var(--primary)}
.status h3{margin:0 0 15px 0;color:var(--primary);font-size:16px}
.metric{display:flex;justify-content:space-between;align-items:center;margin:10px 0;padding:8px 0;border-bottom:1px solid #444}
.metric:last-child{border-bottom:none}
.metric-label{color:var(--text-secondary);font-size:14px}
.metric-value{color:var(--success);font-weight:600;font-size:16px}
.live-indicator{width:8px;height:8px;border-radius:50%;background:var(--success);box-shadow:0 0 6px var(--success);animation:pulse 2s infinite}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:0.5}}
.chart-container{background:#333;padding:20px;border-radius:10px;margin:20px 0;height:200px;position:relative}
.chart{width:100%;height:100%;background:linear-gradient(90deg,transparent 0%,rgba(67,97,238,0.1) 50%,transparent 100%);border-radius:5px;position:relative;overflow:hidden}
.distance-bar{position:absolute;bottom:0;width:4px;background:var(--success);border-radius:2px;transition:height 0.3s ease}
</style>
</head><body>
<div class='container'>
<div class='header'>
<h1>🔬 Real-Time System Diagnostics</h1>
<div style='display:flex;align-items:center;justify-content:center;margin-top:10px'>
<div class='live-indicator'></div>
<span style='margin-left:8px;font-size:14px'>Live Data</span>
</div>
</div>

<div class='menu'>
<a href='/'>🏠 Home</a>
<a href='/mesh'>📡 Multi-Sensor</a>
<a href='/advanced'>⚙️ Advanced</a>
</div>

<div class='status-grid'>
<div class='status'>
<h3>📊 System Status</h3>
<div class='metric'>
<span class='metric-label'>Device Role</span>
<span class='metric-value' id='deviceRole'>)" + String(deviceRole == DEVICE_ROLE_MASTER ? "Master" : "Slave") + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>Current Distance</span>
<span class='metric-value' id='currentDistance'>)" + String(currentDistance) + R"( cm</span>
</div>
<div class='metric'>
<span class='metric-label'>LED Count</span>
<span class='metric-value' id='ledCount'>)" + String(numLeds) + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>Light Mode</span>
<span class='metric-value' id='lightMode'>)" + String(lightMode) + R"(</span>
</div>
</div>

<div class='status'>
<h3>💾 Memory & Performance</h3>
<div class='metric'>
<span class='metric-label'>Free Heap</span>
<span class='metric-value' id='freeHeap'>)" + String(ESP.getFreeHeap()) + R"( bytes</span>
</div>
<div class='metric'>
<span class='metric-label'>Total Heap</span>
<span class='metric-value'>)" + String(ESP.getHeapSize()) + R"( bytes</span>
</div>
<div class='metric'>
<span class='metric-label'>Uptime</span>
<span class='metric-value' id='uptime'>)" + String(millis() / 1000) + R"( sec</span>
</div>
<div class='metric'>
<span class='metric-label'>WiFi RSSI</span>
<span class='metric-value' id='wifiRssi'>)" + String(WiFi.RSSI()) + R"( dBm</span>
</div>
</div>

<div class='status'>
<h3>🎯 Motion Settings</h3>
<div class='metric'>
<span class='metric-label'>Min Distance</span>
<span class='metric-value' id='minDistance'>)" + String(minDistance) + R"( cm</span>
</div>
<div class='metric'>
<span class='metric-label'>Max Distance</span>
<span class='metric-value' id='maxDistance'>)" + String(maxDistance) + R"( cm</span>
</div>
<div class='metric'>
<span class='metric-label'>Brightness</span>
<span class='metric-value' id='brightness'>)" + String(brightness) + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>Motion Smoothing</span>
<span class='metric-value' id='motionSmoothing'>)" + String(motionSmoothingEnabled ? "Enabled" : "Disabled") + R"(</span>
</div>
</div>

<div class='status'>
<h3>📡 Network Info</h3>
<div class='metric'>
<span class='metric-label'>IP Address</span>
<span class='metric-value' id='ipAddress'>)" + WiFi.localIP().toString() + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>MAC Address</span>
<span class='metric-value'>)" + WiFi.macAddress() + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>Connected Slaves</span>
<span class='metric-value' id='slaveCount'>)" + String(numSlaveDevices) + R"(</span>
</div>
<div class='metric'>
<span class='metric-label'>ESP-NOW Status</span>
<span class='metric-value' id='espnowStatus'>Active</span>
</div>
</div>
</div>

<div class='chart-container'>
<h3 style='margin:0 0 15px 0;color:var(--primary)'>📈 Real-Time Distance Chart</h3>
<div class='chart' id='distanceChart'>
<div style='position:absolute;top:10px;left:10px;font-size:12px;color:var(--text-secondary)'>Distance: <span id='chartDistance'>--</span> cm</div>
</div>
</div>

</div>

<script>
function updateDiagnostics() {
  // Fetch all diagnostic data in one call for better performance
  fetch('/diagnosticdata')
    .then(r => r.json())
    .then(data => {
      // Update real-time values
      document.getElementById('currentDistance').textContent = data.currentDistance + ' cm';
      document.getElementById('chartDistance').textContent = data.currentDistance;
      document.getElementById('freeHeap').textContent = data.freeHeap.toLocaleString() + ' bytes';
      document.getElementById('uptime').textContent = data.uptime + ' sec';
      document.getElementById('wifiRssi').textContent = data.wifiRssi + ' dBm';
      
      // Update system settings
      document.getElementById('ledCount').textContent = data.numLeds;
      document.getElementById('lightMode').textContent = getLightModeName(data.lightMode);
      document.getElementById('minDistance').textContent = data.minDistance + ' cm';
      document.getElementById('maxDistance').textContent = data.maxDistance + ' cm';
      document.getElementById('brightness').textContent = data.brightness;
      document.getElementById('motionSmoothing').textContent = data.motionSmoothing ? 'Enabled' : 'Disabled';
      
      // Update device role and slave count
      const roleText = data.deviceRole == 1 ? 'Master' : data.deviceRole == 2 ? 'Slave' : 'Standalone';
      document.getElementById('deviceRole').textContent = roleText;
      document.getElementById('slaveCount').textContent = data.numSlaves;
      
      // Update distance chart
      updateDistanceChart(parseInt(data.currentDistance));
    })
    .catch(e => {
      console.error('Error fetching diagnostic data:', e);
      // Fallback to individual endpoints if batch fails
      fetch('/distance').then(r => r.text()).then(d => {
        document.getElementById('currentDistance').textContent = d + ' cm';
        updateDistanceChart(parseInt(d));
      });
    });
  
  // Schedule next update
  setTimeout(updateDiagnostics, 1000);
}

function getLightModeName(mode) {
  const modes = ['Standard', 'Rainbow', 'Color Wave', 'Breathing', 'Solid', 'Comet', 'Pulse', 'Fire', 'Theater Chase', 'Dual Scan', 'Motion Particles'];
  return modes[mode] || 'Unknown';
}

let distanceHistory = [];
function updateDistanceChart(distance) {
  distanceHistory.push(distance);
  if (distanceHistory.length > 50) distanceHistory.shift();
  
  const chart = document.getElementById('distanceChart');
  const maxDistance = Math.max(...distanceHistory) || 300;
  
  // Clear existing bars
  const existingBars = chart.querySelectorAll('.distance-bar');
  existingBars.forEach(bar => bar.remove());
  
  // Add new bars
  distanceHistory.forEach((dist, index) => {
    const bar = document.createElement('div');
    bar.className = 'distance-bar';
    bar.style.left = (index * 4) + 'px';
    bar.style.height = ((dist / maxDistance) * 160) + 'px';
    chart.appendChild(bar);
  });
}

// Start updates when page loads
document.addEventListener('DOMContentLoaded', updateDiagnostics);
</script>
</body></html>)";
  
  server.send(200, "text/html; charset=utf-8", html);
}

void handleDiagnosticData() {
  // Real-time diagnostic data endpoint
  String json = "{";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"totalHeap\":" + String(ESP.getHeapSize()) + ",";
  json += "\"uptime\":" + String(millis() / 1000) + ",";
  json += "\"wifiRssi\":" + String(WiFi.RSSI()) + ",";
  json += "\"currentDistance\":" + String(currentDistance) + ",";
  json += "\"deviceRole\":" + String(deviceRole) + ",";
  json += "\"numSlaves\":" + String(numSlaveDevices) + ",";
  json += "\"lightMode\":" + String(lightMode) + ",";
  json += "\"brightness\":" + String(brightness) + ",";
  json += "\"numLeds\":" + String(numLeds) + ",";
  json += "\"motionSmoothing\":" + String(motionSmoothingEnabled ? "true" : "false") + ",";
  json += "\"minDistance\":" + String(minDistance) + ",";
  json += "\"maxDistance\":" + String(maxDistance);
  json += "}";
  
  server.send(200, "application/json; charset=utf-8", json);
}

// Function to generate network HTML page
String getNetworkHTML() {
  return buildFullPage(network_tab_full, "network", FPSTR(network_tab_js));
}

void setupWebServer() {
  // Wait a moment to ensure WiFi is fully initialized
  delay(100);
  
  // Define handlers for each endpoint
  server.on("/", HTTP_GET, handleRoot);
  server.on("/advanced", HTTP_GET, handleAdvanced);
  server.on("/effects", HTTP_GET, handleEffects);
  server.on("/mesh", HTTP_GET, handleMesh);
  
  // Use simplified network handlers
  server.on("/network", HTTP_GET, handleNetwork);
  server.on("/network", HTTP_POST, handleNetworkPost);
  
  server.on("/set", HTTP_GET, handleSet);
  server.on("/distance", HTTP_GET, handleDistance);
  server.on("/settings", HTTP_GET, handleSettings);
  
  server.on("/setLightMode", HTTP_GET, handleSetLightMode);
  server.on("/setDirectionalLight", HTTP_GET, handleSetDirectionalLight);
  server.on("/setCenterShift", HTTP_GET, handleSetCenterShift);
  server.on("/setTrailLength", HTTP_GET, handleSetTrailLength);
  server.on("/setBackgroundMode", HTTP_GET, handleSetBackgroundMode);
  server.on("/setMotionSmoothing", HTTP_GET, handleSetMotionSmoothing);
  server.on("/setMotionSmoothingParam", HTTP_GET, handleSetMotionSmoothingParam);
  server.on("/setEffectSpeed", HTTP_GET, handleSetEffectSpeed);
  server.on("/setEffectIntensity", HTTP_GET, handleSetEffectIntensity);
  server.on("/setSensorPriorityMode", HTTP_GET, handleSetSensorPriorityMode);
  
  // LED-specific endpoints
  server.on("/testLEDs", HTTP_GET, handleTestLEDs);
  server.on("/reinitLEDs", HTTP_GET, handleReinitLEDs);
  
  // LED Distribution endpoints
  server.on("/setLEDSegmentMode", HTTP_GET, handleSetLEDSegmentMode);
  server.on("/getLEDSegmentInfo", HTTP_GET, handleGetLEDSegmentInfo);
  server.on("/setLEDSegmentInfo", HTTP_GET, handleSetLEDSegmentInfo);
  
  // ESP-NOW mesh configuration endpoints
  server.on("/getDeviceInfo", HTTP_GET, handleGetDeviceInfo);
  server.on("/setDeviceRole", HTTP_GET, handleSetDeviceRole);
  server.on("/scanForSlaves", HTTP_GET, handleScanForSlaves);
  server.on("/addSlave", HTTP_GET, handleAddSlave);
  server.on("/removeSlave", HTTP_GET, handleRemoveSlave);
  server.on("/setMasterMac", HTTP_GET, handleSetMasterMac);
  server.on("/sensordata", HTTP_GET, handleGetSensorData);
  server.on("/diagnostics", HTTP_GET, handleDiagnostics);
  server.on("/diagnosticdata", HTTP_GET, handleDiagnosticData);
  server.on("/resetdistance", HTTP_GET, handleResetDistanceValues);

  // WiFi management
  server.on("/resetwifi", HTTP_GET, handleResetWifi);
  server.on("/scannetworks", HTTP_GET, handleScanNetworks);
  
  // Serve static files from SPIFFS
  server.serveStatic("/", SPIFFS, "/");
  
  // 404 handler
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not found");
  });
  
  // Start server
  server.begin();
  Serial.println("HTTP server started on port " + String(WEB_SERVER_PORT));
}
void handleSetSensorPriorityMode() {
  if (server.hasArg("mode")) {
    uint8_t mode = server.arg("mode").toInt();
    if (mode <= SENSOR_PRIORITY_ZONE_BASED) {
      setSensorPriorityMode(mode);
      
      server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Sensor priority mode updated\"}");
    } else {
      server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Invalid mode parameter\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing mode parameter\"}");
  }
}

// LED Distribution handlers
void handleSetLEDSegmentMode() {
  if (server.hasArg("mode")) {
    int mode = server.arg("mode").toInt();
    if (mode == LED_SEGMENT_MODE_CONTINUOUS || mode == LED_SEGMENT_MODE_DISTRIBUTED) {
      setLEDSegmentMode(mode);
      server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"LED segment mode updated\"}");
    } else {
      server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Invalid mode parameter\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing mode parameter\"}");
  }
}

void handleGetLEDSegmentInfo() {
  int start, length, total;
  getLEDSegmentInfo(&start, &length, &total);
  
  String json = "{";
  json += "\"mode\":" + String(getLEDSegmentMode()) + ",";
  json += "\"start\":" + String(start) + ",";
  json += "\"length\":" + String(length) + ",";
  json += "\"total\":" + String(total);
  json += "}";
  
  server.send(200, "application/json; charset=utf-8", json);
}

void handleSetLEDSegmentInfo() {
  if (server.hasArg("start") && server.hasArg("length") && server.hasArg("total")) {
    int start = server.arg("start").toInt();
    int length = server.arg("length").toInt();
    int total = server.arg("total").toInt();
    
    // Validate parameters
    if (start >= 0 && length > 0 && total > 0 && start + length <= total) {
      setLEDSegmentInfo(start, length, total);
      saveLEDDistributionSettings();
      
      server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"LED segment info updated\"}");
    } else {
      server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Invalid segment parameters\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing required parameters\"}");
  }
}
// Add this to web_interface.cpp
void handleResetDistanceValues() {
  // Reset just min/max distance settings to defaults
  minDistance = DEFAULT_MIN_DISTANCE;
  maxDistance = DEFAULT_MAX_DISTANCE;
  
  // Save to EEPROM
  EEPROM.write(EEPROM_ADDR_MIN_DIST_L, minDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MIN_DIST_H, (minDistance >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAX_DIST_L, maxDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAX_DIST_H, (maxDistance >> 8) & 0xFF);
  
  // Calculate and save new CRC
  uint8_t newCRC = calculateSystemCRC(); // Use function from eeprom_manager
  EEPROM.write(EEPROM_ADDR_CRC, newCRC);
  
  EEPROM.commit();
  
  // Inform user
  String html = "<html><body style='font-family:Arial;text-align:center;'>"
                "<h1>Distance Settings Reset</h1>"
                "<p>Min Distance: " + String(minDistance) + " cm</p>"
                "<p>Max Distance: " + String(maxDistance) + " cm</p>"
                "<p><a href='/'>Return to main page</a></p>"
                "</body></html>";
  
  server.send(200, "text/html; charset=utf-8", html);
}


// Individual route handlers
void handleRoot() {
  String html = getMainHTML();
  server.sendHeader("Cache-Control", "max-age=300"); // Cache for 5 minutes
  server.sendHeader("Content-Length", String(html.length()));
  server.send(200, "text/html; charset=utf-8", html);
}

void handleAdvanced() {
  String html = getAdvancedHTML();
  server.sendHeader("Cache-Control", "max-age=300");
  server.sendHeader("Content-Length", String(html.length()));
  server.send(200, "text/html; charset=utf-8", html);
}

void handleEffects() {
  String html = getEffectsHTML();
  server.sendHeader("Cache-Control", "max-age=300");
  server.sendHeader("Content-Length", String(html.length()));
  server.send(200, "text/html; charset=utf-8", html);
}

void handleMesh() {
  String html = getMeshHTML();
  server.sendHeader("Cache-Control", "max-age=300");
  server.sendHeader("Content-Length", String(html.length()));
  server.send(200, "text/html; charset=utf-8", html);
}

void handleNetwork() {
  String html = getNetworkHTML();
  server.sendHeader("Cache-Control", "max-age=300");
  server.sendHeader("Content-Length", String(html.length()));
  server.send(200, "text/html; charset=utf-8", html);
}


void handleDistance() {
  server.send(200, "text/plain", String(currentDistance));
}

void handleSettings() {
  // Create JSON with current settings
  String json = "{";
  json += "\"minDistance\":" + String(minDistance) + ",";
  json += "\"maxDistance\":" + String(maxDistance) + ",";
  json += "\"brightness\":" + String(brightness) + ",";
  json += "\"movingLightSpan\":" + String(movingLightSpan) + ",";
  json += "\"redValue\":" + String(redValue) + ",";
  json += "\"greenValue\":" + String(greenValue) + ",";
  json += "\"blueValue\":" + String(blueValue) + ",";
  json += "\"numLeds\":" + String(numLeds) + ",";
  json += "\"centerShift\":" + String(centerShift) + ",";
  json += "\"trailLength\":" + String(trailLength) + ",";
  json += "\"directionLightEnabled\":" + String(directionLightEnabled ? "true" : "false") + ",";
  json += "\"backgroundMode\":" + String(backgroundMode ? "true" : "false") + ",";
  json += "\"lightMode\":" + String(lightMode) + ",";
  json += "\"motionSmoothingEnabled\":" + String(motionSmoothingEnabled ? "true" : "false") + ",";
  json += "\"effectSpeed\":" + String(effectSpeed) + ",";
  json += "\"effectIntensity\":" + String(effectIntensity) + ",";
  json += "\"positionSmoothingFactor\":" + String(positionSmoothingFactor, 3) + ",";
  json += "\"velocitySmoothingFactor\":" + String(velocitySmoothingFactor, 3) + ",";
  json += "\"predictionFactor\":" + String(predictionFactor, 3) + ",";
  json += "\"positionPGain\":" + String(positionPGain, 3) + ",";
  json += "\"positionIGain\":" + String(positionIGain, 3) + ",";
  json += "\"sensorPriorityMode\":" + String(sensorPriorityMode);
  json += "}";
  
  server.send(200, "application/json; charset=utf-8", json);
}

void handleSet() {
  bool settingsChanged = false;
  String response = "{\"status\":\"success\", \"changes\":[";
  bool firstChange = true;
  String errorMessage = "";

  if (server.hasArg("numLeds")) {
    int newNumLeds = server.arg("numLeds").toInt();
    
    // Enhanced validation for LED count
    if (!validateLEDCount(newNumLeds)) {
      errorMessage = "Invalid LED count: " + String(newNumLeds) + " (max: " + String(MAX_SUPPORTED_LEDS) + ")";
    } else if (numLeds != newNumLeds) {
      Serial.printf("Web UI: Changing LED count from %d to %d\n", numLeds, newNumLeds);
      
      // Force reinitialization of LED strip
      reinitializeLEDStrip(newNumLeds);
      
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"numLeds\"";
      firstChange = false;
      
      Serial.printf("Web UI: LED count successfully changed to %d\n", numLeds);
    }
  }
  
  if (server.hasArg("minDist")) {
    int newMinDist = server.arg("minDist").toInt();
    // Ensure newMinDist is in valid range and less than maxDistance
    if (newMinDist >= 0 && newMinDist < maxDistance && newMinDist <= 500) {
      if (minDistance != newMinDist) {
        minDistance = newMinDist;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"minDistance\"";
        firstChange = false;
        
        // Debug logging
        Serial.printf("Web UI set minDistance to %d\n", minDistance);
      }
    } else {
      // Log invalid value and set error message
      Serial.printf("INVALID min distance from web: %d (max is %d)\n", newMinDist, maxDistance);
      errorMessage = "Invalid minimum distance (must be < max distance and ≤ 500)";
    }
  }
  
  if (server.hasArg("maxDist")) {
    int newMaxDist = server.arg("maxDist").toInt();
    // Ensure newMaxDist is in valid range and greater than minDistance
    if (newMaxDist > minDistance && newMaxDist <= 1000) {
      if (maxDistance != newMaxDist) {
        maxDistance = newMaxDist;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"maxDistance\"";
        firstChange = false;
        
        // Debug logging
        Serial.printf("Web UI set maxDistance to %d\n", maxDistance);
      }
    } else {
      // Log invalid value and set error message
      Serial.printf("INVALID max distance from web: %d (min is %d)\n", newMaxDist, minDistance);
      errorMessage = "Invalid maximum distance (must be > min distance and ≤ 1000)";
    }
  }
  
  if (server.hasArg("brightness")) {
    int newBrightness = server.arg("brightness").toInt();
    if (newBrightness >= 0 && newBrightness <= 255) {
      if (brightness != newBrightness) {
        brightness = newBrightness;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"brightness\"";
        firstChange = false;
      }
    } else {
      errorMessage = "Invalid brightness (must be between 0 and 255)";
    }
  }
  
  if (server.hasArg("lightSpan")) {
    int newLightSpan = server.arg("lightSpan").toInt();
    if (newLightSpan > 0 && newLightSpan <= 100) {
      if (movingLightSpan != newLightSpan) {
        movingLightSpan = newLightSpan;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"movingLightSpan\"";
        firstChange = false;
        
        // Save settings immediately
        saveSettings();
        EEPROM.commit();
        updateLEDConfig();
      }
    } else {
      errorMessage = "Invalid light span (must be between 1 and 100)";
    }
  }
  
  // Handle RGB color values
  if (server.hasArg("redValue")) {
    int newRedValue = server.arg("redValue").toInt();
    if (newRedValue >= 0 && newRedValue <= 255) {
      if (redValue != newRedValue) {
        redValue = newRedValue;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"redValue\"";
        firstChange = false;
      }
    } else {
      errorMessage = "Invalid red value (must be between 0 and 255)";
    }
  }
  
  if (server.hasArg("greenValue")) {
    int newGreenValue = server.arg("greenValue").toInt();
    if (newGreenValue >= 0 && newGreenValue <= 255) {
      if (greenValue != newGreenValue) {
        greenValue = newGreenValue;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"greenValue\"";
        firstChange = false;
      }
    } else {
      errorMessage = "Invalid green value (must be between 0 and 255)";
    }
  }
  
  if (server.hasArg("blueValue")) {
    int newBlueValue = server.arg("blueValue").toInt();
    if (newBlueValue >= 0 && newBlueValue <= 255) {
      if (blueValue != newBlueValue) {
        blueValue = newBlueValue;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"blueValue\"";
        firstChange = false;
      }
    } else {
      errorMessage = "Invalid blue value (must be between 0 and 255)";
    }
  }

  response += "]}";

  if (settingsChanged) {
    // Save settings and ensure they are committed
    saveSettings();  // Save all settings to EEPROM
    
    // Add a small delay to ensure EEPROM write completes
    delay(10);
    
    // Update LED configuration (this will handle LED count changes properly)
    updateLEDConfig();
    
    // Send the response that was built
    server.send(200, "application/json; charset=utf-8", response);
  } else if (errorMessage != "") {
    // Return error message if validation failed
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"" + errorMessage + "\"}");
  } else {
    // If nothing changed but there were no errors
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"No changes needed\"}");
  }
}

// Add new endpoint for LED testing
void handleTestLEDs() {
  Serial.println("Web UI: Testing LEDs requested");
  testAllLEDs();
  server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"LED test completed\"}");
}

// Added new endpoint for force LED reinitialization
void handleReinitLEDs() {
  if (server.hasArg("count")) {
    int newCount = server.arg("count").toInt();
    
    if (validateLEDCount(newCount)) {
      Serial.printf("Web UI: Force reinitializing LEDs to %d\n", newCount);
      reinitializeLEDStrip(newCount);
      server.send(200, "application/json; charset=utf-8", 
                 "{\"status\":\"success\",\"message\":\"LEDs reinitialized\",\"count\":" + String(numLeds) + "}");
    } else {
      server.send(400, "application/json; charset=utf-8", 
                 "{\"status\":\"error\",\"message\":\"Invalid LED count\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", 
               "{\"status\":\"error\",\"message\":\"Missing count parameter\"}");
  }
}

void handleSetLightMode() {
  if (server.hasArg("mode")) {
    lightMode = server.arg("mode").toInt();
    saveSettings();
    updateLEDConfig();
    
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Light mode updated\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing mode parameter\"}");
  }
}

void handleSetDirectionalLight() {
  if (server.hasArg("enabled")) {
    String enabledStr = server.arg("enabled");
    bool newValue = (enabledStr == "true" || enabledStr == "1");
    
    if (directionLightEnabled != newValue) {
      directionLightEnabled = newValue;
      saveAdvancedSettings(); // Save to specific section
      EEPROM.commit(); // Force commit to EEPROM
    }
    
    server.send(200, "application/json; charset=utf-8", 
               "{\"status\":\"success\",\"message\":\"Directional light set to " + 
               String(directionLightEnabled ? "enabled" : "disabled") + "\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", 
               "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}

// Update handleSetBackgroundMode function with similar pattern
void handleSetBackgroundMode() {
  if (server.hasArg("enabled")) {
    String enabledStr = server.arg("enabled");
    bool newValue = (enabledStr == "true" || enabledStr == "1");
    
    if (backgroundMode != newValue) {
      backgroundMode = newValue;
      saveAdvancedSettings(); // Save to specific section
      EEPROM.commit(); // Force commit to EEPROM
    }
    
    server.send(200, "application/json; charset=utf-8", 
               "{\"status\":\"success\",\"message\":\"Background mode set to " + 
               String(backgroundMode ? "enabled" : "disabled") + "\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", 
               "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}

void handleSetCenterShift() {
  if (server.hasArg("value")) {
    centerShift = server.arg("value").toInt();
    saveAdvancedSettings(); // Save to specific section
    EEPROM.commit(); // Force commit to EEPROM
    
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Center shift updated\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleSetTrailLength() {
  if (server.hasArg("value")) {
    trailLength = server.arg("value").toInt();
    // Save to EEPROM
    saveAdvancedSettings();
    EEPROM.commit(); // Make sure we explicitly commit
    
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Trail length updated\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleSetMotionSmoothing() {
  if (server.hasArg("enabled")) {
    String enabledStr = server.arg("enabled");
    bool newValue = (enabledStr == "true" || enabledStr == "1");
    
    if (motionSmoothingEnabled != newValue) {
      motionSmoothingEnabled = newValue;
      saveMotionSettings(); // Save to specific section
      EEPROM.commit(); // Force commit to EEPROM
    }
    
    server.send(200, "application/json; charset=utf-8", 
               "{\"status\":\"success\",\"message\":\"Motion smoothing set to " + 
               String(motionSmoothingEnabled ? "enabled" : "disabled") + "\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", 
               "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
} 
void handleSetMotionSmoothingParam() {
  if (server.hasArg("param") && server.hasArg("value")) {
    String param = server.arg("param");
    float value = server.arg("value").toFloat();
    bool validParam = true;
    float constrainedValue = value; // Store the constrained value

    // Validate and constrain values
    if (param == "positionSmoothingFactor") {
      constrainedValue = constrain(value, 0.0, 1.0);
      positionSmoothingFactor = constrainedValue;
    } 
    else if (param == "velocitySmoothingFactor") {
      constrainedValue = constrain(value, 0.0, 1.0);
      velocitySmoothingFactor = constrainedValue;
    } 
    else if (param == "predictionFactor") {
      constrainedValue = constrain(value, 0.0, 1.0);
      predictionFactor = constrainedValue;
    } 
    else if (param == "positionPGain") {
      constrainedValue = constrain(value, 0.0, 1.0);
      positionPGain = constrainedValue;
    } 
    else if (param == "positionIGain") {
      constrainedValue = constrain(value, 0.0, 0.1);  // Tighter constraint for I gain
      positionIGain = constrainedValue;
    } 
    else {
      validParam = false;
      server.send(400, "application/json; charset=utf-8", 
        "{\"error\":\"Invalid parameter\",\"param\":\"" + param + "\"}");
      return;
    }

    if (validParam) {
      // Immediately save to ensure persistence
      saveMotionSettings();
      EEPROM.commit();  // Make sure we explicitly commit
      
      // Prepare a JSON response with the actual constrained value
      String jsonResponse = "{\"param\":\"" + param + "\",\"value\":" + 
        String(constrainedValue, 3) + ",\"status\":\"success\"}";
      
      server.send(200, "application/json; charset=utf-8", jsonResponse);
      
      if (ENABLE_DEBUG_LOGGING) {
        Serial.printf("Updated motion parameter %s to %.3f\n", param.c_str(), constrainedValue);
      }
    }
  } else {
    server.send(400, "application/json; charset=utf-8", 
      "{\"error\":\"Missing parameter or value\"}");
  }
}

void handleSetEffectSpeed() {
  if (server.hasArg("value")) {
    effectSpeed = server.arg("value").toInt();
    saveSettings();
    
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Effect speed updated\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleSetEffectIntensity() {
  if (server.hasArg("value")) {
    effectIntensity = server.arg("value").toInt();
    saveSettings();
    
    server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Effect intensity updated\"}");
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleNetworkPost() {
  if (server.hasArg("ssid") && server.hasArg("deviceName")) {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String deviceName = server.arg("deviceName");
    
    if (ssid.length() == 0 || deviceName.length() == 0) {
      server.send(400, "text/html; charset=utf-8", "<h1>Error</h1><p>SSID and Device Name cannot be empty</p><a href='/network'>Back</a>");
      return;
    }
    
    Serial.printf("[WiFi] Saving settings: SSID=%s, Device=%s\n", 
                 ssid.c_str(), deviceName.c_str());
    
    // Make sure WiFi marker is set
    EEPROM.put(EEPROM_WIFI_MARKER_ADDR, (uint16_t)0xA55A);
    
    // Save credentials
    wifiManager.saveWifiCredentials(ssid.c_str(), password.c_str(), deviceName.c_str());
    
    // Create sanitized hostname for display
    String hostname = wifiManager.getSanitizedHostname(deviceName.c_str());
    
    // Show success page
    String html = "<html><head><meta http-equiv='refresh' content='10;url=/'></head>"
                  "<body style='font-family:Arial;text-align:center;padding:50px;background:#121212;color:#fff;'>"
                  "<h1>WiFi Settings Saved</h1>"
                  "<p>The device will restart in 10 seconds and attempt to connect to the new network.</p>"
                  "<p>If connection is successful, you can access it at:<br>"
                  "<strong>http://" + hostname + ".local</strong></p>"
                  "<p>(Device is restarting...)</p>"
                  "</body></html>";
    
    server.send(200, "text/html; charset=utf-8", html);
    
    // Schedule restart
    extern bool shouldRestartDevice;
    extern unsigned long resetRequestTime;
    shouldRestartDevice = true;
    resetRequestTime = millis();
  } else {
    server.send(400, "text/html; charset=utf-8", "<h1>Error</h1><p>Missing required fields</p><a href='/network'>Back</a>");
  }
}

void handleResetWifi() {
  String html = "<html><head><meta http-equiv='refresh' content='5;url=/'></head>"
                "<body style='font-family:Arial;text-align:center;padding:50px;background:#121212;color:#fff;'>"
                "<h1>WiFi Settings Reset</h1>"
                "<p>All WiFi settings have been cleared.</p>"
                "<p>The device will restart in AP mode in 5 seconds...</p>"
                "</body></html>";
  
  server.send(200, "text/html; charset=utf-8", html);
  
  // Schedule WiFi reset
  extern bool shouldResetWifi;
  extern unsigned long resetRequestTime;
  shouldResetWifi = true;
  resetRequestTime = millis();
}

void handleScanNetworks() {
  std::vector<WiFiNetwork> networks = wifiManager.scanNetworks();
  
  // If no networks found, return an empty array rather than error
  String json = "[";
  
  if (!networks.empty()) {
    // Limit to top 5 networks with strongest signal
    int count = min(5, (int)networks.size());
    
    for (int i = 0; i < count; i++) {
      if (i > 0) json += ",";
      
      // Encode the SSID for JSON (escaping quotes and backslashes)
      String escapedSSID = networks[i].ssid;
      escapedSSID.replace("\\", "\\\\");  // Escape backslashes first
      escapedSSID.replace("\"", "\\\"");  // Then escape quotes
      
      json += "{\"ssid\":\"" + escapedSSID + "\",";
      json += "\"rssi\":" + String(networks[i].rssi) + ",";
      json += "\"secure\":" + String(networks[i].encType != WIFI_AUTH_OPEN ? "true" : "false") + "}";
    }
  }
  
  json += "]";
  
  server.send(200, "application/json; charset=utf-8", json);
}

// ESP-NOW handlers
void handleGetDeviceInfo() {
  // Create a JSON response with device role, MAC address, paired devices, and sensor priority mode
  String json = "{";
  json += "\"role\":" + String(deviceRole) + ",";
  
  // Add MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  json += "\"mac\":\"" + String(macStr) + "\"";
  
  // Add master MAC if this is a slave
  if (deviceRole == DEVICE_ROLE_SLAVE) {
    char masterMacStr[18];
    sprintf(masterMacStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            masterAddress[0], masterAddress[1], masterAddress[2], 
            masterAddress[3], masterAddress[4], masterAddress[5]);
    json += ",\"masterMac\":\"" + String(masterMacStr) + "\"";
  }
  
  // Add slave MACs if this is a master
  if (deviceRole == DEVICE_ROLE_MASTER && numSlaveDevices > 0) {
    json += ",\"slaves\":[";
    for (int i = 0; i < numSlaveDevices; i++) {
      if (i > 0) json += ",";
      char slaveMacStr[18];
      sprintf(slaveMacStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
              slaveAddresses[i][0], slaveAddresses[i][1], slaveAddresses[i][2], 
              slaveAddresses[i][3], slaveAddresses[i][4], slaveAddresses[i][5]);
      json += "\"" + String(slaveMacStr) + "\"";
    }
    json += "]";
  } else {
    json += ",\"slaves\":[]";
  }
  
  // Add sensor priority mode
  json += ",\"sensorPriorityMode\":" + String(sensorPriorityMode);
  
  json += "}";
  
  server.send(200, "application/json; charset=utf-8", json);
}

void handleSetDeviceRole() {
  if (server.hasArg("role")) {
    uint8_t newRole = server.arg("role").toInt();
    if (newRole == DEVICE_ROLE_MASTER || newRole == DEVICE_ROLE_SLAVE) {
      deviceRole = newRole;
      
      // Save to EEPROM
      EEPROM.write(EEPROM_ADDR_DEVICE_ROLE, deviceRole);
      EEPROM.commit();
      
      // Re-initialize ESP-NOW with new role
      esp_now_deinit();
      setupESPNOW();
      
      server.send(200, "application/json; charset=utf-8", "{\"status\":\"success\",\"message\":\"Device role updated\"}");
    } else {
      server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Invalid role value\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", "{\"status\":\"error\",\"message\":\"Missing role parameter\"}");
  }
}

// Add this updated function to web_interface.cpp, replacing the existing handleScanForSlaves

void handleScanForSlaves() {
  Serial.println("Scanning for AmbiSense slave devices...");
  
  // Scan with longer timeout for better detection
  int numDevices = WiFi.scanNetworks(false, true, false, 300);
  
  String jsonResponse = "[";
  int devicesFound = 0;
  
  for (int i = 0; i < numDevices && devicesFound < 10; i++) {
    String ssid = WiFi.SSID(i);
    
    // Only include AmbiSense devices or devices that could be AmbiSense
    bool isAmbiSense = ssid.startsWith(AMBISENSE_DEVICE_PREFIX) || 
                       ssid.startsWith("AmbiSense") ||
                       ssid.indexOf("AmbiSense") >= 0;
    
    if (isAmbiSense || ssid.length() == 0) { // Include hidden networks too
      if (devicesFound > 0) jsonResponse += ",";
      
      uint8_t* bssid = WiFi.BSSID(i);
      char macStr[18];
      sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
              bssid[0], bssid[1], bssid[2], 
              bssid[3], bssid[4], bssid[5]);
      
      // Determine signal strength label
      String signalStrength = "Unknown";
      int rssi = WiFi.RSSI(i);
      if (rssi >= -50) signalStrength = "Excellent";
      else if (rssi >= -60) signalStrength = "Good";
      else if (rssi >= -70) signalStrength = "Fair";
      else signalStrength = "Weak";
      
      String deviceName = ssid.length() > 0 ? ssid : "Hidden AmbiSense Device";
      
      jsonResponse += "{\"name\":\"" + deviceName + "\",";
      jsonResponse += "\"mac\":\"" + String(macStr) + "\",";
      jsonResponse += "\"rssi\":" + String(rssi) + ",";
      jsonResponse += "\"signal\":\"" + signalStrength + "\",";
      jsonResponse += "\"isAmbiSense\":" + String(isAmbiSense ? "true" : "false") + "}";
      
      devicesFound++;
    }
  }
  
  jsonResponse += "]";
  
  // Free memory used by the scan
  WiFi.scanDelete();
  
  // Send the response
  server.send(200, "application/json; charset=utf-8", jsonResponse);
}

void handleAddSlave() {
  if (server.hasArg("mac") && deviceRole == DEVICE_ROLE_MASTER) {
    String macStr = server.arg("mac");
    
    // Parse MAC address
    uint8_t mac[6];
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
      
      for (int i = 0; i < 6; i++) {
        mac[i] = static_cast<uint8_t>(values[i]);
      }
      
      // Check if we already have this slave
      bool alreadyExists = false;
      for (int i = 0; i < numSlaveDevices; i++) {
        bool match = true;
        for (int j = 0; j < 6; j++) {
          if (slaveAddresses[i][j] != mac[j]) {
            match = false;
            break;
          }
        }
        if (match) {
          alreadyExists = true;
          break;
        }
      }
      
      if (!alreadyExists && numSlaveDevices < MAX_SLAVE_DEVICES) {
        // Add the new slave
        for (int i = 0; i < 6; i++) {
          slaveAddresses[numSlaveDevices][i] = mac[i];
        }
        
        // Save to EEPROM
        EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES, numSlaveDevices + 1);
        for (int i = 0; i < 6; i++) {
          EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES + 1 + (numSlaveDevices * 6) + i, mac[i]);
        }
        EEPROM.commit();
        
        // Add as ESP-NOW peer
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, mac, 6);
        peerInfo.channel = 0; // Use WiFi channel 0
        peerInfo.encrypt = false; // No encryption for simplicity
        
        if (esp_now_add_peer(&peerInfo) == ESP_OK) {
          numSlaveDevices++;
          server.send(200, "application/json; charset=utf-8", 
            "{\"status\":\"success\",\"message\":\"Slave device added\"}");
        } else {
          server.send(500, "application/json; charset=utf-8", 
            "{\"status\":\"error\",\"message\":\"Failed to add ESP-NOW peer\"}");
        }
      } else if (alreadyExists) {
        server.send(400, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Slave device already paired\"}");
      } else {
        server.send(400, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Maximum number of slave devices reached\"}");
      }
    } else {
      server.send(400, "application/json; charset=utf-8", 
        "{\"status\":\"error\",\"message\":\"Invalid MAC address format\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", 
      "{\"status\":\"error\",\"message\":\"Missing MAC address or not in master mode\"}");
  }
}

void handleRemoveSlave() {
  if (server.hasArg("mac") && deviceRole == DEVICE_ROLE_MASTER) {
    String macStr = server.arg("mac");
    
    // Parse MAC address
    uint8_t mac[6];
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
      
      for (int i = 0; i < 6; i++) {
        mac[i] = static_cast<uint8_t>(values[i]);
      }
      
      // Find the slave in our list
      int slaveIndex = -1;
      for (int i = 0; i < numSlaveDevices; i++) {
        bool match = true;
        for (int j = 0; j < 6; j++) {
          if (slaveAddresses[i][j] != mac[j]) {
            match = false;
            break;
          }
        }
        if (match) {
          slaveIndex = i;
          break;
        }
      }
      
      if (slaveIndex >= 0) {
        // Remove this peer from ESP-NOW
        esp_now_del_peer(mac);
        
        // Shift remaining slaves down
        for (int i = slaveIndex; i < numSlaveDevices - 1; i++) {
          for (int j = 0; j < 6; j++) {
            slaveAddresses[i][j] = slaveAddresses[i + 1][j];
          }
        }
        
        numSlaveDevices--;
        
        // Update EEPROM
        EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES, numSlaveDevices);
        for (int i = 0; i < numSlaveDevices; i++) {
          for (int j = 0; j < 6; j++) {
            EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES + 1 + (i * 6) + j, slaveAddresses[i][j]);
          }
        }
        EEPROM.commit();
        
        server.send(200, "application/json; charset=utf-8", 
          "{\"status\":\"success\",\"message\":\"Slave device removed\"}");
      } else {
        server.send(404, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Slave device not found\"}");
      }
    } else {
      server.send(400, "application/json; charset=utf-8", 
        "{\"status\":\"error\",\"message\":\"Invalid MAC address format\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", 
      "{\"status\":\"error\",\"message\":\"Missing MAC address or not in master mode\"}");
  }
}
void handleGetSensorData() {
  // Create JSON with latest sensor readings
  String json = "{\"sensors\":[";
  
  for (int i = 0; i <= MAX_SLAVE_DEVICES; i++) {
    if (i > 0 && latestSensorData[i].timestamp == 0) continue; // Skip uninitialized slave devices
    
    if (i > 0) json += ",";
    
    json += "{";
    json += "\"id\":" + String(latestSensorData[i].sensorId) + ",";
    json += "\"distance\":" + String(latestSensorData[i].distance) + ",";
    json += "\"direction\":" + String(latestSensorData[i].direction) + ",";
    json += "\"age\":" + String(millis() - latestSensorData[i].timestamp) + ",";
    json += "\"active\":" + String((millis() - latestSensorData[i].timestamp < 5000) ? "true" : "false");
    json += "}";
  }
  
  json += "],";
  json += "\"selected\":" + String(currentDistance) + ",";
  json += "\"mode\":" + String(sensorPriorityMode);
  json += "}";
  
  server.send(200, "application/json; charset=utf-8", json);
}
void handleSetMasterMac() {
  if (server.hasArg("mac") && deviceRole == DEVICE_ROLE_SLAVE) {
    String macStr = server.arg("mac");
    
    // Parse MAC address
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
      
      // Validate MAC address is not all zeros
      bool allZeros = true;
      for (int i = 0; i < 6; i++) {
        if (values[i] != 0) {
          allZeros = false;
          break;
        }
      }
      
      if (allZeros) {
        server.send(400, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Invalid MAC address - cannot be all zeros\"}");
        return;
      }
      
      // Store new master MAC address
      for (int i = 0; i < 6; i++) {
        masterAddress[i] = static_cast<uint8_t>(values[i]);
        EEPROM.write(EEPROM_ADDR_MASTER_MAC + i, masterAddress[i]);
      }
      
      // Commit to EEPROM first
      if (!EEPROM.commit()) {
        server.send(500, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Failed to save master MAC to EEPROM\"}");
        return;
      }
      
      // Log the new master MAC
      char masterMacStr[18];
      sprintf(masterMacStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
              masterAddress[0], masterAddress[1], masterAddress[2], 
              masterAddress[3], masterAddress[4], masterAddress[5]);
      Serial.printf("Setting new master MAC: %s\n", masterMacStr);
      
      // Remove any existing peers
      esp_now_peer_info_t peerInfo = {};
      if (esp_now_fetch_peer(true, &peerInfo) == ESP_OK) {
        if (memcmp(peerInfo.peer_addr, masterAddress, 6) == 0) {
          esp_now_del_peer(masterAddress);
        }
      }
      
      // Add the master as a peer with auto channel selection
      memcpy(peerInfo.peer_addr, masterAddress, 6);
      peerInfo.channel = WiFi.channel(); // Use current WiFi channel
      peerInfo.encrypt = false;
      
      // Try to add the peer
      esp_err_t addResult = esp_now_add_peer(&peerInfo);
      
      if (addResult == ESP_OK) {
        // Send a test message to verify connection
        sensor_data_t testData = {0};
        testData.sensorId = 1;
        testData.distance = 0;
        testData.timestamp = millis();
        
        esp_err_t sendResult = esp_now_send(masterAddress, (uint8_t*)&testData, sizeof(sensor_data_t));
        
        if (sendResult == ESP_OK) {
          server.send(200, "application/json; charset=utf-8", 
            "{\"status\":\"success\",\"message\":\"Master device configured and test message sent\"}");
        } else {
          server.send(200, "application/json; charset=utf-8", 
            "{\"status\":\"warning\",\"message\":\"Master configured but test message failed\"}");
        }
      } else {
        server.send(500, "application/json; charset=utf-8", 
          "{\"status\":\"error\",\"message\":\"Failed to add master as ESP-NOW peer\"}");
      }
    } else {
      server.send(400, "application/json; charset=utf-8", 
        "{\"status\":\"error\",\"message\":\"Invalid MAC address format\"}");
    }
  } else {
    server.send(400, "application/json; charset=utf-8", 
      "{\"status\":\"error\",\"message\":\"Missing MAC address or not in slave mode\"}");
  }
}
