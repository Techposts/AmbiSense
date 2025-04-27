#include <WebServer.h>
#include <ESPmDNS.h>
#include <pgmspace.h>  // Added for PROGMEM support
#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "wifi_manager.h"

// Global web server instance
WebServer server(WEB_SERVER_PORT);

// Version number for ETag generation
#define WEB_UI_VERSION "1.1.0"

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
void handleSetMotionSmoothing();
void handleSetMotionSmoothingParam();
void handleSetEffectSpeed();
void handleSetEffectIntensity();
void handleCSS();
void handleJS();

// Define the minified CSS content
static const char style_css[] PROGMEM = R"rawliteral(
:root{--primary:#4361ee;--primary-light:#4895ef;--success:#4cc9f0;--bg-dark:#121212;--bg-card:#1e1e1e;--text:#fff;--text-secondary:#b0b0b0;--border-radius:8px}*{margin:0;padding:0;box-sizing:border-box}body{font-family:system-ui,-apple-system,BlinkMacSystemFont,sans-serif;background-color:var(--bg-dark);color:var(--text);min-height:100vh;display:flex;flex-direction:column;padding:20px}.container{background:var(--bg-card);border-radius:var(--border-radius);box-shadow:0 4px 12px rgba(0,0,0,.3);width:100%;max-width:420px;margin:0 auto;overflow:hidden}.header{background:var(--primary);padding:20px;text-align:center}.content{padding:20px}.form-group{margin-bottom:16px}.form-group label{display:block;margin-bottom:6px;font-size:14px;color:var(--text-secondary)}.slider-container{display:flex;align-items:center;gap:10px}.slider{flex:1;-webkit-appearance:none;height:6px;border-radius:3px;background:#333;outline:0}.slider::-webkit-slider-thumb{-webkit-appearance:none;width:16px;height:16px;border-radius:50%;background:var(--primary);cursor:pointer}.slider::-moz-range-thumb{width:16px;height:16px;border-radius:50%;background:var(--primary);cursor:pointer}.slider-value{min-width:45px;padding:4px 8px;border-radius:var(--border-radius);background:#333;text-align:center;font-size:14px}.rgb-label{display:inline-block;width:20px;text-align:center;font-weight:700;margin-right:8px}.rgb-label.r{color:#f55}.rgb-label.g{color:#5f5}.rgb-label.b{color:#55f}input[type=number]{padding:8px;border-radius:var(--border-radius);background:#333;border:none;color:var(--text);font-size:14px;text-align:center;width:70px}.button{display:block;width:100%;padding:12px;background:var(--primary);color:#fff;border:none;border-radius:var(--border-radius);font-size:16px;font-weight:600;cursor:pointer;margin-top:20px}.distance-display{margin-top:20px;text-align:center;padding:15px;border-radius:var(--border-radius);background:#292929}.distance-label{font-size:14px;margin-bottom:8px;color:var(--text-secondary)}.distance-value{font-size:30px;font-weight:700;color:var(--success)}.distance-unit{font-size:14px;opacity:.7;margin-left:3px}.led-visualizer{height:12px;background:#333;margin-top:12px;border-radius:6px;overflow:hidden;position:relative}.led-active{position:absolute;height:100%;border-radius:6px;transition:width .3s ease,left .3s ease}.settings-saved{position:fixed;top:20px;right:20px;padding:10px 15px;background:var(--success);color:#fff;border-radius:var(--border-radius);transform:translateX(200%);transition:transform .3s ease}.settings-saved.show{transform:translateX(0)}.footer{text-align:center;padding:15px 0;margin-top:30px;font-size:12px;color:var(--text-secondary)}.tab-container{display:flex;margin-bottom:15px;border-bottom:1px solid #333}.tab{padding:8px 12px;cursor:pointer;background-color:transparent;border:none;color:var(--text-secondary);font-size:14px;position:relative}.tab.active{color:var(--primary)}.tab.active::after{content:'';position:absolute;bottom:-1px;left:0;width:100%;height:2px;background-color:var(--primary)}.tab-content{display:none}.tab-content.active{display:block}.toggle-container{display:flex;justify-content:space-between;align-items:center;margin-bottom:16px}.toggle-label{font-size:14px;color:var(--text-secondary)}.toggle-switch{position:relative;display:inline-block;width:50px;height:26px}.toggle-switch input{opacity:0;width:0;height:0}.toggle-slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#333;transition:.3s;border-radius:34px}.toggle-slider:before{position:absolute;content:"";height:18px;width:18px;left:4px;bottom:4px;background-color:#fff;transition:.3s;border-radius:50%}input:checked+.toggle-slider{background-color:var(--primary)}input:checked+.toggle-slider:before{transform:translateX(24px)}.mode-selector{background-color:#333;color:var(--text);border:none;padding:8px;border-radius:var(--border-radius);width:100%;margin-bottom:16px;font-size:14px}.color-preview{width:30px;height:30px;border-radius:50%;border:2px solid #555;margin-right:10px}.rgb-container{display:flex;align-items:center}.loading-overlay{position:fixed;top:0;left:0;width:100%;height:100%;background:rgba(0,0,0,.7);display:flex;justify-content:center;align-items:center;z-index:100}.spinner{width:40px;height:40px;border:4px solid rgba(0,0,0,.1);border-radius:50%;border-top-color:var(--primary);animation:spin 1s linear infinite}@keyframes spin{to{transform:rotate(360deg)}}.input-description{font-size:12px;color:var(--text-secondary);margin-top:4px;display:block}
)rawliteral";
// Define the minified JavaScript content
static const char script_js[] PROGMEM = R"rawliteral(
window.onload=function(){loadCurrentSettings();updateMotionSmoothingSliders();const sliders=document.querySelectorAll('.slider');sliders.forEach(slider=>{const valueDisplay=document.getElementById(slider.id+(slider.id.includes('Value')?'Display':'Value'));slider.oninput=function(){valueDisplay.textContent=this.value;if(['redValue','greenValue','blueValue'].includes(this.id)){updateColorPreview()}};slider.addEventListener('change',function(){if(['centerShift','trailLength'].includes(this.id)){return}if(['positionSmoothingFactor','velocitySmoothingFactor','predictionFactor','positionPGain','positionIGain'].includes(this.id)){return}autoSaveChanges(this.id,this.value)})});const numLedsInput=document.getElementById('numLeds');numLedsInput.oninput=function(){const lengthInMeters=(parseInt(this.value)/60).toFixed(1);document.getElementById('ledDensity').textContent=lengthInMeters;updateColorPreview()};numLedsInput.addEventListener('change',function(){autoSaveChanges('numLeds',this.value)});updateDistance()};function autoSaveChanges(settingId,value){let params='';switch(settingId){case'numLeds':params='numLeds='+value;break;case'minDist':params='minDist='+value;break;case'maxDist':params='maxDist='+value;break;case'brightness':params='brightness='+value;break;case'lightSpan':params='lightSpan='+value;break;case'redValue':params='redValue='+value;break;case'greenValue':params='greenValue='+value;break;case'blueValue':params='blueValue='+value;break;default:console.log('Unknown setting ID: '+settingId);return}fetch('/set?'+params).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error saving setting:',error)})}function loadCurrentSettings(){fetch('/settings').then(response=>response.json()).then(settings=>{document.getElementById('numLeds').value=settings.numLeds;document.getElementById('ledDensity').textContent=(settings.numLeds/60).toFixed(1);document.getElementById('minDist').value=settings.minDistance;document.getElementById('minDistValue').textContent=settings.minDistance;document.getElementById('maxDist').value=settings.maxDistance;document.getElementById('maxDistValue').textContent=settings.maxDistance;document.getElementById('brightness').value=settings.brightness;document.getElementById('brightnessValue').textContent=settings.brightness;document.getElementById('lightSpan').value=settings.movingLightSpan;document.getElementById('lightSpanValue').textContent=settings.movingLightSpan;document.getElementById('redValue').value=settings.redValue;document.getElementById('redValueDisplay').textContent=settings.redValue;document.getElementById('greenValue').value=settings.greenValue;document.getElementById('greenValueDisplay').textContent=settings.greenValue;document.getElementById('blueValue').value=settings.blueValue;document.getElementById('blueValueDisplay').textContent=settings.blueValue;document.getElementById('lightMode').value=settings.lightMode;document.getElementById('backgroundMode').checked=settings.backgroundMode;document.getElementById('directionLight').checked=settings.directionLightEnabled;document.getElementById("centerShift").value=settings.centerShift;document.getElementById("centerShiftValue").textContent=settings.centerShift;document.getElementById('trailLength').value=settings.trailLength;document.getElementById('trailLengthValue').textContent=settings.trailLength;document.getElementById('effectSpeed').value=settings.effectSpeed;document.getElementById('effectSpeedValue').textContent=settings.effectSpeed;document.getElementById('effectIntensity').value=settings.effectIntensity;document.getElementById('effectIntensityValue').textContent=settings.effectIntensity;document.getElementById("positionSmoothingFactor").value=settings.positionSmoothingFactor*100;document.getElementById("positionSmoothingValue").textContent=settings.positionSmoothingFactor.toFixed(2);document.getElementById("velocitySmoothingFactor").value=settings.velocitySmoothingFactor*100;document.getElementById("velocitySmoothingValue").textContent=settings.velocitySmoothingFactor.toFixed(2);document.getElementById("predictionFactor").value=settings.predictionFactor*100;document.getElementById("predictionValue").textContent=settings.predictionFactor.toFixed(2);document.getElementById("positionPGain").value=settings.positionPGain*100;document.getElementById("positionPGainValue").textContent=settings.positionPGain.toFixed(2);document.getElementById("positionIGain").value=settings.positionIGain*1000;document.getElementById("positionIGainValue").textContent=settings.positionIGain.toFixed(3);document.getElementById('motionSmoothing').checked=settings.motionSmoothingEnabled;updateColorPreview();const lightMode=document.getElementById('lightMode');if(lightMode){updateModeDescription(lightMode.value)}document.getElementById('loadingOverlay').style.display='none'}).catch(error=>{console.error('Error loading settings:',error);document.getElementById('loadingOverlay').style.display='none'})}function updateColorPreview(){const red=document.getElementById('redValue').value;const green=document.getElementById('greenValue').value;const blue=document.getElementById('blueValue').value;const colorPreview=document.getElementById('colorPreview');colorPreview.style.backgroundColor=`rgb(${red}, ${green}, ${blue})`;const ledVisualizer=document.getElementById('ledVisualizer');ledVisualizer.style.background=`rgb(${red}, ${green}, ${blue})`}function openTab(evt,tabName){const tabContents=document.getElementsByClassName('tab-content');for(let i=0;i<tabContents.length;i++){tabContents[i].classList.remove('active')}const tabs=document.getElementsByClassName('tab');for(let i=0;i<tabs.length;i++){tabs[i].classList.remove('active')}document.getElementById(tabName).classList.add('active');evt.currentTarget.classList.add('active')}function setLightMode(mode){updateModeDescription(mode);fetch('/setLightMode?mode='+mode).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting light mode:',error)})}function updateModeDescription(mode){const modeDescriptions={'0':'Standard mode follows your movement with a light that tracks distance.','1':'Rainbow effect shows a flowing spectrum of colors across the entire LED strip.','2':'Color Wave creates flowing waves of color that ripple across the strip.','3':'Breathing effect smoothly fades the LEDs in and out like breathing.','4':'Solid Color simply displays your selected color at a constant brightness.','5':'Comet Trail creates a moving light with a fading trail that follows motion.','6':'Pulse Waves sends pulses of light radiating outward from your position.','7':'Fire Effect simulates flickering flames with dynamic red and orange hues.','8':'Theater Chase creates a classic marquee effect with moving groups of lights.','9':'Dual Scanning shows two light bars scanning in opposite directions.','10':'Motion Particles spawns particle effects from your movement point.'};const description=document.getElementById('modeDescription');description.textContent=modeDescriptions[mode]||''}function setBackgroundMode(enabled){fetch('/setBackgroundMode?enabled='+enabled).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting background mode:',error)})}function setDirectionalLight(enabled){fetch('/setDirectionalLight?enabled='+enabled).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting directional light:',error)})}function setCenterShift(value){fetch('/setCenterShift?value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting center shift:',error)})}function setTrailLength(value){fetch('/setTrailLength?value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting trail length:',error)})}function setMotionSmoothing(enabled){fetch('/setMotionSmoothing?enabled='+(enabled?'true':'false')).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting motion smoothing:',error)})}function updateMotionParam(param,value){const displayElement=document.getElementById(param+'Value');if(displayElement){let formattedValue;switch(param){case'positionIGain':formattedValue=(value).toFixed(3);break;default:formattedValue=(value).toFixed(2)}displayElement.textContent=formattedValue}fetch('/setMotionSmoothingParam?param='+param+'&value='+value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error("Error updating "+param,error)})}function updateMotionSmoothingSliders(){document.getElementById('positionSmoothingFactor').oninput=function(){document.getElementById('positionSmoothingValue').textContent=(this.value*0.01).toFixed(2)};document.getElementById('velocitySmoothingFactor').oninput=function(){document.getElementById('velocitySmoothingValue').textContent=(this.value*0.01).toFixed(2)};document.getElementById('predictionFactor').oninput=function(){document.getElementById('predictionValue').textContent=(this.value*0.01).toFixed(2)};document.getElementById('positionPGain').oninput=function(){document.getElementById('positionPGainValue').textContent=(this.value*0.01).toFixed(2)};document.getElementById('positionIGain').oninput=function(){document.getElementById('positionIGainValue').textContent=(this.value*0.001).toFixed(3)}}document.addEventListener('DOMContentLoaded',function(){document.getElementById('effectSpeed').addEventListener('change',function(){fetch('/setEffectSpeed?value='+this.value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting effect speed:',error)})});document.getElementById('effectIntensity').addEventListener('change',function(){fetch('/setEffectIntensity?value='+this.value).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}}).catch(error=>{console.error('Error setting effect intensity:',error)})});document.getElementById('saveButton').addEventListener('click',function(){const numLeds=document.getElementById('numLeds').value;const minDist=document.getElementById('minDist').value;const maxDist=document.getElementById('maxDist').value;const brightness=document.getElementById('brightness').value;const lightSpan=document.getElementById('lightSpan').value;const redValue=document.getElementById('redValue').value;const greenValue=document.getElementById('greenValue').value;const blueValue=document.getElementById('blueValue').value;fetch('/set?numLeds='+numLeds+'&minDist='+minDist+'&maxDist='+maxDist+'&brightness='+brightness+'&lightSpan='+lightSpan+'&redValue='+redValue+'&greenValue='+greenValue+'&blueValue='+blueValue).then(response=>response.json()).then(data=>{if(data.status==='success'){showSavedNotification()}else{console.error("Failed to save settings:",data)}}).catch(error=>{console.error("Network error during settings save:",error)})})});function updateDistance(){fetch('/distance').then(response=>response.text()).then(distance=>{distance=parseInt(distance);document.getElementById('liveDistance').innerHTML=distance+'<span class="distance-unit">cm</span>';const minDist=parseInt(document.getElementById('minDist').value);const maxDist=parseInt(document.getElementById('maxDist').value);const percentage=Math.min(100,Math.max(0,((distance-minDist)/(maxDist-minDist))*100));const ledVisualizer=document.getElementById('ledVisualizer');if(document.getElementById('directionLight').checked){const position=100-percentage;ledVisualizer.style.width='30%';ledVisualizer.style.left=`${position-15}%`}else{ledVisualizer.style.width=(100-percentage)+'%';ledVisualizer.style.left='0'}}).catch(error=>{console.error("Error fetching distance:",error)}).finally(()=>{setTimeout(updateDistance,150)});} function showSavedNotification(){const notification=document.getElementById('settingsSaved');notification.classList.add('show');setTimeout(()=>{notification.classList.remove('show')},2000)}
)rawliteral";


