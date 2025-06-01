#ifndef HTML_TEMPLATES_H
#define HTML_TEMPLATES_H

#include <pgmspace.h>

// Common HTML header template (shared across all pages)
const char commonHeaderTemplate[] PROGMEM = R"rawliteral(
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
    
    .tab-container {
      display: flex;
      margin-bottom: 20px;
      border-bottom: 1px solid #333;
      overflow-x: auto;  /* Allow horizontal scrolling if needed */
      white-space: nowrap;  /* Prevent text wrapping */
    }

    .tab {
      flex: 1 0 auto;  /* Allow flexible growth, but don't shrink */
      padding: 10px 8px;  /* Reduced horizontal padding */
      cursor: pointer;
      background-color: transparent;
      border: none;
      color: var(--text-secondary);
      font-size: 12px;  /* Slightly smaller font */
      position: relative;
      text-overflow: ellipsis;  /* Show ... if text is too long */
      overflow: hidden;
      max-width: 100px;  /* Limit tab width */
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
    
    @keyframes spin {
      to { transform: rotate(360deg); }
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
          <button class="tab %BASIC_TAB_ACTIVE%" onclick="window.location.href='/'">Basic</button>
          <button class="tab %ADVANCED_TAB_ACTIVE%" onclick="window.location.href='/advanced'">Advanced</button>
          <button class="tab %EFFECTS_TAB_ACTIVE%" onclick="window.location.href='/effects'">Effects</button>
          <button class="tab %MESH_TAB_ACTIVE%" onclick="window.location.href='/mesh'">Multi-Sensor</button>
          <button class="tab %NETWORK_TAB_ACTIVE%" onclick="window.location.href='/network'">Network</button>
          <button class="tab %DIAGNOSTICS_TAB_ACTIVE%" onclick="window.location.href='/diagnostics'">Diagnostics</button>
        </div>
)rawliteral";

// Common HTML footer template (shared across all pages)
const char commonFooterTemplate[] PROGMEM = R"rawliteral(
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

  <script>
    // Show saved notification
    function showSavedNotification() {
      // Clear any existing timeout to prevent overlapping notifications
      if (window.savedNotificationTimeout) {
        clearTimeout(window.savedNotificationTimeout);
      }
      
      const notification = document.getElementById('settingsSaved');
      
      // Reset animation by removing and re-adding the class
      notification.classList.remove('show');
      
      // Force browser to process the removal before adding again
      setTimeout(() => {
        notification.classList.add('show');
        
        // Set timeout to hide notification after 3 seconds
        window.savedNotificationTimeout = setTimeout(() => {
          notification.classList.remove('show');
        }, 3000);
      }, 10);
    }
    // Live distance updates
    function updateDistance() {
      fetch('/distance')
        .then(response => response.text())
        .then(distance => {
          distance = parseInt(distance);
          document.getElementById('liveDistance').innerHTML = distance + '<span class="distance-unit">cm</span>';
          
          // Update LED visualizer
          const minDist = parseInt(document.getElementById('minDist')?.value || '30');
          const maxDist = parseInt(document.getElementById('maxDist')?.value || '300');
          const percentage = Math.min(100, Math.max(0, ((distance - minDist) / (maxDist - minDist)) * 100));
          
          // Get the LED color based on current settings
          const ledVisualizer = document.getElementById('ledVisualizer');
          
          // Check if other controls exist (only on certain pages)
          const directionLightElement = document.getElementById('directionLight');
          
          if (directionLightElement && directionLightElement.checked) {
            // If directional light is enabled, visualize it differently
            const position = 100 - percentage;
            ledVisualizer.style.width = '30%';
            ledVisualizer.style.left = `${position - 15}%`;
          } else {
            ledVisualizer.style.width = (100 - percentage) + '%';
            ledVisualizer.style.left = '0';
          }
          
          // Update color if RGB sliders exist
          const redElement = document.getElementById('redValue');
          const greenElement = document.getElementById('greenValue');
          const blueElement = document.getElementById('blueValue');
          
          if (redElement && greenElement && blueElement) {
            const red = parseInt(redElement.value);
            const green = parseInt(greenElement.value);
            const blue = parseInt(blueElement.value);
            ledVisualizer.style.background = `rgb(${red}, ${green}, ${blue})`;
          }
        })
        .catch(error => {
          console.error("Error fetching distance:", error);
        })
        .finally(() => {
          setTimeout(updateDistance, 500);
        });
    }
    
    // Start live updates when page loads
    document.addEventListener('DOMContentLoaded', function() {
      updateDistance();
      
      // Hide loading overlay
      document.getElementById('loadingOverlay').style.display = 'none';
    });
  </script>
</body>
</html>
)rawliteral";

// Basic tab template
const char* basicTabTemplate = R"rawliteral(
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
    <input type="range" min="0" max="500" value="30" class="slider" id="minDist">
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

<!-- Simplified Color Control -->
<div class="form-group">
  <label>LED Color</label>
  <div style="display: flex; gap: 15px; margin-bottom: 15px;">
    <div class="color-preview" id="colorPreview"></div>
  </div>
  
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

<div class="form-group">
  <label>Brightness (0-255)</label>
  <div class="slider-container">
    <input type="range" min="0" max="255" value="255" class="slider" id="brightness">
    <div class="slider-value" id="brightnessValue">255</div>
  </div>
</div>
<!-- Add this just before the Save Settings button in basicTabTemplate -->
<div style="text-align: center; margin-top: 20px;">
  <a href="/resetdistance" style="display: inline-block; padding: 10px; background: #ff4d4d; color: white; text-decoration: none; border-radius: 5px;">
    Reset Min/Max Distance to Defaults
  </a>
