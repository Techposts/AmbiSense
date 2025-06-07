#include <WiFi.h>
#include <EEPROM.h>
#include "config.h"
#include "wifi_manager.h"

// Connection timeout constants
#define WIFI_CONNECTION_TIMEOUT 20000  // Increased to 20 seconds
#define WIFI_RECONNECT_DELAY 1000      // Increased delay
#define MAX_CONNECTION_ATTEMPTS 3      // Increased attempts
#define WIFI_CHECK_INTERVAL 30000      // Check connection every 30 seconds
#define WIFI_RETRY_DELAY 2000          // Delay between retries

// WiFi status constants for compatibility across ESP32 core versions
#ifndef WL_WRONG_PASSWORD
#define WL_WRONG_PASSWORD 7
#endif

// Global instance
WifiManager wifiManager;

void WifiManager::begin() {
  Serial.println("[WiFi] Initializing WiFi manager");
  
  pinMode(WIFI_STATUS_LED_PIN, OUTPUT);
  digitalWrite(WIFI_STATUS_LED_PIN, LOW);

  // Force WiFi subsystem reset for clean state
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(500);  // Extended delay for proper reset

  // Check if WiFi section is properly marked
  uint16_t wifiMarker;
  EEPROM.get(EEPROM_WIFI_MARKER_ADDR, wifiMarker);
  if (wifiMarker != 0xA55A) {
    Serial.println("[WiFi] EEPROM WiFi section not initialized");
    
    // Initialize marker and clear WiFi section
    EEPROM.put(EEPROM_WIFI_MARKER_ADDR, (uint16_t)0xA55A);
    for (int i = EEPROM_WIFI_SSID_ADDR; 
         i < EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_DEVICE_NAME_LENGTH; 
         i++) {
      EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("[WiFi] EEPROM WiFi section initialized");
  }

  // Load credentials from EEPROM
  char ssid[MAX_SSID_LENGTH] = {0};
  char password[MAX_PASSWORD_LENGTH] = {0};
  char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};

  // Try to load saved credentials
  if (loadWifiCredentials(ssid, password, deviceName) && strlen(ssid) > 0) {
    Serial.printf("[WiFi] Saved credentials found: SSID='%s'\n", ssid);
    
    // Try to connect with improved sequence
    if (connectToWifiImproved(ssid, password)) {
      _currentMode = WIFI_MANAGER_MODE_STA;
      _connectionAttempts = 0;
      
      // Setup mDNS with device name
      if (strlen(deviceName) > 0) {
        String hostname = getSanitizedHostname(deviceName);
        setupMDNS(hostname.c_str());
      } else {
        // Use default hostname with MAC address if no device name was set
        char defaultName[32];
        uint32_t chipId = ESP.getEfuseMac() & 0xFFFFFFFF;
        sprintf(defaultName, "ambisense-%08X", chipId);
        setupMDNS(defaultName);
      }
      
      Serial.println("[WiFi] Station mode established successfully");
    } else {
      Serial.println("[WiFi] Failed to connect. Starting AP mode");
      startAPModeImproved();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
    }
  } else {
    Serial.println("[WiFi] No saved credentials. Starting AP mode");
    startAPModeImproved();
    _currentMode = WIFI_MANAGER_MODE_AP;
  }
  
  // Print current connection status
  if (_currentMode == WIFI_MANAGER_MODE_STA) {
    Serial.printf("[WiFi] Connected: IP=%s, RSSI=%d, Channel=%d\n", 
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI(),
                  WiFi.channel());
  } else {
    Serial.printf("[WiFi] Running in AP mode: IP=%s, Channel=%d\n", 
                  WiFi.softAPIP().toString().c_str(),
                  WiFi.channel());
  }
  
  // Store the WiFi channel for ESP-NOW compatibility
  _currentChannel = WiFi.channel();
}