void setupWebServer() {
  // Static content with appropriate cache headers
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCSS);
  server.on("/script.js", HTTP_GET, handleJS);
  
  // API endpoints (no caching)
  server.on("/set", handleSet);
  server.on("/distance", handleDistance);
  server.on("/settings", handleSettings);
  server.on("/setLightMode", handleSetLightMode);
  server.on("/setDirectionalLight", handleSetDirectionalLight);
  server.on("/setCenterShift", handleSetCenterShift);
  server.on("/setTrailLength", handleSetTrailLength);
  server.on("/setBackgroundMode", handleSetBackgroundMode);
  
  // Network configuration routes
  server.on("/network", handleNetworkSettings);
  server.on("/resetwifi", handleResetWifi);
  server.on("/scannetworks", handleScanNetworks);
  
  // Motion and effect settings
  server.on("/setMotionSmoothing", handleSetMotionSmoothing);
  server.on("/setMotionSmoothingParam", handleSetMotionSmoothingParam);
  server.on("/setEffectSpeed", handleSetEffectSpeed);
  server.on("/setEffectIntensity", handleSetEffectIntensity);
  
  // Start server
  server.begin();
  Serial.println("Web server started on port " + String(WEB_SERVER_PORT));
}

void handleWebServer() {
  server.handleClient();
}