</div>
<button class="button" id="saveButton">Save Settings</button>

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
      
      // Add change event for auto-save on slider release 
      slider.addEventListener('change', function() {
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
    
    // Save button
    document.getElementById('saveButton').addEventListener('click', function() {
      const numLeds = document.getElementById('numLeds').value;
      const minDist = document.getElementById('minDist').value;
      const maxDist = document.getElementById('maxDist').value;
      const brightness = document.getElementById('brightness').value;
      const redValue = document.getElementById('redValue').value;
      const greenValue = document.getElementById('greenValue').value;
      const blueValue = document.getElementById('blueValue').value;
      
      // Save basic settings
      fetch('/set?numLeds=' + numLeds + 
                '&minDist=' + minDist + 
                '&maxDist=' + maxDist + 
                '&brightness=' + brightness + 
                '&redValue=' + redValue + 
                '&greenValue=' + greenValue + 
                '&blueValue=' + blueValue)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      })
      .catch(error => {
        console.error("Error saving settings:", error);
      });
    });
  };
  
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
        document.getElementById('loadingOverlay').style.display = 'none';
      });
  }
  
  // Update color preview element with current RGB values
  function updateColorPreview() {
    const red = document.getElementById('redValue') ? parseInt(document.getElementById('redValue').value) : 255;
    const green = document.getElementById('greenValue') ? parseInt(document.getElementById('greenValue').value) : 255;
    const blue = document.getElementById('blueValue') ? parseInt(document.getElementById('blueValue').value) : 255;
    
    const colorPreview = document.getElementById('colorPreview');
    const ledVisualizer = document.getElementById('ledVisualizer');
    
    if (colorPreview) {
      colorPreview.style.backgroundColor = `rgb(${red}, ${green}, ${blue})`;
    }
    
    if (ledVisualizer) {
      ledVisualizer.style.backgroundColor = `rgb(${red}, ${green}, ${blue})`;
    }
  }
  
  // Auto-save changes without requiring the save button
  // Enhanced autoSaveChanges function
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
    
    // Store original values in case we need to revert
    const affectedControl = document.getElementById(settingId);
    const originalValue = affectedControl.value;
    
    // Send to server with enhanced error handling
    fetch('/set?' + params)
      .then(response => {
        if (!response.ok) {
          throw new Error('Server returned status: ' + response.status);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        } else if (data.status === 'error') {
          // Revert the control to its original value
          affectedControl.value = originalValue;
          
          // If display element exists, update it too
          const displayElement = document.getElementById(settingId + 
            (settingId.includes('Value') ? 'Display' : 'Value'));
          if (displayElement) {
            displayElement.textContent = originalValue;
          }
          
          // Alert the user
          alert('Error saving setting: ' + (data.message || 'Unknown error'));
        }
      })
      .catch(error => {
        console.error('Error saving setting:', error);
        
        // Revert the control to its original value
        affectedControl.value = originalValue;
        
        // Update display element if it exists
        const displayElement = document.getElementById(settingId + 
          (settingId.includes('Value') ? 'Display' : 'Value'));
        if (displayElement) {
          displayElement.textContent = originalValue;
        }
        
        // Show error notification
        alert('Failed to save setting. Please try again.');
      });
  }
</script>
)rawliteral";

// Advanced tab template
const char advancedTabTemplate[] PROGMEM = R"rawliteral(
<div class="form-group">
  <label>Moving Light Span</label>
  <div class="slider-container">
    <input type="range" min="1" max="100" value="40" class="slider" id="lightSpan">
    <div class="slider-value" id="lightSpanValue">40</div>
  </div>
  <small class="input-description">
     Controls the width of the moving light effect. Higher values create a wider light trail along the LED strip.
  </small>
</div>

<style>
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
  
  .input-description {
    display: block;
    margin-top: 5px;
    font-size: 12px;
    color: var(--text-secondary);
  }
</style>

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
    Controls how quickly the detected position adapts to new measurements.
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
    Determines how smoothly the movement speed is calculated.
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
    Adjusts how far ahead the system predicts object movement.
    // Continuing from the previous advanced tab template...
</div>

<div class="form-group">
  <label>Position P Gain</label>
  <div class="slider-container">
    <input type="range" min="0" max="100" value="10" class="slider" id="positionPGain"
          onchange="updateMotionParam('positionPGain', this.value * 0.01)">
    <div class="slider-value" id="positionPGainValue">0.10</div>
  </div>
  <small class="input-description">
    Proportional gain for position correction.
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
    Integral gain for position correction.
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
    Adjusts the horizontal position of the light effect along the LED strip.
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
    Controls the length of the fading light trail behind the main light effect.
  </small>
</div>

<button class="button" id="saveButton">Save Settings</button>