bool WifiManager::startAPModeImproved() {
  Serial.println("[WiFi] Starting Access Point mode");
  
  // Use a consistent AP name based on chip ID
  char apName[32];
  uint32_t chipId = ESP.getEfuseMac() & 0xFFF; // Use last 12 bits (3 hex digits)
  sprintf(apName, "AmbiSense-%03X-Setup", chipId);
  
  // Complete WiFi reset sequence
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(200);
  
  // Set to AP mode with specific configuration
  WiFi.mode(WIFI_AP);
  delay(100);
  
  // Use fixed channel for ESP-NOW compatibility
  const int AP_CHANNEL = ESPNOW_CHANNEL;  // Use same channel as ESP-NOW
  
  // Start AP with specific configuration
  bool success = WiFi.softAP(apName, WIFI_AP_PASSWORD, AP_CHANNEL, 0, 4);
  
  if (success) {
    delay(100);  // Allow AP to stabilize
    
    Serial.printf("[WiFi] AP started successfully\n");
    Serial.printf("[WiFi] AP Name: %s\n", apName);
    Serial.printf("[WiFi] AP Password: %s\n", WIFI_AP_PASSWORD);
    Serial.printf("[WiFi] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
    Serial.printf("[WiFi] AP Channel: %d\n", WiFi.channel());
    
    _currentChannel = WiFi.channel();
  } else {
    Serial.println("[WiFi] Failed to start AP mode!");
    
    // Retry with different approach
    delay(500);
    WiFi.mode(WIFI_OFF);
    delay(200);
    WiFi.mode(WIFI_AP);
    delay(200);
    
    success = WiFi.softAP(apName, WIFI_AP_PASSWORD);
    if (success) {
      Serial.println("[WiFi] AP started on retry");
      _currentChannel = WiFi.channel();
    }
  }
  
  return success;
}

bool WifiManager::startAPMode() {
  return startAPModeImproved();
}

bool WifiManager::connectToWifiImproved(const char* ssid, const char* password) {
  // Extra validation
  if (!ssid || strlen(ssid) == 0) {
    Serial.println("[WiFi] Cannot connect: SSID is empty");
    return false;
  }
  
  Serial.printf("[WiFi] Connecting to: '%s'\n", ssid);
  
  // Validate SSID characters
  for (int i = 0; i < strlen(ssid); i++) {
    if (ssid[i] < 32 || ssid[i] > 126) {
      Serial.println("[WiFi] Cannot connect: SSID contains invalid characters");
      return false;
    }
  }
  
  // Complete disconnect and reset
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(500);
  
  // Set to station mode
  WiFi.mode(WIFI_STA);
  delay(200);
  
  // Configure WiFi with improved settings
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);  // Disable power saving for stability
  
  // Multiple connection attempts with different strategies
  for (int attempt = 1; attempt <= MAX_CONNECTION_ATTEMPTS; attempt++) {
    Serial.printf("[WiFi] Connection attempt %d/%d\n", attempt, MAX_CONNECTION_ATTEMPTS);
    
    // Start connection
    if (password && strlen(password) > 0) {
      WiFi.begin(ssid, password);
    } else {
      WiFi.begin(ssid);  // Open network
    }
    
    // Wait for connection with detailed feedback
    unsigned long startTime = millis();
    int lastStatus = -1;
    
    while (WiFi.status() != WL_CONNECTED && 
           (millis() - startTime < WIFI_CONNECTION_TIMEOUT)) {
      
      int currentStatus = WiFi.status();
      if (currentStatus != lastStatus) {
        Serial.printf("[WiFi] Status: %s\n", getWiFiStatusString(currentStatus));
        lastStatus = currentStatus;
      }
      
      delay(500);
      
      // Early exit on certain failure conditions (with compatibility check)
      if (currentStatus == WL_CONNECT_FAILED || 
          currentStatus == WL_NO_SSID_AVAIL) {
        Serial.printf("[WiFi] Connection failed with status: %s\n", 
                     getWiFiStatusString(currentStatus));
        break;
      }
      
      // Check for wrong password (if constant is available)
      #ifdef WL_WRONG_PASSWORD
      if (currentStatus == WL_WRONG_PASSWORD) {
        Serial.printf("[WiFi] Connection failed with status: %s\n", 
                     getWiFiStatusString(currentStatus));
        break;
      }
      #endif
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("[WiFi] Connected successfully!\n");
      Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WiFi] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
      Serial.printf("[WiFi] Subnet: %s\n", WiFi.subnetMask().toString().c_str());
      Serial.printf("[WiFi] DNS: %s\n", WiFi.dnsIP().toString().c_str());
      Serial.printf("[WiFi] RSSI: %d dBm\n", WiFi.RSSI());
      Serial.printf("[WiFi] Channel: %d\n", WiFi.channel());
      
      _currentChannel = WiFi.channel();
      return true;
    }
    
    // If not the last attempt, wait before retry
    if (attempt < MAX_CONNECTION_ATTEMPTS) {
      Serial.printf("[WiFi] Attempt %d failed, retrying in %d ms\n", 
                   attempt, WIFI_RETRY_DELAY);
      
      WiFi.disconnect();
      delay(WIFI_RETRY_DELAY);
    }
  }
  
  Serial.printf("[WiFi] All connection attempts failed. Final status: %s\n", 
               getWiFiStatusString(WiFi.status()));
  return false;
}

