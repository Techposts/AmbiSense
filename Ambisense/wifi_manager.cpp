#include <WiFi.h>
#include <ESPmDNS.h>
#include <EEPROM.h>
#include "config.h"
#include "wifi_manager.h"

// Connection timeout constants
#define WIFI_CONNECTION_TIMEOUT 30000  // 30 seconds
#define WIFI_CHECK_INTERVAL 10000      // 10 seconds
#define MAX_CONNECTION_ATTEMPTS 3

// Global instance
WifiManager wifiManager;

void WifiManager::begin() {
  Serial.println("[WiFi] Initializing WiFi manager...");
  pinMode(WIFI_STATUS_LED_PIN, OUTPUT);
  digitalWrite(WIFI_STATUS_LED_PIN, LOW);

  // Always start with WiFi off
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(100);

  // Ensure EEPROM has been initialized with sufficient size
  // Note: This is critical - we should not call EEPROM.begin() again if it was already
  // initialized in setupEEPROM(), so this is just a safety check
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("[WiFi ERROR] Failed to initialize EEPROM!");
    delay(1000);
  }

  char ssid[MAX_SSID_LENGTH] = {0};
  char password[MAX_PASSWORD_LENGTH] = {0};
  char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
  Serial.println("[WiFi DEBUG] Checking if EEPROM is initialized...");
  Serial.println("[WiFi DEBUG] EEPROM size: " + String(EEPROM_SIZE));

  // Try to load saved credentials
  if (loadWifiCredentials(ssid, password, deviceName) && strlen(ssid) > 0) {
    Serial.println("[WiFi] Saved credentials found. Attempting to connect to WiFi...");
    if (connectToWifi(ssid, password)) {
      _currentMode = WIFI_MANAGER_MODE_STA;
      
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
    } else {
      Serial.println("[WiFi] Failed to connect. Starting AP mode...");
      startAPMode();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
    }
  } else {
    Serial.println("[WiFi] No saved credentials. Starting AP mode...");
    startAPMode();
    _currentMode = WIFI_MANAGER_MODE_AP;
  }
}

bool WifiManager::startAPMode() {
  Serial.println("[WiFi] Starting Access Point mode...");
  
  // Create a unique AP name using chip ID if needed
  char apName[32];
  uint32_t chipId = ESP.getEfuseMac() & 0xFFFFFFFF;
  
  // Check if we have a device name saved
  char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
  char ssid[MAX_SSID_LENGTH] = {0};
  char password[MAX_PASSWORD_LENGTH] = {0};
  
  if (loadWifiCredentials(ssid, password, deviceName) && strlen(deviceName) > 0) {
    sprintf(apName, "AmbiSense-%s-Setup", deviceName);
  } else {
    sprintf(apName, "AmbiSense-%08X-Setup", chipId);
  }
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apName, WIFI_AP_PASSWORD);
  
  Serial.print("[WiFi] AP started. Name: ");
  Serial.println(apName);
  Serial.print("[WiFi] AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  return true;
}

bool WifiManager::connectToWifi(const char* ssid, const char* password) {
  if (strlen(ssid) == 0) {
    Serial.println("[WiFi] SSID is empty, cannot connect");
    return false;
  }
  
  Serial.print("[WiFi] Connecting to: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  // Wait for connection with timeout
  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    
    if (millis() - startTime > WIFI_CONNECTION_TIMEOUT) {
      Serial.println("\n[WiFi] Connection timeout!");
      return false;
    }
  }
  
  Serial.println();
  Serial.print("[WiFi] Connected! IP address: ");
  Serial.println(WiFi.localIP());
  
  _connectionAttempts = 0;
  return true;
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
  }
  
  Serial.print("[mDNS] Setting up mDNS responder with hostname: ");
  Serial.println(hostname);
  
  if (!MDNS.begin(hostname)) {
    Serial.println("[mDNS] Error setting up mDNS responder!");
    return false;
  }
  
  // Add service to mDNS
  MDNS.addService("http", "tcp", 80);
  Serial.print("[mDNS] mDNS responder started at http://");
  Serial.print(hostname);
  Serial.println(".local");
  
  _mDNSStarted = true;
  return true;
}

