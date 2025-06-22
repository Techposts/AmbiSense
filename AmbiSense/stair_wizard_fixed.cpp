#include "stair_wizard_fixed.h"
#include "espnow_manager.h"
#include "eeprom_manager.h"
#include "config.h"
#include <WiFi.h>

// Global wizard state
SimpleWizardState wizardState;

// Memory management constants
const size_t MAX_HTML_SIZE = 8192;
const size_t MAX_JSON_SIZE = 2048;
const unsigned long WIZARD_SESSION_TIMEOUT = 300000; // 5 minutes

void initWizard() {
  // Reset wizard state safely
  wizardState = SimpleWizardState();
  wizardState.sessionStart = millis();
  wizardState.isActive = true;
  
  Serial.println("Wizard: Initialized safely");
}

String generateWizardStartPage() {
  // Check available heap before generating large HTML
  if (ESP.getFreeHeap() < 12000) {
    Serial.println("Wizard: Insufficient memory for HTML generation");
    return generateErrorResponse("Insufficient memory available");
  }
  
  SafeStringBuilder html(MAX_HTML_SIZE);
  
  // Build HTML in chunks to manage memory
  html.append(F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"));
  html.append(F("<title>AmbiSense Stair Wizard</title>"));
  html.append(F("<meta name='viewport' content='width=device-width, initial-scale=1'>"));
  
  // Simplified CSS (much smaller than original)
  html.append(F("<style>"));
  html.append(F("*{margin:0;padding:0;box-sizing:border-box}"));
  html.append(F("body{font-family:Arial,sans-serif;background:#121212;color:#fff;padding:20px}"));
  html.append(F(".container{max-width:600px;margin:0 auto;background:#1e1e1e;border-radius:10px;padding:30px}"));
  html.append(F(".layout-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(250px,1fr));gap:20px;margin:20px 0}"));
  html.append(F(".layout-card{background:#333;padding:20px;border-radius:8px;cursor:pointer;border:2px solid transparent;text-align:center}"));
  html.append(F(".layout-card:hover{border-color:#4361ee}"));
  html.append(F(".layout-card.selected{border-color:#4cc9f0;background:#2a4a4a}"));
  html.append(F(".btn{padding:12px 24px;background:#4361ee;color:white;border:none;border-radius:6px;cursor:pointer;font-size:16px}"));
  html.append(F(".btn:disabled{background:#666;cursor:not-allowed}"));
  html.append(F("</style></head><body>"));
  
  if (html.hasOverflowed()) {
    return generateErrorResponse("HTML too large for memory");
  }
  
  // HTML body content
  html.append(F("<div class='container'>"));
  html.append(F("<h1>🏠 AmbiSense Stair Setup Wizard</h1>"));
  html.append(F("<p>Choose your stair layout for automatic configuration</p>"));
  
  html.append(F("<div class='layout-grid'>"));
  
  // Straight layout option
  html.append(F("<div class='layout-card' onclick='selectLayout(\"straight\")' id='layout-straight'>"));
  html.append(F("<h3>Straight Stairs</h3>"));
  html.append(F("<p>Single sensor, simple setup</p>"));
  html.append(F("</div>"));
  
  // L-shaped layout option
  html.append(F("<div class='layout-card' onclick='selectLayout(\"l-shape\")' id='layout-l-shape'>"));
  html.append(F("<h3>L-Shaped Stairs</h3>"));
  html.append(F("<p>Corner turn, 2-3 sensors</p>"));
  html.append(F("</div>"));
  
  // U-shaped layout option
  html.append(F("<div class='layout-card' onclick='selectLayout(\"u-shape\")' id='layout-u-shape'>"));
  html.append(F("<h3>U-Shaped Stairs</h3>"));
  html.append(F("<p>180° turn, 3-4 sensors</p>"));
  html.append(F("</div>"));
  
  html.append(F("</div>"));
  
  // Buttons
  html.append(F("<div style='margin-top:30px;text-align:center'>"));
  html.append(F("<button class='btn' onclick='cancelWizard()' style='margin-right:15px;background:#666'>Cancel</button>"));
  html.append(F("<button class='btn' onclick='nextStep()' id='nextBtn' disabled>Next: Discovery</button>"));
  html.append(F("</div>"));
  
  html.append(F("</div>"));
  
  // Simplified JavaScript
  html.append(F("<script>"));
  html.append(F("let selectedLayout=null;"));
  html.append(F("function selectLayout(layout){"));
  html.append(F("document.querySelectorAll('.layout-card').forEach(c=>c.classList.remove('selected'));"));
  html.append(F("document.getElementById('layout-'+layout).classList.add('selected');"));
  html.append(F("selectedLayout=layout;"));
  html.append(F("document.getElementById('nextBtn').disabled=false;"));
  html.append(F("}"));
  html.append(F("function nextStep(){"));
  html.append(F("if(!selectedLayout){alert('Please select a layout');return;}"));
  html.append(F("fetch('/wizard/layout',{method:'POST',headers:{'Content-Type':'application/json'},"));
  html.append(F("body:JSON.stringify({layout:selectedLayout})})"));
  html.append(F(".then(r=>r.text()).then(html=>document.body.innerHTML=html)"));
  html.append(F(".catch(e=>{alert('Error: '+e);});"));
  html.append(F("}"));
  html.append(F("function cancelWizard(){window.location.href='/mesh';}"));
  html.append(F("</script></body></html>"));
  
  if (html.hasOverflowed()) {
    return generateErrorResponse("HTML generation exceeded memory limits");
  }
  
  return html.build();
}

String processLayoutStep(const String& layout) {
  if (!isWizardSessionValid()) {
    return generateWizardErrorPage("Wizard session expired. Please restart the wizard.");
  }
  
  // Set layout with validation
  if (layout == "straight") {
    wizardState.selectedLayout = LAYOUT_STRAIGHT;
    // Straight stairs don't need wizard - redirect to main config
    SafeStringBuilder html(2048);
    html.append(F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"));
    html.append(F("<title>Straight Stairs Setup</title>"));
    html.append(F("<style>body{font-family:Arial;background:#121212;color:#fff;padding:40px;text-align:center;}"));
    html.append(F(".info{background:#4361ee;padding:20px;border-radius:8px;margin:20px auto;max-width:500px;}"));
    html.append(F("button{background:#4cc9f0;color:#000;border:none;padding:12px 24px;border-radius:6px;cursor:pointer;margin:10px;}"));
    html.append(F("</style></head><body>"));
    html.append(F("<div class='info'><h2>✅ Straight Stairs Setup</h2>"));
    html.append(F("<p>Straight stairs use simple single-device setup. No wizard needed!</p>"));
    html.append(F("<p>Configure your basic settings using the main interface.</p></div>"));
    html.append(F("<button onclick='window.location.href=\"/\"'>Go to Main Configuration</button>"));
    html.append(F("</body></html>"));
    return html.build();
  } else if (layout == "l-shape") {
    wizardState.selectedLayout = LAYOUT_L_SHAPED;
  } else if (layout == "u-shape") {
    wizardState.selectedLayout = LAYOUT_U_SHAPED;
  } else {
    return generateWizardErrorPage("Invalid layout selection: " + layout);
  }
  
  Serial.printf("Wizard: Layout selected - %d\n", wizardState.selectedLayout);
  
  // Return device discovery page for L-shaped and U-shaped stairs
  return handleDeviceDiscovery();
}

String handleDeviceDiscovery() {
  SafeStringBuilder html(MAX_HTML_SIZE);
  
  html.append(F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"));
  html.append(F("<title>Device Discovery</title>"));
  html.append(F("<style>"));
  html.append(F("*{margin:0;padding:0;box-sizing:border-box}"));
  html.append(F("body{font-family:Arial,sans-serif;background:#121212;color:#fff;padding:20px}"));
  html.append(F(".container{max-width:600px;margin:0 auto;background:#1e1e1e;border-radius:10px;padding:30px}"));
  html.append(F(".status{text-align:center;padding:40px}"));
  html.append(F(".btn{padding:12px 24px;background:#4361ee;color:white;border:none;border-radius:6px;cursor:pointer}"));
  html.append(F("</style></head><body>"));
  
  html.append(F("<div class='container'>"));
  html.append(F("<h1>🔍 Device Discovery</h1>"));
  html.append(F("<div class='status' id='status'>"));
  html.append(F("<div style='font-size:48px;margin-bottom:20px'>🔄</div>"));
  html.append(F("<div>Auto-scanning for AmbiSense devices...</div>"));
  html.append(F("</div>"));
  
  html.append(F("<div style='text-align:center;margin-top:30px' id='controls'>"));
  html.append(F("<button class='btn' onclick='startDiscovery()'>Scan Again</button>"));
  html.append(F("<button class='btn' onclick='skipDiscovery()' style='margin-left:15px;background:#666'>Skip</button>"));
  html.append(F("</div>"));
  
  html.append(F("</div>"));
  
  // Fixed discovery JavaScript
  html.append(F("<script>"));
  html.append(F("let isScanning = false;"));
  html.append(F("function startDiscovery(){"));
  html.append(F("if(isScanning) return;"));
  html.append(F("isScanning = true;"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"font-size:48px;margin-bottom:20px\">🔄</div><div>Scanning for AmbiSense devices...</div>';"));
  html.append(F("console.log('Starting discovery scan...');"));
  html.append(F("fetch('/wizard/discover',{method:'POST',headers:{'Content-Type':'application/json'}})"));
  html.append(F(".then(response => {"));
  html.append(F("console.log('Response status:', response.status);"));
  html.append(F("if(!response.ok) throw new Error('Network response was not ok');"));
  html.append(F("return response.json();"));
  html.append(F("})"));
  html.append(F(".then(data=>{"));
  html.append(F("isScanning = false;"));
  html.append(F("console.log('Discovery result:', data);"));
  html.append(F("if(data && data.length > 0){"));
  html.append(F("showDevices(data);"));
  html.append(F("}else{"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"color:#ff6b6b;text-align:center\">❌ No AmbiSense devices found<br><small style=\"color:#999\">Make sure slave devices are powered on and in AP mode</small></div>';"));
  html.append(F("}"));
  html.append(F("})"));
  html.append(F(".catch(error => {"));
  html.append(F("isScanning = false;"));
  html.append(F("console.error('Discovery error:', error);"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"color:#ff6b6b;text-align:center\">❌ Discovery failed<br><small style=\"color:#999\">Please check network connection and try again</small></div>';"));
  html.append(F("});"));
  html.append(F("}"));
  html.append(F("function showDevices(devices){"));
  html.append(F("let html='<h3 style=\"text-align:center;color:#4cc9f0\">✅ Found '+devices.length+' AmbiSense device'+(devices.length>1?'s':'')+'</h3>';"));
  html.append(F("devices.forEach((d,i) => {"));
  html.append(F("html += '<div style=\"background:#333;margin:10px 0;padding:15px;border-radius:5px;border-left:4px solid #4cc9f0\">';"));
  html.append(F("html += '<div style=\"font-weight:bold;color:#fff;margin-bottom:5px\">'+d.name+'</div>';"));
  html.append(F("html += '<div style=\"font-size:12px;color:#999;font-family:monospace\">MAC: '+d.mac+'</div>';"));
  html.append(F("html += '<div style=\"font-size:12px;color:#999\">Signal: '+d.signal+' ('+d.rssi+'dBm)</div>';"));
  html.append(F("html += '<div style=\"font-size:12px;color:#4cc9f0\">Role: '+d.role+'</div>';"));
  html.append(F("html += '</div>';"));
  html.append(F("});"));
  html.append(F("html+='<div style=\"text-align:center;margin-top:20px\">';"));
  html.append(F("html+='<button class=\"btn\" onclick=\"applyConfig()\" style=\"margin-right:10px\">Apply Configuration</button>';"));
  html.append(F("html+='<button class=\"btn\" onclick=\"startDiscovery()\" style=\"background:#666\">Scan Again</button>';"));
  html.append(F("html+='</div>';"));
  html.append(F("document.getElementById('status').innerHTML=html;"));
  html.append(F("}"));
  html.append(F("function skipDiscovery(){"));
  html.append(F("if(confirm('Skip device discovery? You can configure devices manually later.')){"));
  html.append(F("applyConfig();"));
  html.append(F("}"));
  html.append(F("}"));
  html.append(F("function applyConfig(){"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"font-size:48px;margin-bottom:20px\">⚙️</div><div>Applying configuration...</div>';"));
  html.append(F("fetch('/wizard/apply',{method:'POST',headers:{'Content-Type':'application/json'}})"));
  html.append(F(".then(r=>r.json())"));
  html.append(F(".then(data=>{"));
  html.append(F("if(data.status==='success'){"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"color:#4cc9f0;text-align:center\">✅ Configuration applied successfully!</div>';"));
  html.append(F("setTimeout(() => window.location.href='/mesh', 2000);"));
  html.append(F("}else{"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"color:#ff6b6b;text-align:center\">❌ Configuration failed: '+(data.message||'Unknown error')+'</div>';"));
  html.append(F("}"));
  html.append(F("})"));
  html.append(F(".catch(e=>{"));
  html.append(F("console.error('Apply config error:', e);"));
  html.append(F("document.getElementById('status').innerHTML='<div style=\"color:#ff6b6b;text-align:center\">❌ Error applying configuration</div>';"));
  html.append(F("});"));
  html.append(F("}"));
  html.append(F("setTimeout(startDiscovery,1000);"));  // Auto-start discovery after 1 second
  html.append(F("</script></body></html>"));
  
  return html.build();
}

String generateDiscoveryResults() {
  if (!isWizardSessionValid()) {
    return generateErrorResponse("Session expired");
  }
  
  // Simplified device discovery
  SafeStringBuilder json(MAX_JSON_SIZE);
  json.append(F("{\"devices\":["));
  
  // Scan for WiFi networks (simplified)
  int networkCount = WiFi.scanNetworks(false, false, false, 100); // Shorter timeout
  int foundDevices = 0;
  
  for (int i = 0; i < networkCount && foundDevices < 3; i++) {
    String ssid = WiFi.SSID(i);
    
    if (ssid.indexOf("AmbiSense") >= 0) {
      if (foundDevices > 0) json.append(",");
      
      // Simplified device info
      json.append(F("{\"name\":\""));
      json.append(ssid);
      json.append(F("\",\"mac\":\"XX:XX:XX:XX:XX:XX\",\"role\":\""));
      json.append(foundDevices == 0 ? F("Master") : F("Slave"));
      json.append(F("\"}"));
      
      foundDevices++;
    }
  }
  
  WiFi.scanDelete(); // Clean up
  
  json.append(F("],\"count\":"));
  json.append(String(foundDevices));
  json.append(F("}"));
  
  wizardState.discoveredDeviceCount = foundDevices;
  
  return json.build();
}

bool applyLayoutConfiguration(StairLayout layout, int deviceCount) {
  if (!isWizardSessionValid()) {
    return false;
  }
  
  Serial.printf("Wizard: Applying configuration for layout %d with %d devices\n", layout, deviceCount);
  
  // Apply simplified configuration based on layout
  switch (layout) {
    case LAYOUT_STRAIGHT:
      // Single device, no special configuration needed
      setSensorPriorityMode(SENSOR_PRIORITY_MOST_RECENT);
      break;
      
    case LAYOUT_L_SHAPED:
      // Enable zone-based priority for L-shaped stairs
      setSensorPriorityMode(SENSOR_PRIORITY_ZONE_BASED);
      
      // Set LED distribution if multiple devices
      if (deviceCount > 1) {
        setLEDSegmentMode(LED_SEGMENT_MODE_DISTRIBUTED);
        setLEDSegmentInfo(0, wizardState.totalLeds / deviceCount, wizardState.totalLeds);
      }
      break;
      
    case LAYOUT_U_SHAPED:
      // Zone-based with enhanced corner handling
      setSensorPriorityMode(SENSOR_PRIORITY_ZONE_BASED);
      
      if (deviceCount > 2) {
        setLEDSegmentMode(LED_SEGMENT_MODE_DISTRIBUTED);
        setLEDSegmentInfo(0, wizardState.totalLeds / deviceCount, wizardState.totalLeds);
      }
      break;
      
    default:
      return false;
  }
  
  // Save configuration
  saveSystemSettings(); // From eeprom_manager
  
  Serial.println("Wizard: Configuration applied successfully");
  return true;
}

bool validateWizardConfiguration() {
  return isWizardSessionValid() && 
         wizardState.selectedLayout >= LAYOUT_STRAIGHT && 
         wizardState.selectedLayout <= LAYOUT_CUSTOM &&
         wizardState.totalLeds > 0 && 
         wizardState.totalLeds <= MAX_SUPPORTED_LEDS;
}

void resetWizardState() {
  wizardState = SimpleWizardState();
  Serial.println("Wizard: State reset");
}

bool isWizardSessionValid() {
  if (!wizardState.isActive) {
    return false;
  }
  
  unsigned long elapsed = millis() - wizardState.sessionStart;
  if (elapsed > WIZARD_SESSION_TIMEOUT) {
    resetWizardState();
    return false;
  }
  
  return true;
}

String generateErrorResponse(const String& message) {
  return "{\"status\":\"error\",\"message\":\"" + message + "\"}";
}

String generateWizardErrorPage(const String& message) {
  SafeStringBuilder html(2048);
  html.append(F("<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"));
  html.append(F("<title>Wizard Error</title>"));
  html.append(F("<style>body{font-family:Arial;background:#121212;color:#fff;padding:40px;text-align:center;}"));
  html.append(F(".error{background:#ff4444;padding:20px;border-radius:8px;margin:20px auto;max-width:500px;}"));
  html.append(F("button{background:#4361ee;color:white;border:none;padding:12px 24px;border-radius:6px;cursor:pointer;margin:10px;}"));
  html.append(F("</style></head><body>"));
  html.append(F("<div class='error'><h2>⚠️ Wizard Error</h2><p>"));
  html.append(message);
  html.append(F("</p></div>"));
  html.append(F("<button onclick='window.location.href=\"/wizard\"'>Restart Wizard</button>"));
  html.append(F("<button onclick='window.location.href=\"/\"'>Back to Main</button>"));
  html.append(F("</body></html>"));
  return html.build();
}

String generateSuccessResponse(const String& message) {
  return "{\"status\":\"success\",\"message\":\"" + message + "\"}";
}