bool WifiManager::connectToWifi(const char* ssid, const char* password) {
  return connectToWifiImproved(ssid, password);
}

const char* WifiManager::getWiFiStatusString(int status) {
  switch (status) {
    case WL_IDLE_STATUS: return "Idle";
    case WL_NO_SSID_AVAIL: return "No SSID Available";
    case WL_SCAN_COMPLETED: return "Scan Completed";
    case WL_CONNECTED: return "Connected";
    case WL_CONNECT_FAILED: return "Connection Failed";
    case WL_CONNECTION_LOST: return "Connection Lost";
    case WL_DISCONNECTED: return "Disconnected";
    
    // Handle WL_WRONG_PASSWORD conditionally for compatibility
    #ifdef WL_WRONG_PASSWORD
    case WL_WRONG_PASSWORD: return "Wrong Password";
    #endif
    
    // Handle status 7 (wrong password) if WL_WRONG_PASSWORD is not defined
    #ifndef WL_WRONG_PASSWORD
    case 7: return "Authentication Failed";
    #endif
    
    default: 
      // Return status code for unknown values
      static char statusBuffer[32];
      sprintf(statusBuffer, "Unknown Status (%d)", status);
      return statusBuffer;
  }
}

bool WifiManager::loadWifiCredentials(char* ssid, char* password, char* deviceName) {
  Serial.println("[WiFi] Loading WiFi credentials");
  
  // First check for a specific initialization marker
  uint16_t marker;
  EEPROM.get(EEPROM_WIFI_MARKER_ADDR, marker);
  
  if (marker != 0xA55A) {
    Serial.println("[WiFi] No valid marker found in EEPROM WiFi section");
    // Zero out the credentials
    memset(ssid, 0, MAX_SSID_LENGTH);
    memset(password, 0, MAX_PASSWORD_LENGTH);
    memset(deviceName, 0, MAX_DEVICE_NAME_LENGTH);
    
    // Write marker for next time
    EEPROM.put(EEPROM_WIFI_MARKER_ADDR, (uint16_t)0xA55A);
    EEPROM.commit();
    
    return false;
  }
  
  // Read SSID
  for (int i = 0; i < MAX_SSID_LENGTH; i++) {
    ssid[i] = EEPROM.read(EEPROM_WIFI_SSID_ADDR + i);
  }
  ssid[MAX_SSID_LENGTH - 1] = '\0'; // Ensure null termination
  
  // If SSID is empty, there are no saved credentials
  if (strlen(ssid) == 0) {
    Serial.println("[WiFi] No SSID found in EEPROM");
    memset(password, 0, MAX_PASSWORD_LENGTH);
    memset(deviceName, 0, MAX_DEVICE_NAME_LENGTH);
    return false;
  }
  
  // Check for non-printable characters in SSID
  bool validSsid = true;
  for (int i = 0; i < strlen(ssid); i++) {
    if (ssid[i] < 32 || ssid[i] > 126) {
      validSsid = false;
      break;
    }
  }
  
  if (!validSsid) {
    Serial.println("[WiFi] Corrupt SSID with non-printable characters detected");
    memset(ssid, 0, MAX_SSID_LENGTH); 
    memset(password, 0, MAX_PASSWORD_LENGTH);
    memset(deviceName, 0, MAX_DEVICE_NAME_LENGTH);
    return false;
  }
  
  // Read password
  for (int i = 0; i < MAX_PASSWORD_LENGTH; i++) {
    password[i] = EEPROM.read(EEPROM_WIFI_PASS_ADDR + i);
  }
  password[MAX_PASSWORD_LENGTH - 1] = '\0'; // Ensure null termination
  
  // Read device name
  for (int i = 0; i < MAX_DEVICE_NAME_LENGTH; i++) {
    deviceName[i] = EEPROM.read(EEPROM_DEVICE_NAME_ADDR + i);
  }
  deviceName[MAX_DEVICE_NAME_LENGTH - 1] = '\0'; // Ensure null termination
  
  Serial.printf("[WiFi] Loaded SSID: '%s' (length: %d)\n", ssid, strlen(ssid));
  
  return (strlen(ssid) > 0); // Consider valid only if SSID exists
}

