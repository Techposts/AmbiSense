#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <vector>

// Define constants here for use in other files
#define MAX_SSID_LENGTH 32
#define MAX_PASSWORD_LENGTH 64
#define MAX_DEVICE_NAME_LENGTH 32

#define WIFI_STATUS_LED_PIN 8


// Wi-Fi operation modes
enum WifiManagerMode {
  WIFI_MANAGER_MODE_AP,       // Access Point mode (initial setup)
  WIFI_MANAGER_MODE_STA,      // Station mode (connected to home network)
  WIFI_MANAGER_MODE_FALLBACK  // Fallback to AP mode when connection fails
};

// Network information structure
struct WiFiNetwork {
  String ssid;
  int32_t rssi;
  uint8_t encType;
};

// Wi-Fi manager class
class WifiManager {
public:
  // Initialize WiFi manager
  void begin();

  // Setup Access Point mode
  bool startAPMode();

  // Connect to Wi-Fi network in Station mode
  bool connectToWifi(const char* ssid, const char* password);

  // Setup mDNS with given hostname
  bool setupMDNS(const char* hostname);

  // Save Wi-Fi credentials to EEPROM
  void saveWifiCredentials(const char* ssid, const char* password, const char* deviceName);

  // Load Wi-Fi credentials from EEPROM
  bool loadWifiCredentials(char* ssid, char* password, char* deviceName);

  // Reset Wi-Fi settings (clear EEPROM)
  void resetWifiSettings();

  // Check if device is connected to Wi-Fi
  bool isConnected();

  // Get current IP address (AP or STA)
  String getIPAddress();

  // Get current Wi-Fi mode
  WifiManagerMode getMode();

  // Get sanitized mDNS hostname (convert to lowercase, replace spaces with hyphens)
  String getSanitizedHostname(const char* deviceName);

  // Process WiFi events and maintain connection
  void process();
  
  // Scan for available networks and return top results
  std::vector<WiFiNetwork> scanNetworks();
  void updateLedStatus();
  void checkWifiConnection();



private:
  WifiManagerMode _currentMode = WIFI_MANAGER_MODE_AP;
  unsigned long _lastWifiCheck = 0;
  int _connectionAttempts = 0;
  bool _mDNSStarted = false;
   
  // Attempt Wi-Fi reconnection if disconnected
 
};

// Global instance
extern WifiManager wifiManager;

#endif // WIFI_MANAGER_H