<script>
  // Initialize values from server
  window.onload = function() {
    loadCurrentSettings();
    updateMotionSmoothingSliders();
  };
  
  function loadCurrentSettings() {
    // Show loading state
    document.getElementById('loadingOverlay').style.display = 'flex';
    
    fetch('/settings')
      .then(response => {
        if (!response.ok) {
          throw new Error('Server returned status: ' + response.status);
        }
        return response.json();
      })
      .then(settings => {
        // First, check if we're on the advanced tab by looking for its specific controls
        const isAdvancedTab = document.getElementById('lightSpan') !== null;
        
        if (isAdvancedTab) {
          // We're on advanced tab, load those specific settings
          if (document.getElementById('lightSpan')) {
            document.getElementById('lightSpan').value = settings.movingLightSpan || 40;
            document.getElementById('lightSpanValue').textContent = settings.movingLightSpan || 40;
          }
          
          if (document.getElementById('backgroundMode')) {
            document.getElementById('backgroundMode').checked = settings.backgroundMode;
          }
          
          if (document.getElementById('directionLight')) {
            document.getElementById('directionLight').checked = settings.directionLightEnabled;
          }
          
          if (document.getElementById('motionSmoothing')) {
            document.getElementById('motionSmoothing').checked = settings.motionSmoothingEnabled;
          }
          
          if (document.getElementById('positionSmoothingFactor')) {
            // Convert float to slider value (0-100)
            const positionSliderValue = Math.round(settings.positionSmoothingFactor * 100);
            document.getElementById('positionSmoothingFactor').value = positionSliderValue;
            document.getElementById('positionSmoothingValue').textContent = settings.positionSmoothingFactor.toFixed(2);
          }
          
          if (document.getElementById('velocitySmoothingFactor')) {
            const velocitySliderValue = Math.round(settings.velocitySmoothingFactor * 100);
            document.getElementById('velocitySmoothingFactor').value = velocitySliderValue;
            document.getElementById('velocitySmoothingValue').textContent = settings.velocitySmoothingFactor.toFixed(2);
          }
          
          if (document.getElementById('predictionFactor')) {
            const predictionSliderValue = Math.round(settings.predictionFactor * 100);
            document.getElementById('predictionFactor').value = predictionSliderValue;
            document.getElementById('predictionValue').textContent = settings.predictionFactor.toFixed(2);
          }
          
          if (document.getElementById('positionPGain')) {
            const pGainSliderValue = Math.round(settings.positionPGain * 100);
            document.getElementById('positionPGain').value = pGainSliderValue;
            document.getElementById('positionPGainValue').textContent = settings.positionPGain.toFixed(2);
          }
          
          if (document.getElementById('positionIGain')) {
            const iGainSliderValue = Math.round(settings.positionIGain * 1000);
            document.getElementById('positionIGain').value = iGainSliderValue;
            document.getElementById('positionIGainValue').textContent = settings.positionIGain.toFixed(3);
          }
          
          if (document.getElementById('centerShift')) {
            document.getElementById('centerShift').value = settings.centerShift;
            document.getElementById('centerShiftValue').textContent = settings.centerShift;
          }
          
          if (document.getElementById('trailLength')) {
            document.getElementById('trailLength').value = settings.trailLength;
            document.getElementById('trailLengthValue').textContent = settings.trailLength;
          }
        } else {
          // Check if we're on the basic tab by looking for its specific controls
          if (document.getElementById('numLeds')) {
            document.getElementById('numLeds').value = settings.numLeds;
            if (document.getElementById('ledDensity')) {
              document.getElementById('ledDensity').textContent = (settings.numLeds / 60).toFixed(1);
            }
          }
          
          if (document.getElementById('minDist')) {
            document.getElementById('minDist').value = settings.minDistance;
            document.getElementById('minDistValue').textContent = settings.minDistance;
          }
          
          if (document.getElementById('maxDist')) {
            document.getElementById('maxDist').value = settings.maxDistance;
            document.getElementById('maxDistValue').textContent = settings.maxDistance;
          }
          
          if (document.getElementById('brightness')) {
            document.getElementById('brightness').value = settings.brightness;
            document.getElementById('brightnessValue').textContent = settings.brightness;
          }
          
          if (document.getElementById('redValue')) {
            document.getElementById('redValue').value = settings.redValue;
            if (document.getElementById('redValueDisplay')) {
              document.getElementById('redValueDisplay').textContent = settings.redValue;
            }
          }
          
          if (document.getElementById('greenValue')) {
            document.getElementById('greenValue').value = settings.greenValue;
            if (document.getElementById('greenValueDisplay')) {
              document.getElementById('greenValueDisplay').textContent = settings.greenValue;
            }
          }
          
          if (document.getElementById('blueValue')) {
            document.getElementById('blueValue').value = settings.blueValue;
            if (document.getElementById('blueValueDisplay')) {
              document.getElementById('blueValueDisplay').textContent = settings.blueValue;
            }
          }
          
          // Update color preview if we're on that tab
          if (document.getElementById('colorPreview')) {
            updateColorPreview();
          }
        }
        
        // Hide loading overlay
        document.getElementById('loadingOverlay').style.display = 'none';
      })
      .catch(error => {
        console.error('Error loading settings:', error);
        
        // Don't show alert, just quietly hide the loading overlay
        document.getElementById('loadingOverlay').style.display = 'none';
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
    
    document.getElementById('lightSpan').oninput = function() {
      document.getElementById('lightSpanValue').textContent = this.value;
    };
    
    document.getElementById('centerShift').oninput = function() {
      document.getElementById('centerShiftValue').textContent = this.value;
    };
    
    document.getElementById('trailLength').oninput = function() {
      document.getElementById('trailLengthValue').textContent = this.value;
    };
  }
  
  // Save button event
  document.getElementById('saveButton').addEventListener('click', function() {
    // Create a comprehensive save request for all advanced settings
    const lightSpan = document.getElementById('lightSpan').value;
    const centerShift = document.getElementById('centerShift').value;
    const trailLength = document.getElementById('trailLength').value;
    
    // First save the settings via /set endpoint for numeric values
    fetch('/set?lightSpan=' + lightSpan)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          // Now save boolean toggle settings
          const directionLight = document.getElementById('directionLight').checked;
          const backgroundMode = document.getElementById('backgroundMode').checked;
          const motionSmoothing = document.getElementById('motionSmoothing').checked;
          
          // Save all settings one by one with promises
          Promise.all([
            fetch('/setDirectionalLight?enabled=' + directionLight).then(r => r.json()),
            fetch('/setBackgroundMode?enabled=' + backgroundMode).then(r => r.json()),
            fetch('/setMotionSmoothing?enabled=' + motionSmoothing).then(r => r.json()),
            fetch('/setCenterShift?value=' + centerShift).then(r => r.json()),
            fetch('/setTrailLength?value=' + trailLength).then(r => r.json())
          ])
          .then(() => {
            showSavedNotification();
            console.log("All advanced settings saved successfully");
          })
          .catch(error => {
            console.error("Error saving some settings:", error);
            showSavedNotification(); // Still show notification for partial success
          });
        }
      })
      .catch(error => {
        console.error("Error saving settings:", error);
      });
  });
  
  // Functions for settings
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

  // Enhanced updateMotionParam function for advancedTabTemplate
  function updateMotionParam(param, value) {
    // Show a loading indicator or disable inputs
    document.querySelectorAll('input, button').forEach(el => {
      el.disabled = true;
    });
    
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

    // Send update to server with enhanced error handling
    fetch('/setMotionSmoothingParam?param=' + param + '&value=' + value)
      .then(response => {
        if (!response.ok) {
          throw new Error('Server returned status: ' + response.status);
        }
        return response.json();
      })
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
          
          // Update the displayed value to match what the server actually used
          // (in case it was constrained)
          if (displayElement && data.value !== undefined) {
            switch(param) {
              case 'positionIGain':
                displayElement.textContent = parseFloat(data.value).toFixed(3);
                break;
              default:
                displayElement.textContent = parseFloat(data.value).toFixed(2);
            }
          }
        } else {
          alert('Error: ' + (data.error || 'Failed to update parameter'));
        }
      })
      .catch(error => {
        console.error("Network error updating " + param, error);
        alert("Failed to update " + param + ". Please try again.");
      })
      .finally(() => {
        // Re-enable inputs
        document.querySelectorAll('input, button').forEach(el => {
          el.disabled = false;
        });
      });
  }