// Helper function to send responses with appropriate cache headers
void sendCachedResponse(const String& contentType, const String& content, bool shouldCache = true) {
  if (shouldCache) {
    // Generate ETag
    String etag = "W/\"" + String(WEB_UI_VERSION) + "\"";
    
    // Check if client sent If-None-Match header that matches our ETag
    if (server.header("If-None-Match") == etag) {
      server.send(304, "", ""); // Not Modified
      return;
    }
    
    server.sendHeader("ETag", etag);
    server.sendHeader("Cache-Control", "max-age=86400"); // Cache for 24 hours
  } else {
    // For dynamic content, prevent caching
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
  }
  
  server.send(200, contentType, content);
}

// Helper function to send PROGMEM content with appropriate cache headers
void sendCachedProgmemResponse(const String& contentType, const char* content, bool shouldCache = true) {
  if (shouldCache) {
    // Generate ETag
    String etag = "W/\"" + String(WEB_UI_VERSION) + "\"";
    
    // Check if client sent If-None-Match header that matches our ETag
    if (server.header("If-None-Match") == etag) {
      server.send(304, "", ""); // Not Modified
      return;
    }
    
    server.sendHeader("ETag", etag);
    server.sendHeader("Cache-Control", "max-age=86400"); // Cache for 24 hours
  } else {
    // For dynamic content, prevent caching
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
  }
  
  server.send_P(200, contentType.c_str(), content);
}