void WifiManager::saveWifiCredentials(const char* ssid, const char* password, const char* deviceName) {
  Serial.printf("[WiFi] Saving credentials: SSID='%s'\n", ssid);
  
  // Save WiFi marker
  EEPROM.put(EEPROM_WIFI_MARKER_ADDR, (uint16_t)0xA55A);
  
  // Clear existing data first
  for (int i = EEPROM_WIFI_SSID_ADDR; 
       i < EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_DEVICE_NAME_LENGTH; 
       i++) {
    EEPROM.write(i, 0);
  }
  
  // Write SSID (with length checking)
  for (int i = 0; i < strlen(ssid) && i < MAX_SSID_LENGTH - 1; i++) {
    EEPROM.write(EEPROM_WIFI_SSID_ADDR + i, ssid[i]);
  }
  
  // Write password (with length checking)
  for (int i = 0; i < strlen(password) && i < MAX_PASSWORD_LENGTH - 1; i++) {
    EEPROM.write(EEPROM_WIFI_PASS_ADDR + i, password[i]);
  }
  
  // Write device name (with length checking)
  for (int i = 0; i < strlen(deviceName) && i < MAX_DEVICE_NAME_LENGTH - 1; i++) {
    EEPROM.write(EEPROM_DEVICE_NAME_ADDR + i, deviceName[i]);
  }
  
  // Commit changes to flash
  if (EEPROM.commit()) {
    Serial.println("[WiFi] Credentials saved successfully");
    
    // Verify the SSID
    char verifySSID[MAX_SSID_LENGTH] = {0};
    for (int i = 0; i < MAX_SSID_LENGTH - 1; i++) {
      verifySSID[i] = EEPROM.read(EEPROM_WIFI_SSID_ADDR + i);
    }
    
    if (String(verifySSID) != String(ssid)) {
      Serial.println("[WiFi] WARNING: SSID verification failed");
    } else {
      Serial.println("[WiFi] SSID verification successful");
    }
  } else {
    Serial.println("[WiFi] ERROR: Failed to commit credentials to EEPROM");
  }
}

bool WifiManager::setupMDNS(const char* hostname) {
  if (strlen(hostname) == 0) {
    Serial.println("[mDNS] Hostname is empty, cannot setup mDNS");
    return false;
  }
  
  // Ensure we only try to start mDNS once
  if (_mDNSStarted) {
    MDNS.end();
    _mDNSStarted = false;
    delay(100);
  }
  
  Serial.printf("[mDNS] Setting up hostname: %s\n", hostname);
  
  if (!MDNS.begin(hostname)) {
    Serial.println("[mDNS] Error setting up mDNS responder!");
    return false;
  }
  
  // Add service to mDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addServiceTxt("http", "tcp", "device", "AmbiSense");
  MDNS.addServiceTxt("http", "tcp", "version", "5.1");
  
  Serial.printf("[mDNS] mDNS responder started at http://%s.local\n", hostname);
  
  _mDNSStarted = true;
  return true;
}