void WifiManager::saveWifiCredentials(const char* ssid, const char* password, const char* deviceName) {
  Serial.println("[WiFi] Saving WiFi credentials to EEPROM");
  Serial.println("[WiFi DEBUG] SSID to save: " + String(ssid));
  Serial.println("[WiFi DEBUG] SSID length: " + String(strlen(ssid)));
  
  // Clear the credential area first
  for (int i = EEPROM_WIFI_START; 
       i < EEPROM_WIFI_START + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_DEVICE_NAME_LENGTH + 1; 
       i++) {
    EEPROM.write(i, 0);
  }
  
  // Write SSID (with length checking)
  for (int i = 0; i < MAX_SSID_LENGTH && ssid[i] != '\0'; i++) {
    EEPROM.write(EEPROM_WIFI_SSID_ADDR + i, ssid[i]);
  }
  
  // Write password (with length checking)
  for (int i = 0; i < MAX_PASSWORD_LENGTH && password[i] != '\0'; i++) {
    EEPROM.write(EEPROM_WIFI_PASS_ADDR + i, password[i]);
  }
  
  // Write device name (with length checking)
  for (int i = 0; i < MAX_DEVICE_NAME_LENGTH && deviceName[i] != '\0'; i++) {
    EEPROM.write(EEPROM_DEVICE_NAME_ADDR + i, deviceName[i]);
  }
  
  // Write current mode
  EEPROM.write(EEPROM_WIFI_MODE_ADDR, (uint8_t)_currentMode);
  
  // Commit changes to flash - CRITICAL for ESP32
  if (EEPROM.commit()) {
    Serial.println("[WiFi] WiFi credentials committed to EEPROM successfully");
  } else {
    Serial.println("[WiFi ERROR] Failed to commit WiFi credentials to EEPROM");
  }
  
  // Verify what was written
  char verifySSID[MAX_SSID_LENGTH] = {0};
  for (int i = 0; i < MAX_SSID_LENGTH && i < strlen(ssid); i++) {
    verifySSID[i] = EEPROM.read(EEPROM_WIFI_SSID_ADDR + i);
  }
  Serial.println("[WiFi DEBUG] Verified SSID: " + String(verifySSID));
  
  Serial.println("[WiFi] WiFi credentials saved");
}

bool WifiManager::loadWifiCredentials(char* ssid, char* password, char* deviceName) {
  Serial.println("[WiFi] Loading WiFi credentials from EEPROM");
  
  // Read SSID
  for (int i = 0; i < MAX_SSID_LENGTH; i++) {
    ssid[i] = EEPROM.read(EEPROM_WIFI_SSID_ADDR + i);
  }
  ssid[MAX_SSID_LENGTH - 1] = '\0'; // Ensure null termination
  Serial.println("[WiFi DEBUG] Loaded SSID: " + String(ssid));
  Serial.println("[WiFi DEBUG] Loaded SSID length: " + String(strlen(ssid)));
  
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
  
  // Read saved mode
  uint8_t savedMode = EEPROM.read(EEPROM_WIFI_MODE_ADDR);
  if (savedMode < WIFI_MANAGER_MODE_AP || savedMode > WIFI_MANAGER_MODE_FALLBACK) {
    // Invalid mode, default to AP
    savedMode = WIFI_MANAGER_MODE_AP;
  }
  _currentMode = (WifiManagerMode)savedMode;
  
  // Check if there are any saved credentials
  return (strlen(ssid) > 0);
}

void WifiManager::resetWifiSettings() {
  Serial.println("[WiFi] Resetting WiFi settings");
  
  // Clear the credential area
  for (int i = EEPROM_WIFI_START; 
       i < EEPROM_WIFI_START + MAX_SSID_LENGTH + MAX_PASSWORD_LENGTH + MAX_DEVICE_NAME_LENGTH + 1; 
       i++) {
    EEPROM.write(i, 0);
  }
  
  // Set mode to AP
  EEPROM.write(EEPROM_WIFI_MODE_ADDR, WIFI_MANAGER_MODE_AP);
  
  // Commit changes to flash
  EEPROM.commit();
  
  Serial.println("[WiFi] WiFi settings reset. Restarting...");
  
  // Restart to apply changes
  ESP.restart();
}

bool WifiManager::isConnected() {
  return (_currentMode == WIFI_MANAGER_MODE_STA && WiFi.status() == WL_CONNECTED);
}

String WifiManager::getIPAddress() {
  // First check the actual connection status
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  } else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) {
    return WiFi.softAPIP().toString();
  } else {
    // Fall back to mode-based check
    if (_currentMode == WIFI_MANAGER_MODE_AP || _currentMode == WIFI_MANAGER_MODE_FALLBACK) {
      return WiFi.softAPIP().toString();
    } else if (_currentMode == WIFI_MANAGER_MODE_STA) {
      return WiFi.localIP().toString();
    }
  }
  return "0.0.0.0"; // Default if no IP is available
}

WifiManagerMode WifiManager::getMode() {
  return _currentMode;
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
    // First, check if it already starts with the prefix
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
  
  Serial.println("[WiFi DEBUG] Sanitized hostname: " + hostname);
  
  return hostname;
}

void WifiManager::process() {
  unsigned long now = millis();

  // Update LED status
  updateLedStatus();

  // Periodic Wi-Fi reconnect check
  if (_currentMode == WIFI_MANAGER_MODE_STA && (now - _lastWifiCheck > WIFI_CHECK_INTERVAL)) {
    _lastWifiCheck = now;
    checkWifiConnection();
  }
}