// Handler for serving the CSS file
void handleCSS() {
  sendCachedProgmemResponse("text/css", style_css, true);
}

// Handler for serving the JavaScript file
void handleJS() {
  sendCachedProgmemResponse("application/javascript", script_js, true);
}
// Handle scanning for WiFi networks
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
  
  // Send response with appropriate headers for dynamic content
  sendCachedResponse("application/json", json, false);
}

// Handle Distance API (returns real-time distance)
void handleDistance() {
  // Send current distance directly as text for minimum processing overhead
  sendCachedResponse("text/plain", String(currentDistance), false);
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
  
  // Send with no-cache headers as this is dynamic data
  sendCachedResponse("application/json", json, false);
}

// Handle Light Mode Setting
void handleSetLightMode() {
  if (server.hasArg("mode")) {
    lightMode = server.arg("mode").toInt();
    saveSettings();
    updateLEDConfig();
    
    // Return JSON response instead of redirect
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Light mode updated\"}", false);
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
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Directional light setting updated\"}", false);
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
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Center shift updated\"}", false);
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
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Trail length updated\"}", false);
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
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Background mode updated\"}", false);
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
  
  sendCachedResponse("application/json", response, false);
}

// New handler for motion smoothing enable/disable
void handleSetMotionSmoothing() {
  if (server.hasArg("enabled")) {
    motionSmoothingEnabled = (server.arg("enabled") == "true");
    saveSettings();
    
    // Return JSON response instead of redirect
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Motion smoothing setting updated\"}", false);
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing enabled parameter\"}");
  }
}