void WifiManager::checkWifiConnection() {
  if (_currentMode != WIFI_MANAGER_MODE_STA) {
    return; // Don't check if not in station mode
  }
  
  wl_status_t status = WiFi.status();
  
  if (status != WL_CONNECTED) {
    Serial.printf("[WiFi] Connection lost. Status: %s\n", getWiFiStatusString(status));
    
    _connectionAttempts++;
    
    if (_connectionAttempts > MAX_CONNECTION_ATTEMPTS) {
      Serial.println("[WiFi] Max reconnection attempts reached. Switching to AP mode");
      
      startAPModeImproved();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
      _connectionAttempts = 0;
      return;
    }
    
    // Attempt to reconnect
    char ssid[MAX_SSID_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
    
    if (loadWifiCredentials(ssid, password, deviceName) && strlen(ssid) > 0) {
      Serial.printf("[WiFi] Attempting reconnection to '%s' (attempt %d/%d)\n", 
                   ssid, _connectionAttempts, MAX_CONNECTION_ATTEMPTS);
      
      // Quick reconnect attempt
      WiFi.disconnect();
      delay(1000);
      
      if (password && strlen(password) > 0) {
        WiFi.begin(ssid, password);
      } else {
        WiFi.begin(ssid);
      }
      
      // Short wait for quick reconnection
      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED && 
             (millis() - startTime < WIFI_CONNECTION_TIMEOUT / 3)) {
        delay(500);
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("[WiFi] Reconnected successfully");
        _connectionAttempts = 0;
        _currentChannel = WiFi.channel();
        
        // Update ESP-NOW channel if needed
        if (_espnowChannelCallback) {
          _espnowChannelCallback(_currentChannel);
        }
      } else {
        Serial.printf("[WiFi] Reconnect attempt %d failed\n", _connectionAttempts);
      }
    }
  } else {
    // Connection is stable, reset attempt counter
    if (_connectionAttempts > 0) {
      Serial.println("[WiFi] Connection restored");
      _connectionAttempts = 0;
    }
  }
}

std::vector<WiFiNetwork> WifiManager::scanNetworks() {
  std::vector<WiFiNetwork> networks;
  
  Serial.println("[WiFi] Scanning for networks...");
  
  // Ensure WiFi is in correct mode for scanning
  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.mode(WIFI_STA);
    delay(100);
  }
  
  int numNetworks = WiFi.scanNetworks(false, true, false, 300);  // 300ms per channel
  
  if (numNetworks > 0) {
    Serial.printf("[WiFi] Found %d networks\n", numNetworks);
    
    // Process found networks
    for (int i = 0; i < numNetworks && i < 10; i++) {  // Limit to 10 networks
      WiFiNetwork network;
      network.ssid = WiFi.SSID(i);
      network.rssi = WiFi.RSSI(i);
      network.encType = WiFi.encryptionType(i);
      
      // Skip networks with empty SSID (hidden networks without names)
      if (network.ssid.length() > 0) {
        networks.push_back(network);
      }
    }
    
    // Sort by signal strength (strongest first)
    std::sort(networks.begin(), networks.end(), 
              [](const WiFiNetwork& a, const WiFiNetwork& b) {
                return a.rssi > b.rssi;
              });
  } else {
    Serial.println("[WiFi] No networks found");
  }
  
  // Clean up scan results to free memory
  WiFi.scanDelete();
  
  return networks;
}

