/**
 * AmbiSense v5.1.1 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (TechPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * 
 * This version includes optimizations:
 * - Reduced logging for better performance
 * - Standard WebServer instead of AsyncWebServer
 * - Optimized memory usage and reliability
 * - ESP-NOW master-slave support for L/U-shaped stairs
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

// Function to check for factory reset during boot
void checkForFactoryReset() {
  // Check if reset button is held during boot
  if (digitalRead(WIFI_RESET_BUTTON_PIN) == LOW) {
    Serial.println("Factory reset button detected at startup");
    delay(3000); // Wait to confirm it's held
    if (digitalRead(WIFI_RESET_BUTTON_PIN) == LOW) {
      Serial.println("FACTORY RESET: Resetting all settings to defaults");
      resetAllSettings();
      // Visual confirmation
      for(int i=0; i<3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
        delay(200);
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(100); // Give serial a moment to initialize
  
  Serial.println("\n\nAmbiSense v4.3.0 - Radar-Controlled LED System");
  Serial.println("Copyright © 2025 TechPosts Media.");

  // Set up reset button
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Check for factory reset before initializing EEPROM
  checkForFactoryReset();

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
  
  // Initialize EEPROM before WiFi to ensure settings are loaded
  setupEEPROM();
  
  // Initialize hardware components
  setupLEDs();
  updateLEDConfig();
  
  setupRadar();
  
  // Initialize WiFi AFTER other hardware is set up
  wifiManager.begin();
  
  // Initialize ESP-NOW for master-slave communication
  setupESPNOW();
  
  // Setup Web server LAST after WiFi is connected
  setupWebServer();
  
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("System ready.");
  
  // Log current device role
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

  // Process WiFi and radar
  wifiManager.process();
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
  
  // Check if WiFi reset button is held for a long time
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
      wifiManager.resetWifiSettings();
    }
  } else {
    buttonPressStartTime = 0;
    longPressDetected = false;
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