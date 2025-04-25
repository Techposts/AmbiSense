#include <WebServer.h>
#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include <ESPmDNS.h>
#include "wifi_manager.h"

WebServer server(WEB_SERVER_PORT);

// Forward declarations of handler functions
void handleRoot();
void handleSet();
void handleDistance();
void handleSettings();
void handleSetLightMode();
void handleSetDirectionalLight();
void handleSetCenterShift();
void handleSetTrailLength();
void handleSetBackgroundMode();
void handleNetworkSettings();
void handleResetWifi();
void handleScanNetworks();

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/distance", handleDistance);
  server.on("/settings", handleSettings);
  server.on("/setLightMode", handleSetLightMode);
  server.on("/setDirectionalLight", handleSetDirectionalLight);
  server.on("/setCenterShift", handleSetCenterShift);
  server.on("/setTrailLength", handleSetTrailLength);
  server.on("/setBackgroundMode", handleSetBackgroundMode);
  // Add new network configuration routes
  server.on("/network", handleNetworkSettings);
  server.on("/resetwifi", handleResetWifi);
  server.on("/scannetworks", handleScanNetworks);
  // Inside setupWebServer() function, add:
  server.on("/setMotionSmoothing", handleSetMotionSmoothing);
  server.on("/setMotionSmoothingParam", handleSetMotionSmoothingParam);
  server.on("/setEffectSpeed", handleSetEffectSpeed);
  server.on("/setEffectIntensity", handleSetEffectIntensity);
  server.begin();
}

void handleWebServer() {
  server.handleClient();
}

// Replace only this function in web_interface.cpp
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
  
  // Send response with appropriate headers
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.send(200, "application/json", json);
}

// Handle Distance API (returns real-time distance)
void handleDistance() {
  server.send(200, "text/plain", String(currentDistance));
}

// Handle Settings API (returns current settings as JSON)
void handleSettings() {
  // Re-load settings from EEPROM to ensure we have the latest values
  loadSettings();
  
  // Create JSON with all current settings
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
  json += "\"effectIntensity\":" + String(effectIntensity) + "";
  json += ",\"positionSmoothingFactor\":" + String(positionSmoothingFactor, 3);
  json += ",\"velocitySmoothingFactor\":" + String(velocitySmoothingFactor, 3);
  json += ",\"predictionFactor\":" + String(predictionFactor, 3);
  json += ",\"positionPGain\":" + String(positionPGain, 3);
  json += ",\"positionIGain\":" + String(positionIGain, 3);
  json += "}";
  
  server.send(200, "application/json", json);
}

// Handle Light Mode Setting
void handleSetLightMode() {
  if (server.hasArg("mode")) {
    lightMode = server.arg("mode").toInt();
    saveSettings();
    updateLEDConfig();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Light mode updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing mode parameter\"}");
  }
}