</script>
)rawliteral";

// Effects tab template
const char effectsTabTemplate[] PROGMEM = R"rawliteral(
<style>
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
</style>

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

<button class="button" id="saveButton">Save Settings</button>

<script>
  // Initialize values from server
  window.onload = function() {
    loadCurrentSettings();
    setupModeDescriptions();
  };
  
  function loadCurrentSettings() {
    fetch('/settings')
      .then(response => response.json())
      .then(settings => {
        // Apply effects settings
        document.getElementById('lightMode').value = settings.lightMode;
        updateModeDescription(settings.lightMode);
        
        document.getElementById('effectSpeed').value = settings.effectSpeed;
        document.getElementById('effectSpeedValue').textContent = settings.effectSpeed;
        
        document.getElementById('effectIntensity').value = settings.effectIntensity;
        document.getElementById('effectIntensityValue').textContent = settings.effectIntensity;
        
        // Add oninput handlers
        document.getElementById('effectSpeed').oninput = function() {
          document.getElementById('effectSpeedValue').textContent = this.value;
        };
        
        document.getElementById('effectIntensity').oninput = function() {
          document.getElementById('effectIntensityValue').textContent = this.value;
        };
        
        // Hide loading overlay
        document.getElementById('loadingOverlay').style.display = 'none';
      })
      .catch(error => {
        console.error('Error loading settings:', error);
        document.getElementById('loadingOverlay').style.display = 'none';
      });
  }
  
  // Mode description handling
  function setupModeDescriptions() {
    document.getElementById('lightMode').addEventListener('change', function() {
      updateModeDescription(this.value);
    });
  }
  
  function updateModeDescription(mode) {
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
    description.textContent = modeDescriptions[mode] || '';
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
  
  // Save button event
  document.getElementById('saveButton').addEventListener('click', function() {
    const speed = document.getElementById('effectSpeed').value;
    const intensity = document.getElementById('effectIntensity').value;
    
    fetch('/setEffectSpeed?value=' + speed)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          return fetch('/setEffectIntensity?value=' + intensity);
        }
        throw new Error('Failed to save effect speed');
      })
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      })
      .catch(error => {
        console.error('Error saving effect settings:', error);
      });
  });
  
  // Auto-save changes
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
</script>
)rawliteral";

// Multi-sensor (mesh) tab template
const char meshTabTemplate[] PROGMEM = R"rawliteral(
<style>
  .info-box {
    background-color: rgba(0,0,0,0.2);
    padding: 15px;
    border-radius: 5px;
    margin-top: 20px;
  }
  
  .slave-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px;
    background-color: rgba(0,0,0,0.2);
    border-radius: 5px;
    margin-bottom: 10px;
  }
  
  .button-small {
    padding: 5px 10px;
    font-size: 12px;
    border-radius: 5px;
    background: var(--primary);
    color: white;
    border: none;
    cursor: pointer;
  }
  
  .button-danger {
    background: linear-gradient(135deg, #ff4d4d 0%, #ff6666 100%);
  }
  
  .modal {
    display: flex;
    position: fixed;
    z-index: 1000;
    left: 0;
    top: 0;
    width: 100%;
    height: 100%;
    background-color: rgba(0,0,0,0.8);
    justify-content: center;
    align-items: center;
  }
  
  .modal-content {
    background-color: var(--bg-card);
    border-radius: var(--border-radius);
    width: 80%;
    max-width: 500px;
  }
  
  .modal-header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 15px;
    border-bottom: 1px solid #333;
  }
  
  .modal-header h3 {
    margin: 0;
    color: var(--primary);
  }
  
  .close {
    font-size: 24px;
    cursor: pointer;
    color: var(--text-secondary);
  }
  
  .close:hover {
    color: var(--text);
  }
  
  .modal-body {
    padding: 15px;
  }
  
  .device-list {
    max-height: 300px;
    overflow-y: auto;
  }
  
  .device-item {
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px;
    background-color: rgba(0,0,0,0.2);
    border-radius: 5px;
    margin-bottom: 10px;
  }
  
  .help-text {
    font-size: 14px;
    color: var(--text-secondary);
  }
  
  .help-list {
    margin-left: 20px;
    padding-left: 0;
  }
  
  .help-list li {
    margin-bottom: 8px;
    color: var(--text-secondary);
    font-size: 14px;
  }
  
  .no-slaves {
    color: var(--text-secondary);
    font-style: italic;
  }
  
  .input-group {
    margin-bottom: 15px;
  }
  
  .input-group label {
    display: block;
    margin-bottom: 5px;
    font-size: 14px;
    color: var(--text-secondary);
  }
  
  .input-group input {
    width: 100%;
    padding: 10px;
    background-color: #333;
    border: none;
    border-radius: 5px;
    color: var(--text);
  }
</style>

<div class="form-group">
  <label>Device Role</label>
  <select class="mode-selector" id="deviceRole" onchange="setDeviceRole(this.value)">
    <option value="1">Master (LED Controller)</option>
    <option value="2">Slave (Secondary Sensor)</option>
  </select>
  <small class="input-description" id="roleDescription">
    Master controls the LED strip and receives data from slave devices.
    Slave sends sensor readings to the master device.
  </small>
</div>

