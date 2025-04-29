#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "wifi_manager.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(WEB_SERVER_PORT);

// HTML templates stored in PROGMEM to save RAM
#include "html_templates.h"

void setupWebServer() {
  // Initialize SPIFFS with better error handling
  if(!SPIFFS.begin(true)) {
    Serial.println("ERROR: An error occurred while mounting SPIFFS");
    // Continue even if SPIFFS fails - we'll use PROGMEM for templates
    delay(1000); // Add a delay to ensure the error message is seen
  } else {
    Serial.println("SPIFFS mounted successfully");
    // Verify SPIFFS content (debug info)
    Serial.println("SPIFFS contents:");
    File root = SPIFFS.open("/");
    if (root && root.isDirectory()) {
      File file = root.openNextFile();
      while(file) {
        Serial.print("  ");
        Serial.print(file.name());
        Serial.print("\t");
        Serial.println(file.size());
        file = root.openNextFile();
      }
    } else {
      Serial.println("  No files found in SPIFFS");
    }
  }
  
  // Wait a moment to ensure WiFi is fully initialized
  delay(500);
  
  // Main page routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Add debug info
    Serial.println("Serving main page request from: " + request->client()->remoteIP().toString());
    
    // Try to deliver the main HTML template from PROGMEM
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", mainHtmlTemplate);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
    
    Serial.println("Main page served");
  });
  
  // API routes
  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *request) {
    bool settingsChanged = false;
    String response = "{\"status\":\"success\", \"changes\":[";
    bool firstChange = true;

    if (request->hasParam("numLeds")) {
      int newNumLeds = request->getParam("numLeds")->value().toInt();
      if (newNumLeds > 0 && newNumLeds <= 2000) {
        if (numLeds != newNumLeds) {
          numLeds = newNumLeds;
          settingsChanged = true;
          if (!firstChange) response += ",";
          response += "\"numLeds\"";
          firstChange = false;
        }
      }
    }
    
    if (request->hasParam("minDist")) {
      int newMinDist = request->getParam("minDist")->value().toInt();
      if (minDistance != newMinDist) {
        minDistance = newMinDist;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"minDistance\"";
        firstChange = false;
      }
    }
    
    if (request->hasParam("maxDist")) {
      int newMaxDist = request->getParam("maxDist")->value().toInt();
      if (maxDistance != newMaxDist) {
        maxDistance = newMaxDist;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"maxDistance\"";
        firstChange = false;
      }
    }
    
    if (request->hasParam("brightness")) {
      int newBrightness = request->getParam("brightness")->value().toInt();
      if (brightness != newBrightness) {
        brightness = newBrightness;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"brightness\"";
        firstChange = false;
      }
    }
    
    if (request->hasParam("lightSpan")) {
      int newLightSpan = request->getParam("lightSpan")->value().toInt();
      if (movingLightSpan != newLightSpan) {
        movingLightSpan = newLightSpan;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"movingLightSpan\"";
        firstChange = false;
      }
    }
    
    // Handle RGB color values
    if (request->hasParam("redValue")) {
      int newRedValue = request->getParam("redValue")->value().toInt();
      if (redValue != newRedValue) {
        redValue = newRedValue;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"redValue\"";
        firstChange = false;
      }
    }
    
    if (request->hasParam("greenValue")) {
      int newGreenValue = request->getParam("greenValue")->value().toInt();
      if (greenValue != newGreenValue) {
        greenValue = newGreenValue;
        settingsChanged = true;
        if (!firstChange) response += ",";
        response += "\"greenValue\"";
        firstChange = false;
      }
    }
    
    if (request->hasParam("blueValue")) {
      int newBlueValue = request->getParam("blueValue")->value().toInt();
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
    
    request->send(200, "application/json", response);
  });
  
  server.on("/distance", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(currentDistance));
  });
  
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    
    request->send(200, "application/json", json);
  });
  
  server.on("/setLightMode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      lightMode = request->getParam("mode")->value().toInt();
      saveSettings();
      updateLEDConfig();
      
      // Return JSON response
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Light mode updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing mode parameter\"}");
    }
  });
  
  server.on("/setDirectionalLight", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("enabled")) {
      directionLightEnabled = request->getParam("enabled")->value() == "true";
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Directional light setting updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
    }
  });
  
  server.on("/setCenterShift", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      centerShift = request->getParam("value")->value().toInt();
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Center shift updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
    }
  });
  
  server.on("/setTrailLength", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      trailLength = request->getParam("value")->value().toInt();
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Trail length updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
    }
  });
  
  server.on("/setBackgroundMode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("enabled")) {
      backgroundMode = request->getParam("enabled")->value() == "true";
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Background mode updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
    }
  });
  
  // Network routes
  server.on("/network", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Display network settings form
    char ssid[MAX_SSID_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
    
    wifiManager.loadWifiCredentials(ssid, password, deviceName);
    wifiManager.checkWifiConnection();
   
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
    
    // Generate mDNS info if in STA mode
    String mdnsHtml = "";
    if (wifiManager.getMode() == WIFI_MANAGER_MODE_STA) {
      mdnsHtml = "<div class=\"status-item\"><span class=\"status-label\">mDNS Name:</span><span class=\"status-value\"><a style='color:inherit;' href='http://" +
            wifiManager.getSanitizedHostname(deviceName) +
            ".local' target='_blank'>http://" +
            wifiManager.getSanitizedHostname(deviceName) +
            ".local</a></span></div>";
    }
    
    // Start with template
    String html = networkSettingsTemplate;
    
    // Replace placeholders
    html.replace("%MODE_TEXT%", modeText);
    html.replace("%IP_ADDRESS%", ipAddress);
    html.replace("%MDNS_HTML%", mdnsHtml);
    html.replace("%SSID%", ssid);
    html.replace("%DEVICE_NAME%", deviceName);
    
    request->send(200, "text/html", html);
  });
  
  // Handle network form submission via POST
  server.on("/network", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && 
        request->hasParam("password", true) && 
        request->hasParam("deviceName", true)) {
      
      String ssid = request->getParam("ssid", true)->value();
      String password = request->getParam("password", true)->value();
      String deviceName = request->getParam("deviceName", true)->value();
      
      // Validate inputs
      if (ssid.length() == 0 || deviceName.length() == 0) {
        request->send(400, "text/html", "<h1>Error</h1><p>SSID and Device Name cannot be empty</p><a href='/network'>Back</a>");
        return;
      }
      
      // Save credentials to EEPROM
      wifiManager.saveWifiCredentials(ssid.c_str(), password.c_str(), deviceName.c_str());
      
      // Format hostname from the device name
      String hostname = wifiManager.getSanitizedHostname(deviceName.c_str());
      
      String html = networkRestartTemplate;
      // Replace placeholders
      html.replace("%HOSTNAME%", hostname);
      
      request->send(200, "text/html", html);
      
      // Schedule restart (done in the main loop)
      // Don't restart here in the handler!
    } else {
      request->redirect("/network");
    }
  });
  
  server.on("/resetwifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", wifiResetTemplate);
    
    // Schedule WiFi reset for the main loop
    // Don't reset here in the handler!
  });
  
  server.on("/scannetworks", HTTP_GET, [](AsyncWebServerRequest *request) {
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
    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
    response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    response->addHeader("Pragma", "no-cache");
    response->addHeader("Expires", "-1");
    request->send(response);
  });
  
  // Motion smoothing routes
  server.on("/setMotionSmoothing", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("enabled")) {
      motionSmoothingEnabled = (request->getParam("enabled")->value() == "true");
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Motion smoothing setting updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
    }
  });
  
  server.on("/setEffectSpeed", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      effectSpeed = request->getParam("value")->value().toInt();
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Effect speed updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
    }
  });
  
  server.on("/setEffectIntensity", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value")) {
      effectIntensity = request->getParam("value")->value().toInt();
      saveSettings();
      
      request->send(200, "application/json", "{\"status\":\"success\",\"message\":\"Effect intensity updated\"}");
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
    }
  });
  
  server.on("/setMotionSmoothingParam", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("param") && request->hasParam("value")) {
      String param = request->getParam("param")->value();
      float value = request->getParam("value")->value().toFloat();
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
        request->send(400, "application/json", 
          "{\"error\":\"Invalid parameter\",\"param\":\"" + param + "\"}");
        return;
      }

      if (validParam) {
        // Immediately save to ensure persistence
        saveSettings();
        
        // Prepare a JSON response with the actual constrained value
        String jsonResponse = "{\"param\":\"" + param + "\",\"value\":" + 
          String(value, 3) + ",\"status\":\"success\"}";
        
        request->send(200, "application/json", jsonResponse);
        
        // Debug output
        Serial.println("Updated motion smoothing parameters:");
        Serial.printf("- Position Smoothing Factor: %.2f\n", positionSmoothingFactor);
        Serial.printf("- Velocity Smoothing Factor: %.2f\n", velocitySmoothingFactor);
        Serial.printf("- Prediction Factor: %.2f\n", predictionFactor);
        Serial.printf("- Position P Gain: %.2f\n", positionPGain);
        Serial.printf("- Position I Gain: %.3f\n", positionIGain);
      }
    } else {
      request->send(400, "application/json", 
        "{\"error\":\"Missing parameter or value\"}");
    }
  });
  
  // Add a fallback route for serving a simple HTML page if the main template is too large
  server.on("/simple", HTTP_GET, [](AsyncWebServerRequest *request) {
    const char* simpleHtml = "<html><head><title>AmbiSense</title></head>"
                            "<body style='text-align:center;font-family:Arial;background:#121212;color:white;'>"
                            "<h1>AmbiSense</h1>"
                            "<p>Radar-Controlled LED Strip</p>"
                            "<p><a href='/network' style='color:#4361ee;'>Network Settings</a></p>"
                            "</body></html>";
    request->send(200, "text/html", simpleHtml);
  });
  
  // Add a static file server for any files in SPIFFS
  server.serveStatic("/", SPIFFS, "/");
  
  // Handle 404 Not Found
  server.onNotFound([](AsyncWebServerRequest *request) {
    Serial.println("404 Not Found: " + request->url());
    request->send(404, "text/plain", "Not found");
  });
  
  // Start server after a slight delay to ensure all other subsystems are initialized
  delay(100);
  server.begin();
  Serial.println("HTTP server started on port " + String(WEB_SERVER_PORT));
}