void handleSetEffectSpeed() {
  if (server.hasArg("value")) {
    effectSpeed = server.arg("value").toInt();
    saveSettings();
    
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Effect speed updated\"}", false);
  } else {
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"Missing value parameter\"}");
  }
}

void handleSetEffectIntensity() {
  if (server.hasArg("value")) {
    effectIntensity = server.arg("value").toInt();
    saveSettings();
    
    sendCachedResponse("application/json", "{\"status\":\"success\",\"message\":\"Effect intensity updated\"}", false);
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
      
      sendCachedResponse("application/json", jsonResponse, false);
      
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
void handleNetworkSettings() {
  if (server.hasArg("ssid") && server.hasArg("password") && server.hasArg("deviceName")) {
    // Get and validate form data
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    String deviceName = server.arg("deviceName");
    
    // Input validation
    if (ssid.isEmpty() || deviceName.isEmpty()) {
      server.send(400, "text/html", 
        "<h1>Error</h1><p>SSID and Device Name cannot be empty</p><a href='/network'>Back</a>");
      return;
    }
    
    // Sanitize and save credentials
    String hostname = wifiManager.getSanitizedHostname(deviceName.c_str());
    wifiManager.saveWifiCredentials(ssid.c_str(), password.c_str(), deviceName.c_str());
    
    // Prepare restart confirmation HTML
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Network Settings - AmbiSense</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background-color: #121212; 
      color: white; 
      text-align: center; 
      padding: 20px; 
      max-width: 500px; 
      margin: 0 auto; 
    }
    .container {
      background-color: #1e1e1e;
      border-radius: 10px;
      padding: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.3);
    }
    .countdown { 
      font-size: 24px; 
      color: #4cc9f0; 
      margin: 20px 0; 
    }
    a { color: #4361ee; text-decoration: none; }
  </style>
  <script>
    var countdown = 10;
    function updateCountdown() {
      var timerEl = document.getElementById('timer');
      var redirectLink = document.getElementById('redirect-link');
      
      timerEl.textContent = countdown;
      
      if (countdown <= 0) {
        window.location.href = 'http://)rawliteral" + hostname + R"rawliteral(.local';
      } else {
        redirectLink.href = 'http://)rawliteral" + hostname + R"rawliteral(.local';
        countdown--;
        setTimeout(updateCountdown, 1000);
      }
    }
    window.onload = updateCountdown;
  </script>
</head>
<body>
  <div class="container">
    <h1>Network Settings Updated</h1>
    <p>The device will restart and connect to your WiFi network.</p>
    <p>Access the device at: 
      <a id="redirect-link" href="#" style="word-break: break-all;">
        http://)rawliteral" + hostname + R"rawliteral(.local
      </a>
    </p>
    <div class="countdown">Restarting in <span id="timer">10</span> seconds...</div>
  </div>
</body>
</html>
)rawliteral";
    
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
    
    // Determine connection status
    String modeText, ipAddress, mdnsHtml;
    switch (wifiManager.getMode()) {
      case WIFI_MANAGER_MODE_STA:
        modeText = "Connected to Wi-Fi";
        ipAddress = wifiManager.getIPAddress();
        mdnsHtml = "<div class=\"status-item\"><span class=\"status-label\">mDNS Name:</span>"
                   "<span class=\"status-value\"><a href='http://" + 
                   wifiManager.getSanitizedHostname(deviceName) + 
                   ".local'>" + 
                   wifiManager.getSanitizedHostname(deviceName) + 
                   ".local</a></span></div>";
        break;
      case WIFI_MANAGER_MODE_FALLBACK:
        modeText = "Fallback Mode";
        ipAddress = wifiManager.getIPAddress();
        break;
      default:
        modeText = "Access Point";
        ipAddress = wifiManager.getIPAddress();
    }

    // Network settings form HTML with full network scanning functionality
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
        <span class="status-value">)rawliteral" + ipAddress + R"rawliteral(</span>
      </div>
      )rawliteral" + mdnsHtml + R"rawliteral(
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
    </form>
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
    
    sendCachedResponse("text/html", html, false);
  }
}