<div class="form-group">
  <label>Sensor Priority Mode</label>
  <select class="mode-selector" id="sensorPriorityMode" onchange="setSensorPriorityMode(this.value)">
    <option value="0">Most Recent Activity (Default)</option>
    <option value="1">Slave Priority (L-Section First)</option>
    <option value="2">Master Priority (Lower Section First)</option>
    <option value="3">Zone-Based Auto-Switching (Recommended for L-shaped)</option>
  </select>
  <small class="input-description" id="priorityModeDescription">
    Controls how the system decides which sensor reading to use when multiple sensors detect motion.
    Zone-based mode is ideal for L-shaped staircases.
  </small>
</div>


<!-- Master config section -->
<div id="master-config" style="display: none;">
  <div class="form-group">
    <h4>Paired Slave Devices</h4>
    <div id="slave-list">
      <!-- Slave devices will be listed here -->
      <p class="no-slaves">No slave devices paired yet.</p>
    </div>
    <button class="button" id="scanSlaves">Scan for Slave Devices</button>
    <small class="input-description">
      Add slave devices to create a multi-sensor system for L-shaped or U-shaped stairs.
      Each slave will send its sensor readings to this master device.
    </small>
  </div>
</div>

<!-- Slave config section -->
<div id="slave-config" style="display: none;">
  <div class="form-group">
    <h4>Master Device</h4>
    <div class="input-group">
      <label for="masterMac">Master MAC Address</label>
      <input type="text" id="masterMac" placeholder="Format: AA:BB:CC:DD:EE:FF">
    </div>
    <button class="button" id="setMaster">Connect to Master</button>
    <small class="input-description">
      Enter the MAC address of the master device. This device will send its
      sensor readings to the master, which controls the LED strip.
    </small>
  </div>
</div>

<div class="info-box">
  <h4>This Device's MAC Address:</h4>
  <p id="deviceMac">00:00:00:00:00:00</p>
  <p><small>Share this with the master device if this is a slave</small></p>
</div>
<div class="form-group">
  <label>LED Distribution Mode</label>
  <select class="mode-selector" id="ledSegmentMode" onchange="setLEDSegmentMode(this.value)">
    <option value="0">Continuous (Single Strip)</option>
    <option value="1">Distributed (Multiple Devices)</option>
  </select>
  <small class="input-description" id="ledModeDescription">
    Continuous mode uses one device for the entire strip. 
    Distributed mode splits long strips across multiple devices.
  </small>
</div>

<div id="distributed-config" style="display: none;">
  <div class="form-group">
    <label>Total System LEDs</label>
    <input type="number" id="totalSystemLeds" min="1" max="5000" value="600" onchange="updateLEDSegmentInfo()">
    <small class="input-description">
      Total LEDs across all devices in the system.
    </small>
  </div>
  
  <div class="form-group">
    <label>This Device's Segment</label>
    <div class="input-group">
      <label for="segmentStart">Start Position:</label>
      <input type="number" id="segmentStart" min="0" value="0" onchange="updateLEDSegmentInfo()">
    </div>
    <div class="input-group">
      <label for="segmentLength">Length:</label>
      <input type="number" id="segmentLength" min="1" value="300" onchange="updateLEDSegmentInfo()">
    </div>
  </div>
</div>
<div class="form-group">
  <h4>Multi-Sensor Configuration Help</h4>
  <p class="help-text">
    For L-shaped or U-shaped staircases, use multiple AmbiSense devices:
  </p>
  <ul class="help-list">
    <li>Connect the LED strip to the <strong>master</strong> device</li>
    <li>Place <strong>slave</strong> devices at turn points in the staircase</li>
    <li>All devices must use the same settings (min/max distance, etc.)</li>
    <li>The master will intelligently combine sensor data for seamless lighting</li>
  </ul>
</div>

