#include <WiFi.h>
#include <EEPROM.h>
#include "config.h"
#include "wifi_manager.h"

// Connection timeout constants
#define WIFI_CONNECTION_TIMEOUT 15000  // 15 seconds
#define WIFI_RECONNECT_DELAY 500      // Wait between reconnect attempts
#define MAX_CONNECTION_ATTEMPTS 2      // Attempts before falling back to AP mode
#define WIFI_CHECK_INTERVAL 30000      // Check connection every 30 seconds

// Global instance
WifiManager wifiManager;

void WifiManager::begin() {
  Serial.println("[WiFi] Initializing WiFi manager");
  
  pinMode(WIFI_STATUS_LED_PIN, OUTPUT);
  digitalWrite(WIFI_STATUS_LED_PIN, LOW);

  // Always start with WiFi off to ensure clean state
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(200);  // Give WiFi subsystem time to reset

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
    
    // Try to connect
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
      Serial.println("[WiFi] Failed to connect. Starting AP mode");
      startAPMode();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
    }
  } else {
    Serial.println("[WiFi] No saved credentials. Starting AP mode");
    startAPMode();
    _currentMode = WIFI_MANAGER_MODE_AP;
  }
  
  // Print current connection status
  if (_currentMode == WIFI_MANAGER_MODE_STA) {
    Serial.printf("[WiFi] Connected: IP=%s, RSSI=%d\n", 
                  WiFi.localIP().toString().c_str(),
                  WiFi.RSSI());
  } else {
    Serial.printf("[WiFi] Running in AP mode: IP=%s\n", 
                  WiFi.softAPIP().toString().c_str());
  }
}

bool WifiManager::startAPMode() {
  Serial.println("[WiFi] Starting Access Point mode");
  
  // Use a consistent AP name based on chip ID
  char apName[32];
  uint32_t chipId = ESP.getEfuseMac() & 0xFFF; // Use last 12 bits (3 hex digits)
  sprintf(apName, "AmbiSense-%03X-Setup", chipId);
  
  // First ensure WiFi is in the correct mode
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_OFF);
  delay(100);
  WiFi.mode(WIFI_AP);
  delay(100);
  
  // Start AP with specific channel and hidden option
  bool success = WiFi.softAP(apName, WIFI_AP_PASSWORD, 1, 0, 4);  // Channel 1, not hidden, max 4 clients
  
  if (success) {
    Serial.printf("[WiFi] AP started. Name: %s\n", apName);
    Serial.printf("[WiFi] AP IP Address: %s\n", WiFi.softAPIP().toString().c_str());
  } else {
    Serial.println("[WiFi] Failed to start AP mode!");
  }
  
  return success;
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

bool WifiManager::connectToWifi(const char* ssid, const char* password) {
  // Extra validation to avoid connection attempt with empty/invalid SSID
  if (strlen(ssid) == 0) {
    Serial.println("[WiFi] Cannot connect: SSID is empty");
    return false;
  }
  
  // Log SSID with quotes to show if there are leading/trailing spaces
  Serial.printf("[WiFi] Connecting to: '%s'\n", ssid);
  
  // Check if SSID contains only valid characters
  for (int i = 0; i < strlen(ssid); i++) {
    if (ssid[i] < 32 || ssid[i] > 126) {
      Serial.println("[WiFi] Cannot connect: SSID contains invalid characters");
      return false;
    }
  }
  
  // Disconnect from any previous connection
  WiFi.disconnect(true);
  delay(100);
  
  // Set mode explicitly
  WiFi.mode(WIFI_STA);
  delay(100);
  
  // Start connection
  WiFi.begin(ssid, password);
  
  // Wait for connection with timeout
  unsigned long startTime = millis();
  int dotCount = 0;
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    
    // Print feedback every 500ms (but group dots for readability)
    Serial.print(".");
    dotCount++;
    
    if (dotCount >= 20) {
      Serial.println(); // Start a new line if too many dots
      dotCount = 0;
    }
    
    if (millis() - startTime > WIFI_CONNECTION_TIMEOUT) {
      Serial.println("\n[WiFi] Connection timeout");
      return false;
    }
  }
  
  Serial.println();
  Serial.printf("[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
  
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
  
  Serial.printf("[mDNS] Setting up hostname: %s\n", hostname);
  
  if (!MDNS.begin(hostname)) {
    Serial.println("[mDNS] Error setting up mDNS responder!");
    return false;
  }
  
  // Add service to mDNS
  MDNS.addService("http", "tcp", 80);
  Serial.printf("[mDNS] mDNS responder started at http://%s.local\n", hostname);
  
  _mDNSStarted = true;
  return true;
}