// Handle Directional Light Setting
void handleSetDirectionalLight() {
  if (server.hasArg("enabled")) {
    directionLightEnabled = server.arg("enabled") == "true";
    saveSettings();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Directional light setting updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}

// Handle Center Shift Setting
void handleSetCenterShift() {
  if (server.hasArg("value")) {
    centerShift = server.arg("value").toInt();
    saveSettings();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Center shift updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

// Handle Trail Length Setting
void handleSetTrailLength() {
  if (server.hasArg("value")) {
    trailLength = server.arg("value").toInt();
    saveSettings();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Trail length updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

// Handle Background Mode Setting
void handleSetBackgroundMode() {
  if (server.hasArg("enabled")) {
    backgroundMode = server.arg("enabled") == "true";
    saveSettings();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Background mode updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}

// Handle Settings Update
void handleSet() {
  bool settingsChanged = false;
  String response = "{\"status\":\"success\", \"changes\":[";
  bool firstChange = true;

  if (server.hasArg("numLeds")) {
    int newNumLeds = server.arg("numLeds").toInt();
    if (newNumLeds > 0 && newNumLeds <= 2000) { // Reasonable limit
      if (numLeds != newNumLeds) {
        numLeds = newNumLeds;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"numLeds\"";
        firstChange = false;
      }
    }
  }
  
  if (server.hasArg("minDist")) {
    int newMinDist = server.arg("minDist").toInt();
    if (minDistance != newMinDist) {
      minDistance = newMinDist;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"minDistance\"";
      firstChange = false;
    }
  }
  
  if (server.hasArg("maxDist")) {
    int newMaxDist = server.arg("maxDist").toInt();
    if (maxDistance != newMaxDist) {
      maxDistance = newMaxDist;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"maxDistance\"";
      firstChange = false;
    }
  }
  
  if (server.hasArg("brightness")) {
    int newBrightness = server.arg("brightness").toInt();
    if (brightness != newBrightness) {
      brightness = newBrightness;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"brightness\"";
      firstChange = false;
    }
  }
  
  if (server.hasArg("lightSpan")) {
    int newLightSpan = server.arg("lightSpan").toInt();
    if (movingLightSpan != newLightSpan) {
      movingLightSpan = newLightSpan;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"movingLightSpan\"";
      firstChange = false;
    }
  }
  
  // Handle RGB color values
  if (server.hasArg("redValue")) {
    int newRedValue = server.arg("redValue").toInt();
    if (redValue != newRedValue) {
      redValue = newRedValue;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"redValue\"";
      firstChange = false;
    }
  }
  
  if (server.hasArg("greenValue")) {
    int newGreenValue = server.arg("greenValue").toInt();
    if (greenValue != newGreenValue) {
      greenValue = newGreenValue;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"greenValue\"";
      firstChange = false;
    }
  }
  
  if (server.hasArg("blueValue")) {
    int newBlueValue = server.arg("blueValue").toInt();
    if (blueValue != newBlueValue) {
      blueValue = newBlueValue;
      settingsChanged = true;
      if (!firstChange) response += ",";
      response += "\"blueValue\"";
      firstChange = false;
    }
  }

  response += "]}";

  if (settingsChanged) {
    saveSettings();
    updateLEDConfig();
  }
  
  server.send(200, "application/json", response);
}

// New handler for motion smoothing enable/disable
void handleSetMotionSmoothing() {
  if (server.hasArg("enabled")) {
    motionSmoothingEnabled = (server.arg("enabled") == "true");
    saveSettings();
    
    // Return JSON response instead of redirect
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Motion smoothing setting updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}


void handleSetEffectSpeed() {
  if (server.hasArg("value")) {
    effectSpeed = server.arg("value").toInt();
    saveSettings();
    
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Effect speed updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleSetEffectIntensity() {
  if (server.hasArg("value")) {
    effectIntensity = server.arg("value").toInt();
    saveSettings();
    
    server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"Effect intensity updated\"}");
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

// New handler for motion smoothing parameters
void handleSetMotionSmoothingParam() {
  if (server.hasArg("param") && server.hasArg("value")) {
    String param = server.arg("param");
    float value = server.arg("value").toFloat();
    bool validParam = true;

    Serial.printf("Setting motion smoothing parameter %s to %.3f\n", param.c_str(), value);

    // Validate and constrain values
    if (param == "positionSmoothingFactor") {
      positionSmoothingFactor = constrain(value, 0.0, 1.0);
    } 
    else if (param == "velocitySmoothingFactor") {
      velocitySmoothingFactor = constrain(value, 0.0, 1.0);
    } 
    else if (param == "predictionFactor") {
      predictionFactor = constrain(value, 0.0, 1.0);
    } 
    else if (param == "positionPGain") {
      positionPGain = constrain(value, 0.0, 1.0);
    } 
    else if (param == "positionIGain") {
      positionIGain = constrain(value, 0.0, 0.1);  // Tighter constraint for I gain
    } 
    else {
      validParam = false;
      server.send(400, "application/json", 
        "{\"error\":\"Invalid parameter\",\"param\":\"" + param + "\"}");
      return;
    }

    if (validParam) {
      // Immediately save to ensure persistence
      saveSettings();
      
      // Prepare a JSON response with the actual constrained value
      String jsonResponse = "{\"param\":\"" + param + "\",\"value\":" + 
        String(value, 3) + ",\"status\":\"success\"}";
      
      server.send(200, "application/json", jsonResponse);
      
      // Debug output
      Serial.println("Updated motion smoothing parameters:");
      Serial.printf("- Position Smoothing Factor: %.2f\n", positionSmoothingFactor);
      Serial.printf("- Velocity Smoothing Factor: %.2f\n", velocitySmoothingFactor);
      Serial.printf("- Prediction Factor: %.2f\n", predictionFactor);
      Serial.printf("- Position P Gain: %.2f\n", positionPGain);
      Serial.printf("- Position I Gain: %.3f\n", positionIGain);
    }
  } else {
    server.send(400, "application/json", 
      "{\"error\":\"Missing parameter or value\"}");
  }
}



// Enhanced Web Interface
void handleRoot() {
  const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>AmbiSense | TechPosts Media</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    :root {
      --primary: #4361ee;
      --primary-light: #4895ef;
      --success: #4cc9f0;
      --bg-dark: #121212;
      --bg-card: #1e1e1e;
      --text: #ffffff;
      --text-secondary: #b0b0b0;
      --border-radius: 12px;
      --shadow: 0 10px 20px rgba(0,0,0,0.3);
      --transition: all 0.3s ease;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 20px;
      background: linear-gradient(135deg, #121212 0%, #2a2a2a 100%);
    }
    
    .main-content {
      display: flex;
      justify-content: center;
      align-items: center;
      flex: 1;
    }
    
    .container {
      background: var(--bg-card);
      border-radius: var(--border-radius);
      box-shadow: var(--shadow);
      width: 100%;
      max-width: 420px;
      overflow: hidden;
      position: relative;
    }
    
    .header {
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      padding: 25px 20px;
      text-align: center;
      position: relative;
      overflow: hidden;
    }
    
    .header h1 {
      font-size: 24px;
      font-weight: 600;
      margin-bottom: 8px;
      text-shadow: 0 2px 5px rgba(0,0,0,0.2);
      position: relative;
      z-index: 2;
    }
    
    .header p {
      opacity: 0.9;
      font-size: 14px;
      position: relative;
      z-index: 2;
    }
    
    .header::before {
      content: '';
      position: absolute;
      top: -50%;
      left: -50%;
      width: 200%;
      height: 200%;
      background: radial-gradient(circle, rgba(255,255,255,0.1) 0%, rgba(255,255,255,0) 70%);
      z-index: 1;
    }
    
    .content {
      padding: 25px;
    }
    
    .form-group {
      margin-bottom: 22px;
      position: relative;
    }
    
    .form-group label {
      display: block;
      margin-bottom: 8px;
      font-size: 14px;
      color: var(--text-secondary);
      font-weight: 500;
    }
    
    .slider-container {
      display: flex;
      align-items: center;
      gap: 15px;
    }
    
    .slider {
      flex: 1;
      -webkit-appearance: none;
      height: 8px;
      border-radius: 5px;
      background: #333;
      outline: none;
      transition: var(--transition);
    }
    
    .slider::-webkit-slider-thumb {
      -webkit-appearance: none;
      appearance: none;
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: var(--primary);
      cursor: pointer;
      transition: var(--transition);
      box-shadow: 0 0 10px rgba(67, 97, 238, 0.5);
    }
    
    .slider::-moz-range-thumb {
      width: 18px;
      height: 18px;
      border-radius: 50%;
      background: var(--primary);
      cursor: pointer;
      transition: var(--transition);
      box-shadow: 0 0 10px rgba(67, 97, 238, 0.5);
    }
    
    .slider:hover::-webkit-slider-thumb {
      transform: scale(1.1);
    }
    
    .slider-value {
      min-width: 60px;
      padding: 8px 12px;
      border-radius: var(--border-radius);
      background: #333;
      text-align: center;
      font-weight: 600;
      font-size: 14px;
      color: var(--text);
    }
    
    .color-preview {
      width: 60px;
      height: 36px;
      border-radius: var(--border-radius);
      overflow: hidden;
      box-shadow: inset 0 0 5px rgba(0,0,0,0.5);
    }
    
    .rgb-sliders {
      margin-top: 15px;
    }
    
    .rgb-label {
      display: inline-block;
      width: 20px;
      text-align: center;
      font-weight: bold;
      margin-right: 10px;
    }
    
    .rgb-label.r { color: #ff5555; }
    .rgb-label.g { color: #55ff55; }
    .rgb-label.b { color: #5555ff; }
    
    .led-count-container {
      display: flex;
      align-items: center;
      gap: 15px;
    }
    
    .led-count-input {
      flex: 1;
      padding: 10px 12px;
      border-radius: var(--border-radius);
      background: #333;
      border: none;
      color: var(--text);
      font-size: 14px;
      text-align: center;
    }
    
    .led-meter-container {
      margin-top: 15px;
      display: flex;
      gap: 10px;
      align-items: center;
    }
    
    .led-count-meter {
      flex: 1;
      height: 4px;
      background: #333;
      position: relative;
    }
    
    .led-count-meter::before {
      content: '';
      position: absolute;
      height: 100%;
      left: 0;
      background: linear-gradient(90deg, #ff0000, #00ff00, #0000ff);
      transition: var(--transition);
    }
    
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      margin-top: 30px;
      text-transform: uppercase;
      letter-spacing: 1px;
      box-shadow: 0 5px 15px rgba(67, 97, 238, 0.4);
    }
    
    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(67, 97, 238, 0.6);
    }
    
    .button:active {
      transform: translateY(0);
      box-shadow: 0 2px 10px rgba(67, 97, 238, 0.4);
    }
    
    .distance-display {
      margin-top: 30px;
      text-align: center;
      padding: 20px;
      border-radius: var(--border-radius);
      background: rgba(30, 30, 30, 0.6);
      position: relative;
      overflow: hidden;
    }
    
    .distance-display::before {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 2px;
      background: linear-gradient(90deg, var(--primary) 0%, var(--success) 100%);
    }
    
    .distance-label {
      font-size: 14px;
      margin-bottom: 10px;
      color: var(--text-secondary);
    }
    
    .distance-value {
      font-size: 36px;
      font-weight: 700;
      color: var(--success);
      text-shadow: 0 0 10px rgba(76, 201, 240, 0.5);
      transition: var(--transition);
    }
    
    .distance-unit {
      font-size: 16px;
      opacity: 0.7;
      margin-left: 5px;
    }
    
    .led-visualizer {
      height: 15px;
      background: #333;
      margin-top: 15px;
      border-radius: 10px;
      overflow: hidden;
      position: relative;
    }
    
    .led-active {
      position: absolute;
      height: 100%;
      border-radius: 10px;
      transition: var(--transition);
    }
    
    .settings-saved {
      position: fixed;
      top: 20px;
      right: 20px;
      padding: 15px 25px;
      background: linear-gradient(135deg, #4cc9f0 0%, #4361ee 100%);
      color: white;
      border-radius: var(--border-radius);
      transform: translateX(200%);
      transition: transform 0.3s ease;
      box-shadow: 0 5px 15px rgba(0,0,0,0.2);
      z-index: 1000;
    }
    
    .settings-saved.show {
      transform: translateX(0);
    }
    
    .footer {
      text-align: center;
      padding: 15px 0;
      margin-top: 30px;
      font-size: 12px;
      color: var(--text-secondary);
      width: 100%;
    }
    
    .footer p {
      margin: 3px 0;
    }
    
    .branding {
      font-weight: 600;
      color: var(--text);
    }
    
    @media (max-width: 480px) {
      .container {
        border-radius: var(--border-radius);
      }
      
      .header {
        padding: 20px 15px;
      }
      
      .content {
        padding: 20px 15px;
      }
    }
    @media (min-width: 768px) {
      .container {
        max-width: 600px;
      }
      
      .slider-container {
        gap: 20px;
      }
      
      .slider-value {
        min-width: 80px;
        font-size: 16px;
      }
      
      .rgb-sliders .slider-container {
        display: flex;
        align-items: center;
      }
      
      .rgb-label {
        min-width: 30px;
      }
    }
    .section-divider {
      border-top: 1px solid #333;
      margin: 30px 0;
      position: relative;
    }

    .section-title {
      position: absolute;
      top: -10px;
      left: 50%;
      transform: translateX(-50%);
      background: var(--bg-card);
      padding: 0 15px;
      font-size: 12px;
      color: var(--text-secondary);
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .loading-overlay {
      position: absolute;
      top: 0;
      left: 0;
      width: 100%;
      height: 100%;
      background: rgba(0,0,0,0.7);
      display: flex;
      justify-content: center;
      align-items: center;
      z-index: 9999;
    }
    
    .spinner {
      width: 50px;
      height: 50px;
      border: 5px solid rgba(0,0,0,0.1);
      border-radius: 50%;
      border-top-color: var(--primary);
      animation: spin 1s ease-in-out infinite;
    }

    .toggle-switch {
      position: relative;
      display: inline-block;
      width: 60px;
      height: 34px;
    }

    .toggle-switch input {
      opacity: 0;
      width: 0;
      height: 0;
    }

    .toggle-slider {
      position: absolute;
      cursor: pointer;
      top: 0;
      left: 0;
      right: 0;
      bottom: 0;
      background-color: #333;
      transition: .4s;
      border-radius: 34px;
    }

    .toggle-slider:before {
      position: absolute;
      content: "";
      height: 26px;
      width: 26px;
      left: 4px;
      bottom: 4px;
      background-color: white;
      transition: .4s;
      border-radius: 50%;
    }

    input:checked + .toggle-slider {
      background-color: var(--primary);
    }

    input:checked + .toggle-slider:before {
      transform: translateX(26px);
    }

    .toggle-container {
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 20px;
    }

    .toggle-label {
      font-size: 14px;
      color: var(--text-secondary);
    }

    .mode-selector {
      background-color: #333;
      color: var(--text);
      border: none;
      padding: 10px;
      border-radius: var(--border-radius);
      width: 100%;
      margin-bottom: 20px;
      font-size: 14px;
    }
    
    @keyframes spin {
      to { transform: rotate(360deg); }
    }

    .tab-container {
      display: flex;
      margin-bottom: 20px;
      border-bottom: 1px solid #333;
    }

    .tab {
      padding: 10px 15px;
      cursor: pointer;
      background-color: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 14px;
      position: relative;
    }

    .tab.active {
      color: var(--primary);
    }

    .tab.active::after {
      content: '';
      position: absolute;
      bottom: -1px;
      left: 0;
      width: 100%;
      height: 2px;
      background-color: var(--primary);
    }

    .tab-content {
      display: none;
      padding-top: 20px;
    }

    .tab-content.active {
      display: block;
    }
    .ssid-input-container {
      display: flex;
      gap: 10px;
      margin-bottom: 10px;
    }

    .scan-button {
      padding: 10px 15px;
      background: var(--primary);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      cursor: pointer;
      font-weight: 600;
      flex-shrink: 0;
    }

    .scan-button:hover {
      background: var(--primary-light);
    }

    .network-dropdown {
      width: 100%;
      padding: 10px;
      border-radius: 5px;
      border: 1px solid #444;
      background-color: #333;
      color: var(--text);
      font-size: 16px;
      margin-bottom: 15px;
    }

    /* Improved button styles */
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      margin-top: 20px;
      text-transform: uppercase;
      letter-spacing: 1px;
      box-shadow: 0 5px 15px rgba(67, 97, 238, 0.4);
    }

    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(67, 97, 238, 0.6);
    }

    .button.button-secondary {
      background: linear-gradient(135deg, #555 0%, #444 100%);
      box-shadow: 0 5px 15px rgba(0, 0, 0, 0.3);
    }

    .button.button-danger {
      background: linear-gradient(135deg, #ff4d4d 0%, #ff6666 100%);
      box-shadow: 0 5px 15px rgba(255, 77, 77, 0.4);
    }
  </style>
</head>
<body>
  <div class="loading-overlay" id="loadingOverlay">
    <div class="spinner"></div>
  </div>

  <div class="main-content">
    <div class="container">
      <div class="header">
        <h1>AmbiSense</h1>
        <p>Radar-controlled LED strip configuration</p>
      </div>
      
      <div class="content">
        <div class="tab-container">
          <button class="tab active" onclick="openTab(event, 'basic')">Basic</button>
          <button class="tab" onclick="openTab(event, 'advanced')">Advanced</button>
          <button class="tab" onclick="openTab(event, 'effects')">Effects</button>
          <button class="tab" onclick="window.location.href='/network'">Network</button>
        </div>
        
        <div id="basic" class="tab-content active">
          <div class="form-group">
            <label>Number of LEDs</label>
            <div class="led-count-container">
              <input type="number" min="1" max="2000" class="led-count-input" id="numLeds" value="300">
              <div class="slider-value"><span id="ledDensity">5</span>m</div>
            </div>
            <div class="led-meter-container">
              <span>60/m</span>
              <div class="led-count-meter"></div>
            </div>
          </div>
          
          <div class="form-group">
            <label>Minimum Distance (cm)</label>
            <div class="slider-container">
              <input type="range" min="0" max="200" value="30" class="slider" id="minDist">
              <div class="slider-value" id="minDistValue">30</div>
            </div>
          </div>
          
          <div class="form-group">
            <label>Maximum Distance (cm)</label>
            <div class="slider-container">
              <input type="range" min="50" max="500" value="300" class="slider" id="maxDist">
              <div class="slider-value" id="maxDistValue">300</div>
            </div>
          </div>
          
<!-- Completely revised color picker for cross-platform compatibility -->
          <div class="form-group">
            <label>LED Color</label>
            <div class="color-selection-container">
              <!-- Color Preview and Custom Color Picker Button -->
              <div class="color-picker-wrapper">
                <div class="color-preview" id="colorPreview"></div>
                <button type="button" class="color-selector-button" id="colorSelectorButton">Tap to pick color</button>
              </div>
              
              <!-- RGB Sliders -->
              <div class="rgb-sliders">
                <div class="slider-container">
                  <span class="rgb-label r">R</span>
                  <input type="range" min="0" max="255" value="255" class="slider" id="redValue">
                  <div class="slider-value" id="redValueDisplay">255</div>
                </div>
                <div class="slider-container">
                  <span class="rgb-label g">G</span>
                  <input type="range" min="0" max="255" value="255" class="slider" id="greenValue">
                  <div class="slider-value" id="greenValueDisplay">255</div>
                </div>
                <div class="slider-container">
                  <span class="rgb-label b">B</span>
                  <input type="range" min="0" max="255" value="255" class="slider" id="blueValue">
                  <div class="slider-value" id="blueValueDisplay">255</div>
                </div>
              </div>
            </div>
            
            <!-- Custom Color Picker Modal -->
            <div id="colorPickerModal" class="color-picker-modal">
              <div class="color-picker-modal-content">
                <div class="color-picker-header">
                  <h3>Choose Color</h3>
                  <button type="button" class="color-picker-close">&times;</button>
                </div>
                <div class="color-picker-body">
                  <!-- Predefined color palette -->
                  <div class="color-palette">
                    <div class="color-palette-item" data-color="#FF0000" style="background-color: #FF0000;"></div>
                    <div class="color-palette-item" data-color="#00FF00" style="background-color: #00FF00;"></div>
                    <div class="color-palette-item" data-color="#0000FF" style="background-color: #0000FF;"></div>
                    <div class="color-palette-item" data-color="#FFFF00" style="background-color: #FFFF00;"></div>
                    <div class="color-palette-item" data-color="#FF00FF" style="background-color: #FF00FF;"></div>
                    <div class="color-palette-item" data-color="#00FFFF" style="background-color: #00FFFF;"></div>
                    <div class="color-palette-item" data-color="#FFFFFF" style="background-color: #FFFFFF;"></div>
                    <div class="color-palette-item" data-color="#FF8000" style="background-color: #FF8000;"></div>
                    <div class="color-palette-item" data-color="#8000FF" style="background-color: #8000FF;"></div>
                    <div class="color-palette-item" data-color="#00FF80" style="background-color: #00FF80;"></div>
                    <div class="color-palette-item" data-color="#FF0080" style="background-color: #FF0080;"></div>
                    <div class="color-palette-item" data-color="#0080FF" style="background-color: #0080FF;"></div>
                  </div>
                  
                  <!-- Native color input as backup -->
                  <div class="advanced-color-selection">
                    <label for="advancedColorPicker">Advanced color picker:</label>
                    <input type="color" id="advancedColorPicker" value="#FFFFFF">
                  </div>
                </div>
                <div class="color-picker-footer">
                  <button type="button" class="button color-picker-apply">Apply Color</button>
                </div>
              </div>
            </div>
          </div>

          <style>
            /* CSS for the new color picker implementation */
            .color-selection-container {
              display: flex;
              flex-direction: column;
              gap: 20px;
              margin-top: 10px;
            }
            
            .color-picker-wrapper {
              display: flex;
              flex-direction: column;
              align-items: center;
              gap: 12px;
            }
            
            .color-preview {
              width: 80px;
              height: 80px;
              border-radius: 50%;
              background-color: rgb(255, 255, 255);
              box-shadow: 0 0 15px rgba(67, 97, 238, 0.3);
              border: 3px solid var(--primary);
              transition: transform 0.2s ease;
            }
            
            .color-selector-button {
              background: var(--primary);
              color: white;
              border: none;
              padding: 8px 16px;
              border-radius: 20px;
              font-size: 14px;
              font-weight: 600;
              cursor: pointer;
              transition: all 0.2s ease;
            }
            
            .color-selector-button:hover {
              background: var(--primary-light);
              transform: translateY(-2px);
              box-shadow: 0 4px 10px rgba(0,0,0,0.2);
            }
            
            .rgb-sliders {
              width: 100%;
            }
            
            /* Modal styling */
            .color-picker-modal {
              display: none;
              position: fixed;
              top: 0;
              left: 0;
              width: 100%;
              height: 100%;
              background-color: rgba(0,0,0,0.8);
              z-index: 9999;
              justify-content: center;
              align-items: center;
            }
            
            .color-picker-modal-content {
              background-color: var(--bg-card);
              border-radius: var(--border-radius);
              width: 90%;
              max-width: 400px;
              box-shadow: 0 5px 20px rgba(0,0,0,0.4);
            }
            
            .color-picker-header {
              display: flex;
              justify-content: space-between;
              align-items: center;
              padding: 15px 20px;
              border-bottom: 1px solid #333;
            }
            
            .color-picker-header h3 {
              margin: 0;
              color: var(--primary);
              font-size: 18px;
            }
            
            .color-picker-close {
              background: none;
              border: none;
              color: var(--text-secondary);
              font-size: 24px;
              cursor: pointer;
            }
            
            .color-picker-body {
              padding: 20px;
            }
            
            .color-palette {
              display: grid;
              grid-template-columns: repeat(4, 1fr);
              gap: 10px;
              margin-bottom: 20px;
            }
            
            .color-palette-item {
              width: 50px;
              height: 50px;
              border-radius: 8px;
              cursor: pointer;
              border: 2px solid #444;
              transition: transform 0.2s ease, box-shadow 0.2s ease;
            }
            
            .color-palette-item:hover {
              transform: scale(1.1);
              box-shadow: 0 4px 10px rgba(0,0,0,0.3);
            }
            
            .color-palette-item.selected {
              border: 2px solid white;
              box-shadow: 0 0 8px rgba(255,255,255,0.5);
            }
            
            .advanced-color-selection {
              margin-top: 10px;
              display: flex;
              flex-direction: column;
              gap: 10px;
            }
            
            .advanced-color-selection label {
              font-size: 14px;
              color: var(--text-secondary);
            }
            
            .advanced-color-selection input[type="color"] {
              width: 100%;
              height: 40px;
              border: none;
              border-radius: var(--border-radius);
              cursor: pointer;
              background: transparent;
            }
            
            .color-picker-footer {
              padding: 15px 20px;
              border-top: 1px solid #333;
              text-align: right;
            }
            
            @media (min-width: 480px) {
              .color-selection-container {
                flex-direction: row;
                align-items: center;
              }
              
              .color-picker-wrapper {
                flex-shrink: 0;
              }
            }
          </style>

          <script>
          // Add these functions to your JavaScript section
          document.addEventListener('DOMContentLoaded', function() {
            // Elements
            const colorPreview = document.getElementById('colorPreview');
            const colorSelectorButton = document.getElementById('colorSelectorButton');
            const colorPickerModal = document.getElementById('colorPickerModal');
            const closeButton = document.querySelector('.color-picker-close');
            const applyButton = document.querySelector('.color-picker-apply');
            const paletteItems = document.querySelectorAll('.color-palette-item');
            const advancedColorPicker = document.getElementById('advancedColorPicker');
            
            // RGB sliders
            const redSlider = document.getElementById('redValue');
            const greenSlider = document.getElementById('greenValue');
            const blueSlider = document.getElementById('blueValue');
            const redDisplay = document.getElementById('redValueDisplay');
            const greenDisplay = document.getElementById('greenValueDisplay');
            const blueDisplay = document.getElementById('blueValueDisplay');
            
            let selectedColor = '#FFFFFF'; // Default color
            
            // Open color picker modal
            colorSelectorButton.addEventListener('click', function() {
              colorPickerModal.style.display = 'flex';
              
              // Set the advanced picker to current color
              const currentColor = rgbToHex(
                parseInt(redSlider.value), 
                parseInt(greenSlider.value), 
                parseInt(blueSlider.value)
              );
              advancedColorPicker.value = currentColor;
              
              // Update palette selected state
              updatePaletteSelection(currentColor);
            });
            
            // Close color picker modal
            closeButton.addEventListener('click', function() {
              colorPickerModal.style.display = 'none';
            });
            
            // Close modal if clicking outside content
            colorPickerModal.addEventListener('click', function(e) {
              if (e.target === colorPickerModal) {
                colorPickerModal.style.display = 'none';
              }
            });
            
            // Handle palette selections
            paletteItems.forEach(item => {
              item.addEventListener('click', function() {
                // Update selection visuals
                paletteItems.forEach(el => el.classList.remove('selected'));
                this.classList.add('selected');
                
                // Update selected color
                selectedColor = this.getAttribute('data-color');
                advancedColorPicker.value = selectedColor;
              });
            });
            
            // Handle advanced color picker changes
            advancedColorPicker.addEventListener('input', function() {
              selectedColor = this.value;
              updatePaletteSelection(selectedColor);
            });
            
            // Apply selected color
            applyButton.addEventListener('click', function() {
              // Close modal
              colorPickerModal.style.display = 'none';
              
              // Update RGB values from selected color
              updateRGBFromHex(selectedColor);
            });
            
            // Sync RGB sliders with color preview
            function syncSliders() {
              redSlider.addEventListener('input', updateColorFromSliders);
              greenSlider.addEventListener('input', updateColorFromSliders);
              blueSlider.addEventListener('input', updateColorFromSliders);
            }
            
            function updateColorFromSliders() {
              const r = parseInt(redSlider.value);
              const g = parseInt(greenSlider.value);
              const b = parseInt(blueSlider.value);
              
              redDisplay.textContent = r;
              greenDisplay.textContent = g;
              blueDisplay.textContent = b;
              
              updateColorPreview(r, g, b);
            }
            
            function updateColorPreview(r, g, b) {
              const red = r || parseInt(redSlider.value);
              const green = g || parseInt(greenSlider.value);
              const blue = b || parseInt(blueSlider.value);
              
              colorPreview.style.backgroundColor = `rgb(${red}, ${green}, ${blue})`;
              
              // Update LED visualizer if it exists
              const ledVisualizer = document.getElementById('ledVisualizer');
              if (ledVisualizer) {
                ledVisualizer.style.background = `rgb(${red}, ${green}, ${blue})`;
              }
            }
            
            function updateRGBFromHex(hexColor) {
              // Convert hex to RGB
              const r = parseInt(hexColor.substr(1, 2), 16);
              const g = parseInt(hexColor.substr(3, 2), 16);
              const b = parseInt(hexColor.substr(5, 2), 16);
              
              // Update sliders
              redSlider.value = r;
              greenSlider.value = g;
              blueSlider.value = b;
              
              // Update displays
              redDisplay.textContent = r;
              greenDisplay.textContent = g;
              blueDisplay.textContent = b;
              
              // Update preview
              updateColorPreview(r, g, b);
            }
            
            function rgbToHex(r, g, b) {
              return "#" + ((1 << 24) + (r << 16) + (g << 8) + b).toString(16).slice(1).toUpperCase();
            }
            
            function updatePaletteSelection(hexColor) {
              paletteItems.forEach(item => {
                const itemColor = item.getAttribute('data-color').toUpperCase();
                if (itemColor === hexColor.toUpperCase()) {
                  item.classList.add('selected');
                } else {
                  item.classList.remove('selected');
                }
              });
            }
            
            // Initialize the color preview and sliders sync
            updateColorPreview();
            syncSliders();
          });

          // Make sure to include this function in your existing load settings function
          function setInitialColorFromSettings(red, green, blue) {
            document.getElementById('redValue').value = red;
            document.getElementById('greenValue').value = green;
            document.getElementById('blueValue').value = blue;
            
            document.getElementById('redValueDisplay').textContent = red;
            document.getElementById('greenValueDisplay').textContent = green;
            document.getElementById('blueValueDisplay').textContent = blue;
            
            const colorPreview = document.getElementById('colorPreview');
            if (colorPreview) {
              colorPreview.style.backgroundColor = `rgb(${red}, ${green}, ${blue})`;
            }
            
            const ledVisualizer = document.getElementById('ledVisualizer');
            if (ledVisualizer) {
              ledVisualizer.style.background = `rgb(${red}, ${green}, ${blue})`;
            }
          }
          </script>
          
          <div class="form-group">
            <label>Brightness (0-255)</label>
            <div class="slider-container">
              <input type="range" min="0" max="255" value="255" class="slider" id="brightness">
              <div class="slider-value" id="brightnessValue">255</div>
            </div>
          </div>
        </div>
        
        <div id="advanced" class="tab-content">
        <div class="form-group">
          <label>Moving Light Span</label>
          <div class="slider-container">
            <input type="range" min="1" max="100" value="40" class="slider" id="lightSpan">
            <div class="slider-value" id="lightSpanValue">40</div>
          </div>
          <small class="input-description">
             Controls the width of the moving light effect. Higher values create a wider light trail along the LED strip, allowing the light to cover more LEDs as it moves.
          </small>
        </div>

        <div class="toggle-container">
          <span class="toggle-label">Background Mode</span>
          <label class="toggle-switch">
            <input type="checkbox" id="backgroundMode" onchange="setBackgroundMode(this.checked)">
            <span class="toggle-slider"></span>
          </label>

        </div>
        
        <div class="toggle-container">
          <span class="toggle-label">Directional Light</span>
          <label class="toggle-switch">
            <input type="checkbox" id="directionLight" onchange="setDirectionalLight(this.checked)">
            <span class="toggle-slider"></span>
          </label>
 
        </div>

        <div class="toggle-container">
          <span class="toggle-label">Motion Smoothing</span>
          <label class="toggle-switch">
            <input type="checkbox" id="motionSmoothing" onchange="setMotionSmoothing(this.checked)">
            <span class="toggle-slider"></span>
          </label>

        </div>

        <div class="form-group">
          <label>Position Smoothing Factor</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="20" class="slider" id="positionSmoothingFactor" 
                  onchange="updateMotionParam('positionSmoothingFactor', this.value * 0.01)">
            <div class="slider-value" id="positionSmoothingValue">0.20</div>
          </div>
          <small class="input-description">
            Controls how quickly the detected position adapts to new measurements. Lower values create smoother, more gradual position changes, while higher values make the response more responsive but potentially jittery.
          </small>
        </div>

        <div class="form-group">
          <label>Velocity Smoothing Factor</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="10" class="slider" id="velocitySmoothingFactor"
                  onchange="updateMotionParam('velocitySmoothingFactor', this.value * 0.01)">
            <div class="slider-value" id="velocitySmoothingValue">0.10</div>
          </div>
          <small class="input-description">
            Determines how smoothly the movement speed is calculated. Lower values create more stable velocity estimations, reducing sudden speed changes and making light movement more consistent.
          </small>
        </div>

        <div class="form-group">
          <label>Position Prediction Factor</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="50" class="slider" id="predictionFactor"
                  onchange="updateMotionParam('predictionFactor', this.value * 0.01)">
            <div class="slider-value" id="predictionValue">0.50</div>
          </div>
          <small class="input-description">
            Adjusts how far ahead the system predicts object movement. Higher values make the light anticipate movement more aggressively, creating a more fluid and responsive tracking effect.
          </small>
        </div>

        <div class="form-group">
          <label>Position P Gain</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="10" class="slider" id="positionPGain"
                  onchange="updateMotionParam('positionPGain', this.value * 0.01)">
            <div class="slider-value" id="positionPGainValue">0.10</div>
          </div>
          <small class="input-description">
            Proportional gain for position correction. Controls how strongly the system responds to position errors. Higher values increase responsiveness but may cause more oscillation.
          </small>
        </div>

        <div class="form-group">
          <label>Position I Gain</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="1" class="slider" id="positionIGain"
                  onchange="updateMotionParam('positionIGain', this.value * 0.001)">
            <div class="slider-value" id="positionIGainValue">0.01</div>
          </div>
          <small class="input-description">
            Integral gain for position correction. Helps eliminate steady-state errors by accumulating past position differences. Very low values are recommended to prevent overshooting.
          </small>
        </div>
        
        <div class="form-group">
          <label>Center Shift</label>
          <div class="slider-container">
            <input type="range" min="-100" max="100" value="0" class="slider" id="centerShift" 
                   onchange="setCenterShift(this.value)">
            <div class="slider-value" id="centerShiftValue">0</div>
          </div>
          <small class="input-description">
            Adjusts the horizontal position of the light effect along the LED strip. Positive values shift the light to the right, negative values shift to the left, allowing fine-tuning of the light's starting position.
          </small>
        </div>
        
        <div class="form-group">
          <label>Trail Length</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="0" class="slider" id="trailLength"
                   onchange="setTrailLength(this.value)">
            <div class="slider-value" id="trailLengthValue">0</div>
          </div>
          <small class="input-description">
            Controls the length of the fading light trail behind the main light effect. Higher values create a longer, more gradual fade-out effect, simulating motion blur or trailing lights.
          </small>
        </div>
      </div>
        <div id="effects" class="tab-content">
          <div class="form-group">
            <label>Light Mode</label>
            <select class="mode-selector" id="lightMode" onchange="setLightMode(this.value)">
              <option value="0">Standard (Moving Light)</option>
              <option value="4">Solid Color</option>
              <option value="1">Rainbow Effect</option>
              <option value="2">Color Wave</option>
              <option value="3">Breathing</option>
              <option value="5">Comet Trail</option>
              <option value="6">Pulse Waves</option>
              <option value="7">Fire Effect</option>
              <option value="8">Theater Chase</option>
              <option value="9">Dual Scanning</option>
              <option value="10">Motion Particles</option>
            </select>
            <small class="input-description" id="modeDescription">
              Standard mode follows your movement with a light that tracks distance.
            </small>
          </div>

          
          <div class="form-group">
            <label>Effect Speed</label>
            <div class="slider-container">
              <input type="range" min="1" max="100" value="50" class="slider" id="effectSpeed">
              <div class="slider-value" id="effectSpeedValue">50</div>
            </div>
          </div>
          
          <div class="form-group">
            <label>Effect Intensity</label>
            <div class="slider-container">
              <input type="range" min="1" max="100" value="50" class="slider" id="effectIntensity">
              <div class="slider-value" id="effectIntensityValue">50</div>
            </div>
          </div>
        </div>
        
        <button class="button" id="saveButton">Save Settings</button>

        
        <div class="distance-display">
          <div class="distance-label">Current Distance</div>
          <div class="distance-value" id="liveDistance">--<span class="distance-unit">cm</span></div>
          
          <div class="led-visualizer">
            <div class="led-active" id="ledVisualizer" style="width: 0%;"></div>
          </div>
        </div>
      </div>
    </div>
  </div>
  
  <div class="footer">
    <p class="branding">AmbiSense by Ravi Singh (TechPosts Media)</p>
    <p>Radar-Controlled LED System</p>
    <p>Copyright &copy; 2025 TechPosts Media. All rights reserved.</p>
  </div>
  
  <div class="settings-saved" id="settingsSaved">
    Settings saved successfully!
  </div>
  </content>

  <script>
    // Initialize values from server
    window.onload = function() {
      // Load all current settings first
      loadCurrentSettings();
      updateMotionSmoothingSliders();
      
      // Set up slider value displays
      const sliders = document.querySelectorAll('.slider');
      sliders.forEach(slider => {
        const valueDisplay = document.getElementById(slider.id + (slider.id.includes('Value') ? 'Display' : 'Value'));
        
        slider.oninput = function() {
          valueDisplay.textContent = this.value;
          
          // Handle RGB color preview updates
          if (['redValue', 'greenValue', 'blueValue'].includes(this.id)) {
            updateColorPreview();
          }
        };
        
        // Add change event for auto-save on slider release 
        slider.addEventListener('change', function() {
          if (['centerShift', 'trailLength'].includes(this.id)) {
            // These already have specific handlers with onchange
            return;
          }
          
          if (['positionSmoothingFactor','velocitySmoothingFactor', 'predictionFactor', 'positionPGain', 'positionIGain'].includes(this.id)) {
            // These have specific motion parameter handlers with onchange
            return;
          }
          
          // Auto-save when slider is released
          autoSaveChanges(this.id, this.value);
        });
      });
      
      // Set up LED count calculator
      const numLedsInput = document.getElementById('numLeds');
      numLedsInput.oninput = function() {
        // Assuming 60 LEDs per meter
        const lengthInMeters = (parseInt(this.value) / 60).toFixed(1);
        document.getElementById('ledDensity').textContent = lengthInMeters;
        
        // Update LED strip visualizer color
        updateColorPreview();
      };
      
      // Add change event to auto-save when input loses focus or enter is pressed
      numLedsInput.addEventListener('change', function() {
        autoSaveChanges('numLeds', this.value);
      });
      
      // Start live updates
      updateDistance();
    };
    
    // Auto-save changes without requiring the save button
    function autoSaveChanges(settingId, value) {
      let params = '';
      
      // Map control IDs to server parameter names
      switch(settingId) {
        case 'numLeds':
          params = 'numLeds=' + value;
          break;
        case 'minDist':
          params = 'minDist=' + value;
          break;
        case 'maxDist':
          params = 'maxDist=' + value;
          break;
        case 'brightness':
          params = 'brightness=' + value;
          break;
        case 'lightSpan':
          params = 'lightSpan=' + value;
          break;
        case 'redValue':
          params = 'redValue=' + value;
          break;
        case 'greenValue':
          params = 'greenValue=' + value;
          break;
        case 'blueValue':
          params = 'blueValue=' + value;
          break;
        default:
          console.log('Unknown setting ID: ' + settingId);
          return;
      }
      
      // Send to server
      fetch('/set?' + params)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error saving setting:', error);
        });
    }
    
    // Load current settings from the device
    function loadCurrentSettings() {
      fetch('/settings')
        .then(response => response.json())
        .then(settings => {
          // Apply settings to the form
          document.getElementById('numLeds').value = settings.numLeds;
          document.getElementById('ledDensity').textContent = (settings.numLeds / 60).toFixed(1);
          
          document.getElementById('minDist').value = settings.minDistance;
          document.getElementById('minDistValue').textContent = settings.minDistance;
          
          document.getElementById('maxDist').value = settings.maxDistance;
          document.getElementById('maxDistValue').textContent = settings.maxDistance;
          
          document.getElementById('brightness').value = settings.brightness;
          document.getElementById('brightnessValue').textContent = settings.brightness;
          
          document.getElementById('lightSpan').value = settings.movingLightSpan;
          document.getElementById('lightSpanValue').textContent = settings.movingLightSpan;
          
          document.getElementById('redValue').value = settings.redValue;
          document.getElementById('redValueDisplay').textContent = settings.redValue;
          
          document.getElementById('greenValue').value = settings.greenValue;
          document.getElementById('greenValueDisplay').textContent = settings.greenValue;
          
          document.getElementById('blueValue').value = settings.blueValue;
          document.getElementById('blueValueDisplay').textContent = settings.blueValue;
          
          // Advanced settings
          document.getElementById('lightMode').value = settings.lightMode;
          document.getElementById('backgroundMode').checked = settings.backgroundMode;
          document.getElementById('directionLight').checked = settings.directionLightEnabled;
          document.getElementById("centerShift").value = settings.centerShift;
          document.getElementById("centerShiftValue").textContent = settings.centerShift;
          document.getElementById('trailLength').value = settings.trailLength;
          document.getElementById('trailLengthValue').textContent = settings.trailLength;
          document.getElementById('effectSpeed').value = settings.effectSpeed;
          document.getElementById('effectSpeedValue').textContent = settings.effectSpeed;
          document.getElementById('effectIntensity').value = settings.effectIntensity;
          document.getElementById('effectIntensityValue').textContent = settings.effectIntensity;
          document.getElementById("positionSmoothingFactor").value = settings.positionSmoothingFactor * 100;
          document.getElementById("positionSmoothingValue").textContent = settings.positionSmoothingFactor.toFixed(2);

          document.getElementById("velocitySmoothingFactor").value = settings.velocitySmoothingFactor * 100;
          document.getElementById("velocitySmoothingValue").textContent = settings.velocitySmoothingFactor.toFixed(2);

// and so on...

          

          window.addEventListener('DOMContentLoaded', function() {
            // Set the initial mode description
            const lightMode = document.getElementById('lightMode');
            if (lightMode) {
              const event = new Event('change');
              lightMode.dispatchEvent(event);
            }
          });
          // Update color preview
          updateColorPreview();
          loadMotionSmoothingSettings();
          
          // Hide loading overlay
          document.getElementById('loadingOverlay').style.display = 'none';
        })
        .catch(error => {
          console.error('Error loading settings:', error);
          // Hide loading overlay
          document.getElementById('loadingOverlay').style.display = 'none';
        });
    }
    
    // Update color preview based on RGB sliders
    function updateColorPreview() {
      const red = document.getElementById('redValue').value;
      const green = document.getElementById('greenValue').value;
      const blue = document.getElementById('blueValue').value;
      
      const colorPreview = document.getElementById('colorPreview');
      colorPreview.style.backgroundColor = `rgb(${red}, ${green}, ${blue})`;
      
      // Also update the LED visualizer color
      const ledVisualizer = document.getElementById('ledVisualizer');
      ledVisualizer.style.background = `rgb(${red}, ${green}, ${blue})`;
    }
    
    // Tab navigation function
    function openTab(evt, tabName) {
      // Hide all tab content
      const tabContents = document.getElementsByClassName('tab-content');
      for (let i = 0; i < tabContents.length; i++) {
        tabContents[i].classList.remove('active');
      }
      
      // Remove active class from all tabs
      const tabs = document.getElementsByClassName('tab');
      for (let i = 0; i < tabs.length; i++) {
        tabs[i].classList.remove('active');
      }
      
      // Show the selected tab content and mark the button as active
      document.getElementById(tabName).classList.add('active');
      evt.currentTarget.classList.add('active');
    }
    
    // Functions for settings
    function setLightMode(mode) {
      fetch('/setLightMode?mode=' + mode)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting light mode:', error);
        });
    }
    
    function setBackgroundMode(enabled) {
      fetch('/setBackgroundMode?enabled=' + enabled)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting background mode:', error);
        });
    }
    
    function setDirectionalLight(enabled) {
      fetch('/setDirectionalLight?enabled=' + enabled)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting directional light:', error);
        });
    }
    
    function setCenterShift(value) {
      fetch('/setCenterShift?value=' + value)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting center shift:', error);
        });
    }
    
    function setTrailLength(value) {
      fetch('/setTrailLength?value=' + value)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting trail length:', error);
        });
    }
    
    // Motion smoothing functions
    function setMotionSmoothing(enabled) {
      fetch('/setMotionSmoothing?enabled=' + (enabled ? 'true' : 'false'))
        .then(response => {
          if (response.ok) {
            return response.json();
          }
          throw new Error('Network response was not ok');
        })
        .then(data => {
          if (data.status === 'success') {
            console.log("Motion smoothing " + (enabled ? "enabled" : "disabled"));
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting motion smoothing:', error);
        });
    }

    function updateMotionParam(param, value) {
      // Immediately update the display
      const displayElement = document.getElementById(param + 'Value');
      if (displayElement) {
        // Format the value based on the parameter
        let formattedValue;
        switch(param) {
          case 'positionIGain':
            formattedValue = (value).toFixed(3);
            break;
          default:
            formattedValue = (value).toFixed(2);
        }
        displayElement.textContent = formattedValue;
      }

      // Slider update (for sliders with different scaling)
      const sliderElement = document.getElementById(param);
      if (sliderElement) {
        switch(param) {
          case 'positionSmoothingFactor':
          case 'velocitySmoothingFactor':
          case 'predictionFactor':
          case 'positionPGain':
            sliderElement.value = Math.round(value * 100);
            break;
          case 'positionIGain':
            sliderElement.value = Math.round(value * 1000);
            break;
        }
      }

      // Send update to server
      fetch('/setMotionSmoothingParam?param=' + param + '&value=' + value)
        .then(response => {
          if (response.ok) {
            return response.json();
          }
          throw new Error('Network response was not ok');
        })
        .then(data => {
          if (data.status === 'success') {
            console.log("Updated " + param + " to " + value);
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error("Network error updating " + param, error);
        });
    }

    // Update slider displays for motion smoothing
    function updateMotionSmoothingSliders() {
      document.getElementById('positionSmoothingFactor').oninput = function() {
        document.getElementById('positionSmoothingValue').textContent = 
          (this.value * 0.01).toFixed(2);
      };
      
      document.getElementById('velocitySmoothingFactor').oninput = function() {
        document.getElementById('velocitySmoothingValue').textContent = 
          (this.value * 0.01).toFixed(2);
      };
      
      document.getElementById('predictionFactor').oninput = function() {
        document.getElementById('predictionValue').textContent = 
          (this.value * 0.01).toFixed(2);
      };
      
      document.getElementById('positionPGain').oninput = function() {
        document.getElementById('positionPGainValue').textContent = 
          (this.value * 0.01).toFixed(2);
      };
      
      document.getElementById('positionIGain').oninput = function() {
        document.getElementById('positionIGainValue').textContent = 
          (this.value * 0.001).toFixed(3);
      };
    }

    

    // Load motion smoothing settings from the server
    function loadMotionSmoothingSettings() {
      // This will be called by the loadCurrentSettings function
      fetch('/settings')
        .then(response => response.json())
        .then(settings => {
          // Set motion smoothing toggle
          document.getElementById('motionSmoothing').checked = settings.motionSmoothingEnabled;
          
          // Set position smoothing factor
          const posFactorSlider = document.getElementById('positionSmoothingFactor');
          posFactorSlider.value = Math.round(settings.positionSmoothingFactor * 100);
          document.getElementById('positionSmoothingValue').textContent = 
            settings.positionSmoothingFactor.toFixed(2);
          
          // Set velocity smoothing factor
          const velFactorSlider = document.getElementById('velocitySmoothingFactor');
          velFactorSlider.value = Math.round(settings.velocitySmoothingFactor * 100);
          document.getElementById('velocitySmoothingValue').textContent = 
            settings.velocitySmoothingFactor.toFixed(2);
          
          // Set prediction factor
          const predFactorSlider = document.getElementById('predictionFactor');
          predFactorSlider.value = Math.round(settings.predictionFactor * 100);
          document.getElementById('predictionValue').textContent = 
            settings.predictionFactor.toFixed(2);
          
          // Set position P gain
          const pGainSlider = document.getElementById('positionPGain');
          pGainSlider.value = Math.round(settings.positionPGain * 100);
          document.getElementById('positionPGainValue').textContent = 
            settings.positionPGain.toFixed(2);
          
          // Set position I gain
          const iGainSlider = document.getElementById('positionIGain');
          iGainSlider.value = Math.round(settings.positionIGain * 1000);
          document.getElementById('positionIGainValue').textContent = 
            settings.positionIGain.toFixed(3);
        })
        .catch(error => {
          console.error("Error loading motion smoothing settings:", error);
        });
    }
    
    // Save settings
    document.getElementById('saveButton').addEventListener('click', function() {
        const numLeds = document.getElementById('numLeds').value;
        const minDist = document.getElementById('minDist').value;
        const maxDist = document.getElementById('maxDist').value;
        const brightness = document.getElementById('brightness').value;
        const lightSpan = document.getElementById('lightSpan').value;
        const redValue = document.getElementById('redValue').value;
        const greenValue = document.getElementById('greenValue').value;
        const blueValue = document.getElementById('blueValue').value;
        
        // Save basic settings
        fetch('/set?numLeds=' + numLeds + 
                    '&minDist=' + minDist + 
                    '&maxDist=' + maxDist + 
                    '&brightness=' + brightness + 
                    '&lightSpan=' + lightSpan + 
                    '&redValue=' + redValue + 
                    '&greenValue=' + greenValue + 
                    '&blueValue=' + blueValue)
          .then(response => response.json())
          .then(data => {
            if (data.status === 'success') {
              // Show saved notification
              showSavedNotification();
            } else {
              console.error("Failed to save settings:", data);
            }
          })
          .catch(error => {
            console.error("Network error during settings save:", error);
          });
    });
    
    document.getElementById('lightMode').addEventListener('change', function() {
      const modeDescriptions = {
        '0': 'Standard mode follows your movement with a light that tracks distance.',
        '1': 'Rainbow effect shows a flowing spectrum of colors across the entire LED strip.',
        '2': 'Color Wave creates flowing waves of color that ripple across the strip.',
        '3': 'Breathing effect smoothly fades the LEDs in and out like breathing.',
        '4': 'Solid Color simply displays your selected color at a constant brightness.',
        '5': 'Comet Trail creates a moving light with a fading trail that follows motion.',
        '6': 'Pulse Waves sends pulses of light radiating outward from your position.',
        '7': 'Fire Effect simulates flickering flames with dynamic red and orange hues.',
        '8': 'Theater Chase creates a classic marquee effect with moving groups of lights.',
        '9': 'Dual Scanning shows two light bars scanning in opposite directions.',
        '10': 'Motion Particles spawns particle effects from your movement point.'
      };
      
      const description = document.getElementById('modeDescription');
      description.textContent = modeDescriptions[this.value] || '';
    });
    document.getElementById('effectSpeed').addEventListener('change', function() {
      const value = this.value;
      fetch('/setEffectSpeed?value=' + value)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting effect speed:', error);
        });
    });

    // Add handler for effect intensity slider
    document.getElementById('effectIntensity').addEventListener('change', function() {
      const value = this.value;
      fetch('/setEffectIntensity?value=' + value)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
          }
        })
        .catch(error => {
          console.error('Error setting effect intensity:', error);
        });
    });



    // Live distance updates
    function updateDistance() {
      fetch('/distance')
        .then(response => response.text())
        .then(distance => {
          distance = parseInt(distance);
          document.getElementById('liveDistance').innerHTML = distance + '<span class="distance-unit">cm</span>';
          
          // Update LED visualizer
          const minDist = parseInt(document.getElementById('minDist').value);
          const maxDist = parseInt(document.getElementById('maxDist').value);
          const percentage = Math.min(100, Math.max(0, ((distance - minDist) / (maxDist - minDist)) * 100));
          
          // Get the LED color based on current settings
          const ledVisualizer = document.getElementById('ledVisualizer');
          
          // If directional light is enabled, visualize it differently
          if (document.getElementById('directionLight').checked) {
            // Calculate direction based on distance change (simplified for visualization)
            const position = 100 - percentage;
            ledVisualizer.style.width = '30%';
            ledVisualizer.style.left = `${position - 15}%`;
          } else {
            ledVisualizer.style.width = (100 - percentage) + '%';
            ledVisualizer.style.left = '0';
          }
        })
        .catch(error => {
          console.error("Error fetching distance:", error);
        })
        .finally(() => {
          setTimeout(updateDistance, 500);
        });
    }
    
    // Show saved notification
    function showSavedNotification() {
      const notification = document.getElementById('settingsSaved');
      notification.classList.add('show');
      
      setTimeout(() => {
        notification.classList.remove('show');
      }, 3000);
    }
  </script>
</body>
</html>
  )rawliteral";
  
  server.send(200, "text/html", html);
}