<script>
  // Initialize values from server
  window.onload = function() {
    loadDeviceInfo();
  };
  
  function loadDeviceInfo() {
    fetch('/getDeviceInfo')
      .then(response => response.json())
      .then(data => {
        // Set the device role in UI
        document.getElementById('deviceRole').value = data.role;
        
        // Update MAC address display
        document.getElementById('deviceMac').textContent = data.mac;
        
        // Show appropriate config section
        document.getElementById('master-config').style.display = data.role === 1 ? 'block' : 'none';
        document.getElementById('slave-config').style.display = data.role === 2 ? 'block' : 'none';
        document.getElementById('sensorPriorityMode').value = data.sensorPriorityMode;

        // If slave, populate master MAC
        if (data.role === 2 && data.masterMac) {
          document.getElementById('masterMac').value = data.masterMac;
        }
        
        // If master, populate slave list
        if (data.role === 1 && data.slaves && data.slaves.length > 0) {
          const slaveList = document.getElementById('slave-list');
          slaveList.innerHTML = '';
          
          data.slaves.forEach(mac => {
            const div = document.createElement('div');
            div.className = 'slave-item';
            div.innerHTML = `
              <span class="slave-mac">${mac}</span>
              <button class="button-small button-danger" onclick="removeSlave('${mac}')">Remove</button>
            `;
            slaveList.appendChild(div);
          });
        }
        
        // Hide loading overlay
        document.getElementById('loadingOverlay').style.display = 'none';
      })
      .catch(error => {
        console.error('Error loading device info:', error);
        document.getElementById('loadingOverlay').style.display = 'none';
      });
  }
  function setLEDSegmentMode(mode) {
    const distributedConfig = document.getElementById('distributed-config');
    distributedConfig.style.display = mode === '1' ? 'block' : 'none';
    
    fetch('/setLEDSegmentMode?mode=' + mode)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      });
  }
  function updateLEDSegmentInfo() {
    const start = document.getElementById('segmentStart').value;
    const length = document.getElementById('segmentLength').value;
    const total = document.getElementById('totalSystemLeds').value;
    
    fetch('/setLEDSegmentInfo?start=' + start + '&length=' + length + '&total=' + total)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      });
  }
  // Set device role
  function setDeviceRole(role) {
    document.getElementById('master-config').style.display = role === '1' ? 'block' : 'none';
    document.getElementById('slave-config').style.display = role === '2' ? 'block' : 'none';
    
    // Save the role to the server
    fetch('/setDeviceRole?role=' + role)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      })
      .catch(error => {
        console.error('Error setting device role:', error);
      });
  }
  function setSensorPriorityMode(mode) {
    fetch('/setSensorPriorityMode?mode=' + mode)
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
        }
      })
      .catch(error => {
        console.error('Error setting sensor priority mode:', error);
      });
  }
  // Initialize UI event handlers
  document.addEventListener('DOMContentLoaded', function() {
    // Set up scan button if it exists
    if (document.getElementById('scanSlaves')) {
      document.getElementById('scanSlaves').addEventListener('click', function() {
        this.textContent = 'Scanning...';
        this.disabled = true;
        
        fetch('/scanForSlaves')
          .then(response => response.json())
          .then(data => {
            // Create modal to display found devices
            const modal = document.createElement('div');
            modal.className = 'modal';
            modal.innerHTML = `
              <div class="modal-content">
                <div class="modal-header">
                  <h3>Found Devices</h3>
                  <span class="close">&times;</span>
                </div>
                <div class="modal-body">
                  <p>Select a device to add as slave:</p>
                  <div class="device-list">
                    ${data.length ? '' : '<p>No devices found</p>'}
                    ${data.map(device => `
                      <div class="device-item">
                        <span>${device.name || 'Unknown'}</span>
                        <span>${device.mac}</span>
                        <button class="button-small" onclick="addSlave('${device.mac}')">Add</button>
                      </div>
                    `).join('')}
                  </div>
                </div>
              </div>
            `;
            
            document.body.appendChild(modal);
            
            // Close modal on X click
            const closeBtn = modal.querySelector('.close');
            closeBtn.onclick = function() {
              document.body.removeChild(modal);
            };
            
            // Reset button
            this.textContent = 'Scan for Slave Devices';
            this.disabled = false;
          })
          .catch(error => {
            console.error('Error scanning:', error);
            this.textContent = 'Scan for Slave Devices';
            this.disabled = false;
          });
      });
    }
    
    // Set up master connect button
    if (document.getElementById('setMaster')) {
      document.getElementById('setMaster').addEventListener('click', function() {
        const mac = document.getElementById('masterMac').value;
        if (!mac.match(/^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$/)) {
          alert('Invalid MAC address format. Please use format AA:BB:CC:DD:EE:FF');
          return;
        }
        
        fetch('/setMasterMac?mac=' + encodeURIComponent(mac))
          .then(response => response.json())
          .then(data => {
            if (data.status === 'success') {
              showSavedNotification();
            }
          })
          .catch(error => {
            console.error('Error setting master MAC:', error);
          });
      });
    }
  });
  
  // Function to add a slave device
  function addSlave(mac) {
    fetch('/addSlave?mac=' + encodeURIComponent(mac))
      .then(response => response.json())
      .then(data => {
        if (data.status === 'success') {
          showSavedNotification();
          
          // Reload device info to update the slave list
          loadDeviceInfo();
          
          // Close any open modal
          const modal = document.querySelector('.modal');
          if (modal) {
            document.body.removeChild(modal);
          }
        }
      })
      .catch(error => {
        console.error('Error adding slave:', error);
      });
  }
  
  // Function to remove a slave device
  function removeSlave(mac) {
    if (confirm('Are you sure you want to remove this slave device?')) {
      fetch('/removeSlave?mac=' + encodeURIComponent(mac))
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            showSavedNotification();
            
            // Reload device info to update the slave list
            loadDeviceInfo();
          }
        })
        .catch(error => {
          console.error('Error removing slave:', error);
        });
    }
  }
</script>
)rawliteral";

// Diagnostics tab template
const char diagnosticsTabTemplate[] PROGMEM = R"rawliteral(
<style>
  .sensor-list {
    margin-top: 20px;
  }
  
  .sensor-item {
    background-color: rgba(0,0,0,0.2);
    padding: 12px;
    border-radius: 10px;
    margin-bottom: 10px;
    display: flex;
    justify-content: space-between;
    align-items: center;
  }
  
  .sensor-id {
    font-weight: bold;
    color: var(--primary);
  }
  
  .sensor-active {
    color: #4cc9f0;
  }
  
  .sensor-inactive {
    color: #ff6b6b;
    font-style: italic;
  }
  
  .info-box {
    background-color: rgba(0,0,0,0.2);
    padding: 15px;
    border-radius: 10px;
    margin-top: 20px;
  }
  
  .selected-distance {
    font-size: 24px;
    font-weight: bold;
    color: var(--primary);
    text-align: center;
    margin: 15px 0;
  }
</style>

<div class="info-box">
  <h4>Current System Status</h4>
  <p>Priority Mode: <span id="currentPriorityMode">Loading...</span></p>
  <p>Selected Distance: <span id="selectedDistance" class="selected-distance">--</span> cm</p>
  <p>Min/Max Range: <span id="minMaxRange">--</span></p>
</div>

<div class="form-group">
  <h4>Active Sensors</h4>
  <div id="sensorList" class="sensor-list">
    <p>Loading sensor data...</p>
  </div>
</div>

<button class="button" id="refreshButton">Refresh Data</button>

