/**
 * AmbiSense v5.1.1 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (TechPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * 
 * This version includes critical fixes:
 * - Fixed factory reset false triggering
 * - Reverted problematic MAC address logic
 * - Simplified initialization sequence
 *
 * Hardware: ESP32 + LD2410 Radar Sensor + NeoPixel LED Strip
 */   

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <EEPROM.h>  
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "radar_manager.h"
#include "web_interface.h"
#include "wifi_manager.h"
#include "espnow_manager.h"  // For ESP-NOW support

#define WIFI_RESET_BUTTON_PIN 7
#define SHORT_PRESS_TIME 2000
#define LONG_PRESS_TIME 10000

// Global variables for ESP-NOW
uint8_t deviceRole = DEFAULT_DEVICE_ROLE;
uint8_t masterAddress[6] = {0};
uint8_t slaveAddresses[MAX_SLAVE_DEVICES][6] = {0};
uint8_t numSlaveDevices = 0;

unsigned long buttonPressStart = 0;
bool buttonPreviouslyPressed = false;
bool systemEnabled = true;

unsigned long previousMillis = 0;
int animationInterval = 30;

// Flags for handling WiFi reset and device restart
bool shouldResetWifi = false;
bool shouldRestartDevice = false;
unsigned long resetRequestTime = 0;

int ledSegmentMode = DEFAULT_LED_SEGMENT_MODE;
int ledSegmentStart = DEFAULT_LED_SEGMENT_START;  
int ledSegmentLength = DEFAULT_LED_SEGMENT_LENGTH;
int totalSystemLeds = DEFAULT_TOTAL_SYSTEM_LEDS;

// FIXED: Proper factory reset function with button validation
void checkForFactoryReset() {
  // CRITICAL: Only check if button pin is properly configured
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);
  delay(50); // Allow pin to stabilize
  
  // Check multiple times to ensure it's not a false reading
  bool buttonHeld = true;
  for (int i = 0; i < 5; i++) {
    if (digitalRead(WIFI_RESET_BUTTON_PIN) != LOW) {
      buttonHeld = false;
      break;
    }
    delay(100);
  }
  
  if (!buttonHeld) {
    return; // Button not consistently held, exit
  }
  
  Serial.println("Factory reset button confirmed - button is being held");
  delay(2000); // Additional confirmation time
  
  // Final check - button must still be held
  if (digitalRead(WIFI_RESET_BUTTON_PIN) == LOW) {
    Serial.println("FACTORY RESET: Resetting all settings to defaults");
    
    // Visual confirmation
    for(int i=0; i<3; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(200);
      digitalWrite(LED_BUILTIN, LOW);
      delay(200);
    }
    
    // Perform the reset
    resetAllSettings();
    
    // Additional visual confirmation
    for(int i=0; i<5; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    
    Serial.println("Factory reset complete");
  } else {
    Serial.println("Factory reset cancelled - button released");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100); // Give serial a moment to initialize
  
  Serial.println("\n\nAmbiSense v5.1 - Radar-Controlled LED System");
  Serial.println("Copyright © 2025 TechPosts Media.");
  Serial.println("Enhanced with WiFi/ESP-NOW fixes");

  // Set up pins early
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);

  // Initialize EEPROM with better error handling
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("ERROR: Failed to initialize EEPROM!");
    delay(1000);
    // Flash LED to indicate error
    for(int i=0; i<5; i++) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    // Try again after a delay
    delay(500);
    if (!EEPROM.begin(EEPROM_SIZE)) {
      Serial.println("CRITICAL: EEPROM failed to initialize after retry!");
      // Continue but settings won't persist
    }
  }
  
  // Check for factory reset ONLY if button is actually connected and held
  checkForFactoryReset();

  // Initialize EEPROM before WiFi to ensure settings are loaded
  setupEEPROM();
  
  // Initialize hardware components BEFORE WiFi
  Serial.println("Initializing hardware components...");
  setupLEDs();
  updateLEDConfig();
  setupRadar();
  
  // CRITICAL: Initialize WiFi FIRST, let it stabilize
  Serial.println("Initializing WiFi manager...");
  wifiManager.begin();
  
  // Wait for WiFi to be stable before initializing ESP-NOW
  Serial.println("Waiting for WiFi to stabilize...");
  delay(2000);  // Give WiFi time to stabilize
  
  // Check WiFi status
  if (wifiManager.getMode() == WIFI_MANAGER_MODE_STA) {
    Serial.printf("WiFi connected on channel %d\n", WiFi.channel());
  } else {
    Serial.printf("WiFi in AP mode on channel %d\n", WiFi.channel());
  }
  
  // SIMPLIFIED: Initialize ESP-NOW without complex MAC checking
  Serial.println("Initializing ESP-NOW...");
  setupESPNOW();
  
  // Verify ESP-NOW initialization
  if (isESPNowInitialized()) {
    Serial.printf("ESP-NOW initialized successfully on channel %d\n", getCurrentESPNowChannel());
    
    // Verify channel alignment
    int wifiChannel = WiFi.channel();
    int espnowChannel = getCurrentESPNowChannel();
    if (wifiChannel != espnowChannel) {
      Serial.printf("WARNING: Channel mismatch - WiFi: %d, ESP-NOW: %d\n", 
                   wifiChannel, espnowChannel);
    } else {
      Serial.println("Channel alignment verified");
    }
  } else {
    Serial.println("WARNING: ESP-NOW initialization failed or incomplete");
  }
  
  // Setup Web server LAST after WiFi and ESP-NOW are initialized
  Serial.println("Starting web server...");
  setupWebServer();

  Serial.println("System ready.");
  
  // Log current device role and configuration
  if (deviceRole == DEVICE_ROLE_MASTER) {
    Serial.printf("Device configured as MASTER with %d paired slaves.\n", numSlaveDevices);
  } else if (deviceRole == DEVICE_ROLE_SLAVE) {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            masterAddress[0], masterAddress[1], masterAddress[2], 
            masterAddress[3], masterAddress[4], masterAddress[5]);
    Serial.printf("Device configured as SLAVE. Master: %s\n", macStr);
  }
  
  // Show min/max distance configuration
  Serial.printf("Min Distance: %d cm, Max Distance: %d cm\n", minDistance, maxDistance);
  
  // Log the CRC for debugging
  uint8_t currentCRC = calculateSystemCRC();
  Serial.printf("Settings CRC: 0x%02X\n", currentCRC);
  
  // Print final diagnostics
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("WiFi Mode: %d, Channel: %d\n", wifiManager.getMode(), WiFi.channel());
  Serial.printf("ESP-NOW initialized: %s\n", isESPNowInitialized() ? "Yes" : "No");
}