void WifiManager::process() {
  unsigned long now = millis();

  // Update LED status
  updateLedStatus();

  // Periodic WiFi health check
  if (_currentMode == WIFI_MANAGER_MODE_STA && (now - _lastWifiCheck > WIFI_CHECK_INTERVAL)) {
    _lastWifiCheck = now;
    checkWifiConnection();
  }
  
  // Handle mode transitions
  if (_pendingModeChange != WIFI_MANAGER_MODE_NONE) {
    handleModeChange();
  }
}

void WifiManager::handleModeChange() {
  if (_pendingModeChange == WIFI_MANAGER_MODE_AP && _currentMode != WIFI_MANAGER_MODE_AP) {
    Serial.println("[WiFi] Switching to AP mode");
    startAPModeImproved();
    _currentMode = WIFI_MANAGER_MODE_AP;
  }
  
  _pendingModeChange = WIFI_MANAGER_MODE_NONE;
}

void WifiManager::updateLedStatus() {
  unsigned long now = millis();
  
  if (_currentMode == WIFI_MANAGER_MODE_STA && WiFi.status() == WL_CONNECTED) {
    // Connected mode: Slow blink (2 second on, 1 second off)
    int cycle = (now / 1000) % 3;
    digitalWrite(WIFI_STATUS_LED_PIN, cycle < 2 ? HIGH : LOW);
  } else {
    // AP/Fallback Mode: Fast blink (500ms intervals)
    digitalWrite(WIFI_STATUS_LED_PIN, ((now / 500) % 2 == 0) ? HIGH : LOW);
  }
}

void WifiManager::resetWifiSettings() {
  Serial.println("[WiFi] Resetting WiFi settings");
  
  // Stop mDNS if running
  if (_mDNSStarted) {
    MDNS.end();
    _mDNSStarted = false;
  }
  
  // Clear the WiFi credential area
  for (int i = EEPROM_WIFI_SSID_ADDR; 
       i < EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_DEVICE_NAME_LENGTH; 
       i++) {
    EEPROM.write(i, 0);
  }
  
  // Keep the marker to indicate section is initialized
  EEPROM.put(EEPROM_WIFI_MARKER_ADDR, (uint16_t)0xA55A);
  
  // Commit changes to flash
  EEPROM.commit();
  
  Serial.println("[WiFi] WiFi settings reset. Restarting...");
  
  // Small delay before restart
  delay(1000);
  
  // Restart to apply changes
  ESP.restart();
}

String WifiManager::getIPAddress() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  } else {
    return WiFi.softAPIP().toString();
  }
}

WifiManagerMode WifiManager::getMode() {
  return _currentMode;
}

int WifiManager::getCurrentChannel() {
  return _currentChannel;
}

void WifiManager::setESPNowChannelCallback(std::function<void(int)> callback) {
  _espnowChannelCallback = callback;
}

String WifiManager::getSanitizedHostname(const char* deviceName) {
  String hostname;
  
  if (deviceName && strlen(deviceName) > 0) {
    String name = String(deviceName);
    
    // Convert to lowercase
    name.toLowerCase();
    
    // Replace spaces with hyphens
    name.replace(" ", "-");
    
    // Replace other invalid characters
    name.replace(".", "-");
    name.replace("_", "-");
    
    // Remove any other non-alphanumeric characters except hyphen
    String validName = "";
    for (unsigned int i = 0; i < name.length(); i++) {
      char c = name.charAt(i);
      if (isalnum(c) || c == '-') {
        validName += c;
      }
    }
    
    // Make sure we don't double-prefix with "ambisense-"
    if (validName.startsWith("ambisense-")) {
      hostname = validName; // Keep as is if already prefixed
    } else {
      hostname = "ambisense-" + validName; // Add prefix if not present
    }
  } else {
    // Use MAC address as fallback
    uint32_t chipId = ESP.getEfuseMac() & 0xFFFFFFFF;
    char idStr[9];
    sprintf(idStr, "%08X", chipId);
    hostname = "ambisense-" + String(idStr);
  }
  
  return hostname;
}

bool WifiManager::isConnected() {
  return (WiFi.status() == WL_CONNECTED && _currentMode == WIFI_MANAGER_MODE_STA);
}