<script>
  // Load sensor data when page loads
  document.addEventListener('DOMContentLoaded', function() {
    loadSensorData();
    
    // Setup refresh button
    document.getElementById('refreshButton').addEventListener('click', function() {
      loadSensorData();
    });
    
    // Auto-refresh every 5 seconds
    setInterval(loadSensorData, 5000);
  });
  
  function loadSensorData() {
    fetch('/sensordata')
      .then(response => response.json())
      .then(data => {
        updateSensorDisplay(data);
      })
      .catch(error => {
        console.error('Error loading sensor data:', error);
      });
      
    // Also load min/max settings
    fetch('/settings')
      .then(response => response.json())
      .then(settings => {
        document.getElementById('minMaxRange').textContent = 
          `${settings.minDistance} - ${settings.maxDistance} cm`;
      })
      .catch(error => {
        console.error('Error loading settings:', error);
      });
  }
  
  function updateSensorDisplay(data) {
    const sensorList = document.getElementById('sensorList');
    const selected = data.selected;
    document.getElementById('selectedDistance').textContent = selected;
    
    // Set priority mode text
    const modeName = getPriorityModeName(data.mode);
    document.getElementById('currentPriorityMode').textContent = modeName;
    
    // Update sensor list
    if (data.sensors && data.sensors.length > 0) {
      let html = '';
      
      data.sensors.forEach(sensor => {
        const isActive = sensor.active;
        const isSelected = (sensor.distance === selected);
        
        html += `
          <div class="sensor-item ${isSelected ? 'selected-sensor' : ''}">
            <div>
              <span class="sensor-id">Sensor ${sensor.id === 0 ? 'Master' : '#' + sensor.id}</span>
              <span class="${isActive ? 'sensor-active' : 'sensor-inactive'}">${isActive ? 'Active' : 'Inactive'}</span>
              ${isSelected ? ' <strong>(Selected)</strong>' : ''}
            </div>
            <div>
              <strong>${sensor.distance} cm</strong>
              <span>${getDirectionText(sensor.direction)}</span>
              <small>(${Math.round(sensor.age/1000)}s ago)</small>
            </div>
          </div>
        `;
      });
      
      sensorList.innerHTML = html;
    } else {
      sensorList.innerHTML = '<p>No active sensors found</p>';
    }
  }
  
  function getPriorityModeName(mode) {
    switch(parseInt(mode)) {
      case 0: return 'Most Recent Activity';
      case 1: return 'Slave First';
      case 2: return 'Master First';
      case 3: return 'Zone-Based';
      default: return 'Unknown';
    }
  }
  
  function getDirectionText(direction) {
    switch(parseInt(direction)) {
      case -1: return '↓ Approaching';
      case 0: return '• Stationary';
      case 1: return '↑ Moving Away';
      default: return '';
    }
  }
</script>
)rawliteral";
// In html_templates.h, add this new template that follows the structure of other tab templates