void loop() {
  // Handle button input
  handleButton();
  
  // Process pending actions
  handlePendingActions();
  
  // Process web server client requests
  server.handleClient();
  
  if (!systemEnabled) {
    for (int i = 0; i < numLeds; i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    wifiManager.process();
    return;
  }

  // CRITICAL: Process WiFi manager first (includes connection monitoring)
  wifiManager.process();
  
  // CRITICAL: Process ESP-NOW maintenance (includes channel sync)
  espnowMaintenance();
  
  // Process radar readings
  processRadarReading();

  // Handle animation updates at specified interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= animationInterval) {
    previousMillis = currentMillis;

    // Only use updateLEDs directly for non-standard modes
    // Standard mode is handled in processRadarReading() and ESP-NOW logic
    if (lightMode != LIGHT_MODE_STANDARD) {
      updateLEDs(currentDistance);
    }
  }
  
  // Enhanced WiFi reset button handling
  static bool longPressDetected = false;
  static unsigned long buttonPressStartTime = 0;
  
  if (digitalRead(WIFI_RESET_BUTTON_PIN) == LOW) {
    if (buttonPressStartTime == 0) {
      buttonPressStartTime = millis();
    } else if (!longPressDetected && millis() - buttonPressStartTime > 5000) {
      // 5 second long press
      longPressDetected = true;
      
      // Visual feedback
      for (int i = 0; i < 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
      }
      
      Serial.println("[WiFi] Factory reset detected: Clearing WiFi settings");
      Serial.println("[ESP-NOW] ESP-NOW will be reinitialized after WiFi reset");
      
      wifiManager.resetWifiSettings();
    }
  } else {
    buttonPressStartTime = 0;
    longPressDetected = false;
  }
  
  // Periodic diagnostics (every 60 seconds)
  static unsigned long lastDiagnostics = 0;
  if (millis() - lastDiagnostics > 60000) {
    lastDiagnostics = millis();
    
    // Quick health check
    Serial.printf("Health: WiFi=%s, ESP-NOW=%s, Heap=%d\n",
                 wifiManager.isConnected() ? "OK" : "FAIL",
                 isESPNowInitialized() ? "OK" : "FAIL",
                 ESP.getFreeHeap());
                 
    // Check for channel mismatches
    if (isESPNowInitialized()) {
      int wifiCh = WiFi.channel();
      int espnowCh = getCurrentESPNowChannel();
      if (wifiCh != espnowCh) {
        Serial.printf("WARNING: Channel mismatch - WiFi:%d ESP-NOW:%d\n", wifiCh, espnowCh);
      }
    }
  }
}

void handleButton() {
  bool buttonPressed = digitalRead(WIFI_RESET_BUTTON_PIN) == LOW;

  if (buttonPressed && !buttonPreviouslyPressed) {
    buttonPressStart = millis();
    buttonPreviouslyPressed = true;
  }

  if (!buttonPressed && buttonPreviouslyPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;

    if (pressDuration >= LONG_PRESS_TIME) {
      Serial.println("Long press → Scheduling WiFi reset");
      shouldResetWifi = true;
      resetRequestTime = millis();
    } else if (pressDuration >= 50 && pressDuration <= SHORT_PRESS_TIME) {
      systemEnabled = !systemEnabled;
      Serial.println(systemEnabled ? "System ON" : "System OFF");
    }

    buttonPreviouslyPressed = false;
  }
}

void handlePendingActions() {
  // Handle WiFi reset if requested from button or web interface
  if (shouldResetWifi && (millis() - resetRequestTime > 2000)) {
    shouldResetWifi = false;
    Serial.println("Executing WiFi reset...");
    wifiManager.resetWifiSettings();
    // resetWifiSettings() will restart the device
  }
  
  // Handle device restart if requested from web interface
  if (shouldRestartDevice && (millis() - resetRequestTime > 2000)) {
    shouldRestartDevice = false;
    Serial.println("Restarting device...");
    ESP.restart();
  }
}