void WifiManager::checkWifiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] Connection lost, attempting to reconnect");
    
    _connectionAttempts++;
    
    if (_connectionAttempts > MAX_CONNECTION_ATTEMPTS) {
      Serial.println("[WiFi] Max reconnection attempts reached. Switching to AP mode");
      
      startAPMode();
      _currentMode = WIFI_MANAGER_MODE_FALLBACK;
      return;
    }
    
    // Attempt to reconnect with clean WiFi state
    WiFi.disconnect(true);
    delay(100);
    
    char ssid[MAX_SSID_LENGTH] = {0};
    char password[MAX_PASSWORD_LENGTH] = {0};
    char deviceName[MAX_DEVICE_NAME_LENGTH] = {0};
    
    if (loadWifiCredentials(ssid, password, deviceName) && strlen(ssid) > 0) {
      // Reconnect
      WiFi.begin(ssid, password);
      
      // Short wait for quick reconnection
      unsigned long startTime = millis();
      Serial.print("[WiFi] Reconnecting");
      
      while (WiFi.status() != WL_CONNECTED && 
             (millis() - startTime < WIFI_CONNECTION_TIMEOUT / 2)) {
        delay(500);
        Serial.print(".");
      }
      
      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Reconnected successfully");
        _connectionAttempts = 0;
        _currentMode = WIFI_MANAGER_MODE_STA;
      } else {
        Serial.println("\n[WiFi] Reconnect failed");
      }
    }
  }
}
std::vector<WiFiNetwork> WifiManager::scanNetworks() {
  std::vector<WiFiNetwork> networks;
  
  Serial.println("[WiFi] Scanning for networks...");
  
  int numNetworks = WiFi.scanNetworks(false, true);  // Non-blocking, include hidden networks
  
  if (numNetworks > 0) {
    Serial.printf("[WiFi] Found %d networks\n", numNetworks);
    
    // Sort networks by signal strength
    for (int i = 0; i < numNetworks; i++) {
      WiFiNetwork network;
      network.ssid = WiFi.SSID(i);
      network.rssi = WiFi.RSSI(i);
      network.encType = WiFi.encryptionType(i);
      
      networks.push_back(network);
    }
    
    // Sort by signal strength
    std::sort(networks.begin(), networks.end(), 
              [](const WiFiNetwork& a, const WiFiNetwork& b) {
                return a.rssi > b.rssi;  // Stronger signal first
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

  // Less frequent WiFi check to reduce processing overhead
  if (_currentMode == WIFI_MANAGER_MODE_STA && (now - _lastWifiCheck > WIFI_CHECK_INTERVAL)) {
    _lastWifiCheck = now;
    checkWifiConnection();
  }
}

void WifiManager::updateLedStatus() {
  unsigned long now = millis();
  
  if (_currentMode == WIFI_MANAGER_MODE_STA && WiFi.status() == WL_CONNECTED) {
    // Connected mode: Blink every 3 seconds
    digitalWrite(WIFI_STATUS_LED_PIN, ((now / 1000) % 3 == 0) ? HIGH : LOW);
  } else {
    // AP/Fallback Mode: Fast blink (500ms)
    digitalWrite(WIFI_STATUS_LED_PIN, ((now / 500) % 2 == 0) ? HIGH : LOW);
  }
}

void WifiManager::resetWifiSettings() {
  Serial.println("[WiFi] Resetting WiFi settings");
  
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