void handleResetWifi() {
  // Simplified reset WiFi handler
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Reset WiFi - AmbiSense</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { 
      font-family: Arial, sans-serif; 
      background-color: #121212; 
      color: white; 
      text-align: center; 
      display: flex; 
      justify-content: center; 
      align-items: center; 
      height: 100vh; 
      margin: 0; 
    }
    .container {
      background-color: #1e1e1e;
      padding: 30px;
      border-radius: 15px;
      box-shadow: 0 10px 20px rgba(0,0,0,0.3);
    }
    .countdown {
      font-size: 48px;
      color: #ff4d4d;
      margin: 20px 0;
    }
  </style>
  <script>
    let countdown = 5;
    function updateCountdown() {
      const timerEl = document.getElementById('timer');
      timerEl.textContent = countdown;
      
      if (countdown <= 0) {
        window.location.href = '/';
      } else {
        countdown--;
        setTimeout(updateCountdown, 1000);
      }
    }
    window.onload = updateCountdown;
  </script>
</head>
<body>
  <div class="container">
    <h1>Network Settings Reset</h1>
    <p>All network settings have been cleared.</p>
    <p>Device will restart in Access Point mode.</p>
    <div class="countdown"><span id="timer">5</span></div>
  </div>
</body>
</html>
)rawliteral";
  
  sendCachedResponse("text/html", html, false);
  
  // Reset WiFi settings with a short delay
  delay(1000);
  wifiManager.resetWifiSettings();
}
// Replace the handleRoot function in web_interface.cpp with this optimized version
void handleRoot() {
  // Minified HTML content with external CSS and JS references
  const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <title>AmbiSense | TechPosts Media</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <link rel="stylesheet" href="/style.css">
</head>
<body>
  <div class="loading-overlay" id="loadingOverlay">
    <div class="spinner"></div>
  </div>

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
          <div class="slider-container">
            <input type="number" min="1" max="2000" id="numLeds" value="300">
            <div class="slider-value"><span id="ledDensity">5</span>m</div>
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
        
        <div class="form-group">
          <label>LED Color</label>
          <div class="rgb-container">
            <div class="color-preview" id="colorPreview"></div>
            <div class="rgb-sliders" style="flex-grow: 1;">
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
        </div>
        
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
          <small class="input-description">Controls the width of the moving light effect</small>
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
        </div>

        <div class="form-group">
          <label>Velocity Smoothing Factor</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="10" class="slider" id="velocitySmoothingFactor"
                  onchange="updateMotionParam('velocitySmoothingFactor', this.value * 0.01)">
            <div class="slider-value" id="velocitySmoothingValue">0.10</div>
          </div>
        </div>

        <div class="form-group">
          <label>Position Prediction Factor</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="50" class="slider" id="predictionFactor"
                  onchange="updateMotionParam('predictionFactor', this.value * 0.01)">
            <div class="slider-value" id="predictionValue">0.50</div>
          </div>
        </div>

        <div class="form-group">
          <label>Position P Gain</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="10" class="slider" id="positionPGain"
                  onchange="updateMotionParam('positionPGain', this.value * 0.01)">
            <div class="slider-value" id="positionPGainValue">0.10</div>
          </div>
        </div>

        <div class="form-group">
          <label>Position I Gain</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="1" class="slider" id="positionIGain"
                  onchange="updateMotionParam('positionIGain', this.value * 0.001)">
            <div class="slider-value" id="positionIGainValue">0.01</div>
          </div>
        </div>
        
        <div class="form-group">
          <label>Center Shift</label>
          <div class="slider-container">
            <input type="range" min="-100" max="100" value="0" class="slider" id="centerShift" 
                   onchange="setCenterShift(this.value)">
            <div class="slider-value" id="centerShiftValue">0</div>
          </div>
        </div>
        
        <div class="form-group">
          <label>Trail Length</label>
          <div class="slider-container">
            <input type="range" min="0" max="100" value="0" class="slider" id="trailLength"
                   onchange="setTrailLength(this.value)">
            <div class="slider-value" id="trailLengthValue">0</div>
          </div>
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
  
  <div class="footer">
    <p>AmbiSense by Ravi Singh (TechPosts Media)</p>
    <p>Copyright &copy; 2025 TechPosts Media. All rights reserved.</p>
  </div>
  
  <div class="settings-saved" id="settingsSaved">
    Settings saved successfully!
  </div>

  <script src="/script.js"></script>
</body>
</html>
)rawliteral";

  // Send the HTML with appropriate cache headers to allow browser caching
  sendCachedResponse("text/html", html, true);
}