// Network tab template
const char networkTabTemplate[] PROGMEM = R"rawliteral(
<style>
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
  
  .input-group label {
    display: block;
    margin-bottom: 5px;
    color: var(--text-secondary);
  }
  
  .input-group input[type="text"],
  .input-group input[type="password"] {
    width: 100%;
    padding: 10px;
    border-radius: 5px;
    border: 1px solid #444;
    background-color: #333;
    color: var(--text);
    font-size: 16px;
  }
  
  .button-secondary {
    background: #444;
    margin-top: 10px;
  }
  
  .button-danger {
    background: linear-gradient(135deg, #ff4d4d 0%, #ff6666 100%);
    margin-top: 30px;
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

<div class="status">
  <div class="status-item">
    <span class="status-label">Current Mode:</span>
    <span class="status-value">%MODE_TEXT%</span>
  </div>
  <div class="status-item">
    <span class="status-label">IP Address:</span>
    <span class="status-value" id="current-ip">%IP_ADDRESS%</span>
  </div>
  %MDNS_HTML%
</div>

<form method="post" action="/network">
  <div class="input-group">
    <label for="ssid">WiFi Network Name (SSID)</label>
    <div class="ssid-input-container">
      <input type="text" id="ssid" name="ssid" value="%SSID%" placeholder="Enter network name" required>
      <button type="button" class="scan-button" onclick="scanNetworks()">Scan</button>
    </div>
    <select id="network-list" class="network-dropdown" style="display: none;" onchange="selectNetwork(this.value)">
      <option value="">Select a network...</option>
    </select>
  </div>

  <div class="input-group">
    <label for="password">WiFi Password</label>
    <input type="password" id="password" name="password" placeholder="Enter network password">
  </div>

  <div class="input-group">
    <label for="deviceName">Device Name (used for mDNS address)</label>
    <input type="text" id="deviceName" name="deviceName" value="%DEVICE_NAME%" placeholder="Enter device name" required>
    <small style="color: var(--text-secondary); font-size: 12px; margin-top: 5px; display: block;">
      Your device will be accessible at: http://ambisense-[name].local
    </small>
  </div>
  
  <button type="submit" class="button">Save Settings</button>
  <button type="button" class="button button-secondary" onclick="window.location.href='/'">Cancel</button>
</form>

<button type="button" class="button button-danger" onclick="if(confirm('Are you sure you want to reset all network settings?')) { window.location.href='/resetwifi'; }">Reset Network Settings</button>

<script>
// Network scanning functions
function scanNetworks() {
  // Show dropdown and scanning message
  const networkList = document.getElementById('network-list');
  networkList.innerHTML = '<option value="">Scanning...</option>';
  networkList.style.display = 'block';
  
  // Change scan button appearance while scanning
  const scanButton = document.querySelector('.scan-button');
  scanButton.disabled = true;
  scanButton.innerHTML = 'Scanning...';
  scanButton.style.opacity = '0.7';
  
  const scanTimeout = setTimeout(() => {
    scanButton.disabled = false;
    scanButton.innerHTML = 'Scan';
    scanButton.style.opacity = '1';
    networkList.innerHTML = '<option value="">Scan timed out. Try again</option>';
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
      
      if (!networks || networks.length === 0) {
        networkList.innerHTML = '<option value="">No networks found</option>';
        return;
      }
      
      // Create document fragment for better performance
      const fragment = document.createDocumentFragment();
      
      // Create default option
      const defaultOption = document.createElement('option');
      defaultOption.value = '';
      defaultOption.textContent = 'Select a network...';
      fragment.appendChild(defaultOption);
      
      // Add network options
      networks.forEach(network => {
        const option = document.createElement('option');
        option.value = network.ssid;
        
        // Determine security and signal strength
        const secureIcon = network.secure ? '🔒 ' : '';
        const signalStrength = getSignalStrengthLabel(network.rssi);
        
        option.textContent = `${secureIcon}${network.ssid} (${signalStrength})`;
        fragment.appendChild(option);
      });
      
      // Clear and populate network list
      networkList.innerHTML = '';
      networkList.appendChild(fragment);
    })
    .catch(error => {
      clearTimeout(scanTimeout);
      console.error('Error scanning networks:', error);
      networkList.innerHTML = '<option value="">Scan failed, try again</option>';
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
)rawliteral";

// Simple page template for low memory devices
const char simplePageTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>AmbiSense | Simple Mode</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body {
      font-family: Arial, sans-serif;
      background: #121212;
      color: white;
      margin: 0;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background: #1e1e1e;
      padding: 20px;
      .container {
      max-width: 600px;
      margin: 0 auto;
      background: #1e1e1e;
      padding: 20px;
      border-radius: 10px;
    }
    h1 {
      text-align: center;
      color: #4361ee;
    }
    input, select {
      width: 100%;
      padding: 8px;
      margin: 5px 0 15px;
      background: #333;
      border: none;
      color: white;
      border-radius: 5px;
    }
    label {
      display: block;
      margin-top: 10px;
      color: #b0b0b0;
    }
    button {
      background: #4361ee;
      color: white;
      border: none;
      padding: 10px 15px;
      border-radius: 5px;
      cursor: pointer;
      width: 100%;
      margin-top: 20px;
    }
    .footer {
      text-align: center;
      margin-top: 30px;
      font-size: 12px;
      color: #b0b0b0;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>AmbiSense Simple Mode</h1>
    <p>This is a lightweight interface for devices with limited memory. For full functionality, use the main interface.</p>
    
    <form id="settingsForm">
      <label for="numLeds">Number of LEDs:</label>
      <input type="number" id="numLeds" name="numLeds" min="1" max="2000" value="300">
      
      <label for="minDist">Minimum Distance (cm):</label>
      <input type="number" id="minDist" name="minDist" min="0" max="200" value="30">
      
      <label for="maxDist">Maximum Distance (cm):</label>
      <input type="number" id="maxDist" name="maxDist" min="50" max="500" value="300">
      
      <label for="brightness">Brightness (0-255):</label>
      <input type="number" id="brightness" name="brightness" min="0" max="255" value="255">
      
      <label for="lightMode">Light Mode:</label>
      <select id="lightMode" name="lightMode">
        <option value="0">Standard</option>
        <option value="4">Solid Color</option>
        <option value="1">Rainbow</option>
        <option value="2">Color Wave</option>
        <option value="3">Breathing</option>
      </select>
      
      <button type="button" onclick="saveSettings()">Save Settings</button>
    </form>
    
    <div class="current-distance">
      <label>Current Distance: <span id="distance">--</span> cm</label>
    </div>
  </div>
  
  <div class="footer">
    <p>AmbiSense by Ravi Singh (TechPosts Media)</p>
    <p>Copyright &copy; 2025 TechPosts Media. All rights reserved.</p>
  </div>
  
  <script>
    // Load current settings
    window.onload = function() {
      fetch('/settings')
        .then(response => response.json())
        .then(settings => {
          document.getElementById('numLeds').value = settings.numLeds;
          document.getElementById('minDist').value = settings.minDistance;
          document.getElementById('maxDist').value = settings.maxDistance;
          document.getElementById('brightness').value = settings.brightness;
          document.getElementById('lightMode').value = settings.lightMode;
          
          // Start distance updates
          updateDistance();
        })
        .catch(error => {
          console.error('Error loading settings:', error);
        });
    };
    
    // Save settings
    function saveSettings() {
      const numLeds = document.getElementById('numLeds').value;
      const minDist = document.getElementById('minDist').value;
      const maxDist = document.getElementById('maxDist').value;
      const brightness = document.getElementById('brightness').value;
      const lightMode = document.getElementById('lightMode').value;
      
      fetch('/set?numLeds=' + numLeds + 
                '&minDist=' + minDist + 
                '&maxDist=' + maxDist + 
                '&brightness=' + brightness)
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            return fetch('/setLightMode?mode=' + lightMode);
          }
          throw new Error('Failed to save basic settings');
        })
        .then(response => response.json())
        .then(data => {
          if (data.status === 'success') {
            alert('Settings saved successfully!');
          }
        })
        .catch(error => {
          console.error('Error saving settings:', error);
          alert('Error saving settings. Please try again.');
        });
    }
    
    // Update distance display
    function updateDistance() {
      fetch('/distance')
        .then(response => response.text())
        .then(distance => {
          document.getElementById('distance').textContent = distance;
        })
        .catch(error => {
          console.error('Error fetching distance:', error);
        })
        .finally(() => {
          setTimeout(updateDistance, 1000);
        });
    }
  </script>
</body>
</html>
)rawliteral";

// WiFi Reset HTML Template
const char wifiResetTemplate[] PROGMEM = R"rawliteral(
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

// Network Restart HTML Template
const char networkRestartTemplate[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>Network Settings - AmbiSense</title>
  <meta name='viewport' content='width=device-width, initial-scale=1'>
  <style>
    body{font-family:Arial;margin:0;padding:20px;text-align:center;}
    .container{max-width:500px;margin:0 auto;}
    h1{color:#4361ee;}
    .info{background-color:#f0f0f0;padding:15px;border-radius:5px;margin:20px 0;}
    .countdown{font-size:24px;font-weight:bold;margin:20px 0;color:#4361ee;}
  </style>
  <script>
    var countdown=10;
    function updateCountdown(){
      document.getElementById('timer').innerText=countdown;
      countdown--;
      if(countdown<0){
        window.location.href='http://%HOSTNAME%.local';
      } else {
        setTimeout(updateCountdown,1000);
      }
    }
    window.onload=function(){updateCountdown();};
  </script>
</head>
<body>
  <div class='container'>
    <h1>Network Settings Updated</h1>
    <div class='info'>
      <p>The device will now restart and attempt to connect to your WiFi network.</p>
      <p>If successful, you'll be able to access it at:<br><strong>http://%HOSTNAME%.local</strong></p>
      <p>Device restarting in <span id='timer'>10</span> seconds...</p>
    </div>
    <div id='countdown-info' class='countdown'></div>
  </div>
</body>
</html>
)rawliteral";


#endif // HTML_TEMPLATES_H