void WifiManager::updateLedStatus() {
  unsigned long now = millis();
  
  if (_currentMode == WIFI_MANAGER_MODE_STA && WiFi.status() == WL_CONNECTED) {
    // Connected mode: Double blink every 5 seconds
    if (now - _lastLedUpdateTime >= 100) { // 100ms timing for blinks
      _lastLedUpdateTime = now;
      
      switch (_ledBlinkPhase) {
        case 0: // First LED ON
          digitalWrite(WIFI_STATUS_LED_PIN, LOW); // ON
          _ledBlinkPhase++;
          break;
        case 1: // First LED OFF
          digitalWrite(WIFI_STATUS_LED_PIN, HIGH); // OFF
          _ledBlinkPhase++;
          break;
        case 2: // Short pause
          // Just increment the phase
          _ledBlinkPhase++;
          break;
        case 3: // Second LED ON
          digitalWrite(WIFI_STATUS_LED_PIN, LOW); // ON
          _ledBlinkPhase++;
          break;
        case 4: // Second LED OFF
          digitalWrite(WIFI_STATUS_LED_PIN, HIGH); // OFF
          _ledBlinkPhase++;
          break;
        case 5: // Long pause (about 4.5 seconds)
          if (now - _lastLedUpdateTime >= 4500) {
            _ledBlinkPhase = 0; // Reset to start new cycle
          }
          break;
        default:
          _ledBlinkPhase = 0; // Reset if invalid phase
          break;
      }
    }
  } else {
    // AP/Fallback Mode: Fast blink (200ms)
    if (now - _lastLedUpdateTime >= 200) {
      _lastLedUpdateTime = now;
      _ledState = !_ledState;
      digitalWrite(WIFI_STATUS_LED_PIN, _ledState ? LOW : HIGH); // Toggle (inverted)
    }
  }
}

void WifiManager::checkWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection lost, attempting to reconnect...");
    
    _connectionAttempts++;
    
    if (_connectionAttempts > MAX_CONNECTION_ATTEMPTS) {
      Serial.println("[WiFi] Max connection attempts reached. Switching to AP mode...");
      startAPMode();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
      return;
    }
    
    // Attempt to reconnect
    char ssid[MAX_SSID_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
    
    if (loadWifiCredentials(ssid, password, deviceName)) {
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
      
      // Quick check for reconnection
      unsigned long startTime = millis();
      while (WiFi.status() != WL_CONNECTED && (millis() - startTime < WIFI_CONNECTION_TIMEOUT)) {
        delay(500);
        Serial.print(".");
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Reconnected successfully");
        _connectionAttempts = 0;
        _currentMode = WIFI_MANAGER_MODE_STA; // Ensure mode is set correctly
      } else {
        Serial.println("\n[WiFi] Reconnect failed");
      }
    }
  }
}

std::vector<WiFiNetwork> WifiManager::scanNetworks() {
  Serial.println("[WiFi] Scanning for networks...");
  
  std::vector<WiFiNetwork> networks;
  
  // Start network scan (async=false is more reliable)
  int networksFound = WiFi.scanNetworks(false, true); // Non-blocking, show hidden networks
  
  if (networksFound == 0) {
    Serial.println("[WiFi] No networks found");
  } else {
    Serial.printf("[WiFi] %d networks found\n", networksFound);
    
    // Create a vector of networks
    for (int i = 0; i < networksFound; ++i) {
      WiFiNetwork network;
      
      // Get raw SSID
      String rawSSID = WiFi.SSID(i);
      
      // Clean SSID - aggressively filter to only allow alphanumeric, space, and common punctuation
      String cleanSSID = "";
      for (unsigned int j = 0; j < rawSSID.length(); j++) {
        char c = rawSSID.charAt(j);
        // Accept only:
        // - Alphanumeric characters (a-z, A-Z, 0-9)
        // - Common punctuation and space
        if (isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.' || 
            c == '(' || c == ')' || c == '[' || c == ']' || c == '+') {
          cleanSSID += c;
        }
      }
      
      // If nothing valid is left or SSID is empty, mark as hidden
      network.ssid = cleanSSID.length() > 0 ? cleanSSID : "[Hidden Network]";
      network.rssi = WiFi.RSSI(i);
      network.encType = WiFi.encryptionType(i);
      
      networks.push_back(network);
    }
    
    // Sort networks by signal strength (RSSI)
    std::sort(networks.begin(), networks.end(), 
      [](const WiFiNetwork& a, const WiFiNetwork& b) {
        return a.rssi > b.rssi; // Higher RSSI (less negative) is better
      });
  }
  
  // Free memory used by scan
  WiFi.scanDelete();
  
  return networks;
}