void handleNetworkSettings() {
  if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("deviceName")) {
    // Get form data
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String deviceName = server.arg("deviceName");
    
    // Validate inputs
    if (ssid.length() == 0 || deviceName.length() == 0) {
      server.send(400, "text/html", "<h1>Error</h1><p>SSID and Device Name cannot be empty</p><a href='/network'>Back</a>");
      return;
    }
    
    // Save credentials to EEPROM
    wifiManager.saveWifiCredentials(ssid.c_str(), password.c_str(), deviceName.c_str());
    
    
    // Format hostname from the device name

      String hostname = wifiManager.getSanitizedHostname(deviceName.c_str());

      String html = "<!DOCTYPE html><html><head><title>Network Settings - AmbiSense</title>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body{font-family:Arial;margin:0;padding:20px;text-align:center;}";
      html += ".container{max-width:500px;margin:0 auto;}h1{color:#4361ee;}";
      html += ".info{background-color:#f0f0f0;padding:15px;border-radius:5px;margin:20px 0;}";
      html += ".countdown{font-size:24px;font-weight:bold;margin:20px 0;color:#4361ee;}</style>";
      html += "<script>var countdown=10;function updateCountdown(){";
      html += "document.getElementById('timer').innerText=countdown;countdown--;";
      html += "if(countdown<0){window.location.href='http://" + hostname + ".local';}";
      html += "else{setTimeout(updateCountdown,1000);}}";
      html += "window.onload=function(){updateCountdown();};</script></head>";
      html += "<body><div class='container'><h1>Network Settings Updated</h1>";
      html += "<div class='info'><p>The device will now restart and attempt to connect to your WiFi network.</p>";
      html += "<p>If successful, you'll be able to access it at:<br><strong>http://" + hostname + ".local</strong></p>";
      html += "<p>Device restarting in <span id='timer'>10</span> seconds...</p></div>";
      html += "<div id='countdown-info' class='countdown'></div></div></body></html>";
          
    server.send(200, "text/html", html);
    
    // Schedule restart
    delay(1000);
    ESP.restart();  
  } else {
    // Display network settings form
    char ssid[MAX_SSID_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
    
    wifiManager.loadWifiCredentials(ssid, password, deviceName);
    wifiManager.checkWifiConnection();
 
    
    // Generate mDNS info if in STA mode
    String mdnsHtml = "";
    if (wifiManager.getMode() == WIFI_MANAGER_MODE_STA) {
      mdnsHtml = "<div class=\"status-item\"><span class=\"status-label\">mDNS Name:</span><span class=\"status-value\"><a style='color:inherit;' href='http://" +
            wifiManager.getSanitizedHostname(deviceName) +
            ".local' target='_blank'>http://" +
            wifiManager.getSanitizedHostname(deviceName) +
            ".local</a></span></div>";

    }
    
    // Get the IP address from the WiFi manager
    String ipAddress = wifiManager.getIPAddress();
    // Determine current mode text
    String modeText;
    if (wifiManager.getMode() == WIFI_MANAGER_MODE_STA) {
      modeText = "Connected to Wi-Fi";
    } else if (wifiManager.getMode() == WIFI_MANAGER_MODE_FALLBACK) {
      modeText = "Fallback Mode";
    } else {
      modeText = "Access Point";
    }


    // HTML template for network settings page
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Network Settings - AmbiSense</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary: #4361ee;
      --primary-light: #4895ef;
      --success: #4cc9f0;
      --bg-dark: #121212;
      --bg-card: #1e1e1e;
      --text: #ffffff;
      --text-secondary: #b0b0b0;
      --border-radius: 12px;
      --shadow: 0 10px 20px rgba(0,0,0,0.3);
      --transition: all 0.3s ease;
    }
    
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background-color: var(--bg-dark);
      color: var(--text);
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      justify-content: space-between;
      padding: 20px;
      background: linear-gradient(135deg, #121212 0%, #2a2a2a 100%);
    }
    
    .container {
      max-width: 500px;
      margin: 0 auto;
      background: var(--bg-card);
      border-radius: var(--border-radius);
      padding: 20px;
      box-shadow: var(--shadow);
    }
    
    h1 {
      color: var(--primary);
      margin-bottom: 20px;
      text-align: center;
    }
    
    .input-group {
      margin-bottom: 20px;
    }
    
    label {
      display: block;
      margin-bottom: 5px;
      color: var(--text-secondary);
    }
    
    input[type="text"],
    input[type="password"] {
      width: 100%;
      padding: 10px;
      border-radius: 5px;
      border: 1px solid #444;
      background-color: #333;
      color: var(--text);
      font-size: 16px;
    }
    
    .status {
      background-color: rgba(0,0,0,0.2);
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
    }
    
    .status-item {
      display: flex;
      justify-content: space-between;
      margin-bottom: 10px;
    }
    
    .status-label {
      color: var(--text-secondary);
    }
    
    .status-value {
      color: var(--success);
      font-weight: bold;
    }
    
    .button {
      display: block;
      width: 100%;
      padding: 15px;
      background: linear-gradient(135deg, var(--primary) 0%, var(--primary-light) 100%);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      font-size: 16px;
      font-weight: 600;
      cursor: pointer;
      transition: var(--transition);
      margin-top: 30px;
      text-transform: uppercase;
      letter-spacing: 1px;
    }
    
    .button:hover {
      transform: translateY(-3px);
      box-shadow: 0 8px 20px rgba(67, 97, 238, 0.6);
    }
    
    .button-secondary {
      background: #444;
      margin-top: 10px;
    }
    
    .button-danger {
      background: linear-gradient(135deg, #ff4d4d 0%, #ff6666 100%);
    }
    
    .back-link {
      display: block;
      text-align: center;
      margin-top: 20px;
      color: var(--text-secondary);
      text-decoration: none;
    }
    
    .back-link:hover {
      color: var(--primary);
    }
    
    .tab-container {
      display: flex;
      margin-bottom: 20px;
      border-bottom: 1px solid #333;
    }

    .tab {
      padding: 10px 15px;
      cursor: pointer;
      background-color: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 14px;
      position: relative;
    }

    .tab.active {
      color: var(--primary);
    }

    .tab.active::after {
      content: '';
      position: absolute;
      bottom: -1px;
      left: 0;
      width: 100%;
      height: 2px;
      background-color: var(--primary);
    }
    
    .ssid-input-container {
      display: flex;
      gap: 10px;
      margin-bottom: 10px;
    }

    .scan-button {
      padding: 10px 15px;
      background: var(--primary);
      color: white;
      border: none;
      border-radius: var(--border-radius);
      cursor: pointer;
      font-weight: 600;
      flex-shrink: 0;
    }

    .scan-button:hover {
      background: var(--primary-light);
    }

    .network-dropdown {
      width: 100%;
      padding: 10px;
      border-radius: 5px;
      border: 1px solid #444;
      background-color: #333;
      color: var(--text);
      font-size: 16px;
      margin-bottom: 15px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Network Settings</h1>
    
    <div class="status">
      <div class="status-item">
        <span class="status-label">Current Mode:</span>
        <span class="status-value">)rawliteral" + modeText + R"rawliteral(</span>
      </div>
      <div class="status-item">
        <span class="status-label">IP Address:</span>
        <span class="status-value" id="current-ip">)rawliteral" + ipAddress + R"rawliteral(</span>
      </div>
      )rawliteral" + mdnsHtml + R"rawliteral(
    </div>
    
    <div class="tab-container">
      <button class="tab" onclick="window.location.href='/'">Basic</button>
      <button class="tab" onclick="window.location.href='/'">Advanced</button>
      <button class="tab" onclick="window.location.href='/'">Effects</button>
      <button class="tab active" onclick="window.location.href='/network'">Network</button>
    </div>
    
    <form method="post" action="/network">
      <div class="input-group">
        <label for="ssid">WiFi Network Name (SSID)</label>
        <div class="ssid-input-container">
          <input type="text" id="ssid" name="ssid" value="" placeholder="Enter network name" required>
          <button type="button" class="scan-button" onclick="scanNetworks()">Scan</button>
        </div>
        <select id="network-list" class="network-dropdown" style="display: none;" onchange="selectNetwork(this.value)">
          <option value="">Select a network...</option>
        </select>
      </div>

      <div class="input-group">
        <label for="password">WiFi Password</label>
        <input type="password" id="password" name="password" value="" placeholder="Enter network password">
      </div>

      <div class="input-group">
        <label for="deviceName">Device Name (used for mDNS address)</label>
        <input type="text" id="deviceName" name="deviceName" value="" placeholder="Enter device name" required>
        <small style="color: var(--text-secondary); font-size: 12px; margin-top: 5px; display: block;">
          Your device will be accessible at: http://ambisense-[name].local
        </small>
      </div>
      
      <button type="submit" class="button">Save Settings</button>
      <button type="button" class="button button-secondary" onclick="window.location.href='/'">Cancel</button>
    </form>
    
    <button type="button" class="button button-danger" style="margin-top: 30px;" onclick="if(confirm('Are you sure you want to reset all network settings?')) { window.location.href='/resetwifi'; }">Reset Network Settings</button>
    
    <a href="/" class="back-link">&larr; Back to main page</a>
  </div>
  <script>
  // Network scanning functions
  function scanNetworks() {
  // Show dropdown and scanning message
  document.getElementById('network-list').innerHTML = '<option value="">Scanning...</option>';
  document.getElementById('network-list').style.display = 'block';
  
  // Change scan button appearance while scanning
  const scanButton = document.querySelector('.scan-button');
  scanButton.disabled = true;
  scanButton.innerHTML = 'Scanning...';
  scanButton.style.opacity = '0.7';
  
 const scanTimeout = setTimeout(() => {
    scanButton.disabled = false;
    scanButton.innerHTML = 'Scan';
    scanButton.style.opacity = '1';
    document.getElementById('network-list').innerHTML = '<option value="">Scan timed out. Try again</option>';
  }, 15000); // 15 second timeout
  
  fetch('/scannetworks')
    .then(response => {
      if (!response.ok) {
        throw new Error(`Network response was not ok: ${response.status}`);
      }
      return response.json();
    })
    .then(networks => {
      clearTimeout(scanTimeout);
      scanButton.disabled = false;
      scanButton.innerHTML = 'Scan';
      scanButton.style.opacity = '1';
      
      if (networks.length === 0) {
        document.getElementById('network-list').innerHTML = '<option value="">No networks found</option>';
        return;
      }
      
      let options = '<option value="">Select a network...</option>';
      networks.forEach(network => {
        const signalStrength = getSignalStrengthLabel(network.rssi);
        const secureIcon = network.secure ? '📶🔒 ' : '📶 ';
        // Use textContent for safe HTML embedding
        const option = document.createElement('option');
        option.value = network.ssid;
        option.textContent = `${secureIcon}${network.ssid} (${signalStrength})`;
        options += option.outerHTML;
      });
      document.getElementById('network-list').innerHTML = options;
    })
    .catch(error => {
      clearTimeout(scanTimeout);
      console.error('Error scanning networks:', error);
      document.getElementById('network-list').innerHTML = '<option value="">Scan failed, try again</option>';
      scanButton.disabled = false;
      scanButton.innerHTML = 'Scan';
      scanButton.style.opacity = '1';
    });
}

  function selectNetwork(ssid) {
    if (ssid) {
      document.getElementById('ssid').value = ssid;
    }
  }

  function getSignalStrengthLabel(rssi) {
    if (rssi >= -50) return 'Excellent';
    if (rssi >= -60) return 'Good';
    if (rssi >= -70) return 'Fair';
    return 'Weak';
  }
</script>
</body>
</html>
    )rawliteral";
    
    server.send(200, "text/html", html);
  }
}

