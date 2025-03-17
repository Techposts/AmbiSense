#include <WebServer.h>
#include "config.h"
#include "web_interface.h"
#include "eeprom_manager.h"
#include "led_controller.h"

WebServer server(WEB_SERVER_PORT);

// Forward declarations of handler functions
void handleRoot();
void handleSet();
void handleDistance();
void handleSettings(); // NEW: API endpoint to get current settings

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.on("/distance", handleDistance);
  server.on("/settings", handleSettings); // NEW: Add settings endpoint
  server.begin();
}

void handleWebServer() {
  server.handleClient();
}

// ✅ Handle Distance API (returns real-time distance)
void handleDistance() {
  server.send(200, "text/plain", String(currentDistance));
}

// ✅ NEW: Handle Settings API (returns current settings as JSON)
void handleSettings() {
  String json = "{";
  json += "\"minDistance\":" + String(minDistance) + ",";
  json += "\"maxDistance\":" + String(maxDistance) + ",";
  json += "\"brightness\":" + String(brightness) + ",";
  json += "\"movingLightSpan\":" + String(movingLightSpan) + ",";
  json += "\"numLeds\":" + String(numLeds) + ",";
  json += "\"redValue\":" + String(redValue) + ",";
  json += "\"greenValue\":" + String(greenValue) + ",";
  json += "\"blueValue\":" + String(blueValue);
  json += "}";
  
  server.send(200, "application/json", json);
}

// ✅ Handle Settings Update
void handleSet() {
  if (server.hasArg("numLeds")) {
    int newNumLeds = server.arg("numLeds").toInt();
    if (newNumLeds > 0 && newNumLeds <= 2000) { // Reasonable limit
      numLeds = newNumLeds;
    }
  }
  
  if (server.hasArg("minDist")) minDistance = server.arg("minDist").toInt();
  if (server.hasArg("maxDist")) maxDistance = server.arg("maxDist").toInt();
  
  if (server.hasArg("brightness")) {
    brightness = server.arg("brightness").toInt();
  }
  
  if (server.hasArg("lightSpan")) movingLightSpan = server.arg("lightSpan").toInt();
  
  // Handle RGB color values
  if (server.hasArg("redValue")) redValue = server.arg("redValue").toInt();
  if (server.hasArg("greenValue")) greenValue = server.arg("greenValue").toInt();
  if (server.hasArg("blueValue")) blueValue = server.arg("blueValue").toInt();

  saveSettings();
  updateLEDConfig();
  
  server.send(200, "text/html", "<h3>Settings Updated! <a href='/'>Back</a></h3>");
}

// ✅ Enhanced Web Interface
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
    
    @keyframes spin {
      to { transform: rotate(360deg); }
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

        <div class="section-divider">
          <span class="section-title">Distance Settings</span>
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

        <div class="section-divider">
          <span class="section-title">Appearance</span>
        </div>
        
        <div class="form-group">
          <label>LED Color</label>
          <div class="slider-container">
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
            <div class="color-preview" id="colorPreview"></div>
          </div>
        </div>
        
        <div class="form-group">
          <label>Brightness (0-255)</label>
          <div class="slider-container">
            <input type="range" min="0" max="255" value="255" class="slider" id="brightness">
            <div class="slider-value" id="brightnessValue">255</div>
          </div>
        </div>
        
        <div class="form-group">
          <label>Moving Light Span</label>
          <div class="slider-container">
            <input type="range" min="1" max="100" value="40" class="slider" id="lightSpan">
            <div class="slider-value" id="lightSpanValue">40</div>
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
    <p>Copyright © 2025 TechPosts Media. All rights reserved.</p>
  </div>
  
  <div class="settings-saved" id="settingsSaved">
    Settings saved successfully!
  </div>
  
  <script>
    // Initialize values from server
    window.onload = function() {
      // Load all current settings first
      loadCurrentSettings();
      
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
      
      // Start live updates
      updateDistance();
    };
    
    // NEW: Load current settings from the device
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
          
          // Update color preview
          updateColorPreview();
          
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
      
      const xhr = new XMLHttpRequest();
      xhr.open('GET', '/set?numLeds=' + numLeds + 
                     '&minDist=' + minDist + 
                     '&maxDist=' + maxDist + 
                     '&brightness=' + brightness + 
                     '&lightSpan=' + lightSpan + 
                     '&redValue=' + redValue + 
                     '&greenValue=' + greenValue + 
                     '&blueValue=' + blueValue, true);
      
      xhr.onload = function() {
        if (xhr.status === 200) {
          showSavedNotification();
        }
      };
      xhr.send();
    });
    
    // Live distance updates
    function updateDistance() {
      const xhr = new XMLHttpRequest();
      xhr.open('GET', '/distance', true);
      xhr.onload = function() {
        if (xhr.status === 200) {
          const distance = parseInt(xhr.responseText);
          document.getElementById('liveDistance').innerHTML = distance + '<span class="distance-unit">cm</span>';
          
          // Update LED visualizer
          const minDist = parseInt(document.getElementById('minDist').value);
          const maxDist = parseInt(document.getElementById('maxDist').value);
          const percentage = Math.min(100, Math.max(0, ((distance - minDist) / (maxDist - minDist)) * 100));
          document.getElementById('ledVisualizer').style.width = (100 - percentage) + '%';
        }
      };
      xhr.send();
      
      setTimeout(updateDistance, 500);
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