// Handler for wifi reset
void handleResetWifi() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Reset WiFi - AmbiSense</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <style>
        body { font-family: Arial; margin: 0; padding: 20px; text-align: center; }
        .container { max-width: 500px; margin: 0 auto; }
        h1 { color: #ff4d4d; }
        .info { background-color: #f0f0f0; padding: 15px; border-radius: 5px; margin: 20px 0; }
        .countdown { font-size: 24px; font-weight: bold; margin: 20px 0; color: #ff4d4d; }
      </style>
      <script>
        var countdown = 5;
        function updateCountdown() {
          document.getElementById('timer').innerText = countdown;
          countdown--;
          if (countdown < 0) {
            window.location.href = '/';
          } else {
            setTimeout(updateCountdown, 1000);
          }
        }
        window.onload = function() {
          updateCountdown();
        };
      </script>
    </head>
    <body>
      <div class="container">
        <h1>Network Settings Reset</h1>
        <div class="info">
          <p>All network settings have been reset.</p>
          <p>The device will restart in AP mode.</p>
          <p>Restarting in <span id="timer">5</span> seconds...</p>
        </div>
      </div>
    </body>
    </html>
  )rawliteral";
  
  server.send(200, "text/html", html);
  
  // Reset WiFi settings
  delay(1000);
  wifiManager.resetWifiSettings();
  // resetWifiSettings() will restart the device
}