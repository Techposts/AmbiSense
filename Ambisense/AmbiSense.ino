/**
 * AmbiSense v4.1 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (TechPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * 
 * This version includes optimizations:
 * - Asynchronous web server for better performance
 * - PROGMEM for storing web UI assets
 * - SPIFFS for additional file storage
 * - Improved reliability and responsiveness
 *
 * Hardware: ESP32 + LD2410 Radar Sensor + NeoPixel LED Strip
 */

#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "radar_manager.h"
#include "web_interface.h"
#include "wifi_manager.h"

#define WIFI_RESET_BUTTON_PIN 7
#define SHORT_PRESS_TIME 2000
#define LONG_PRESS_TIME 10000

unsigned long buttonPressStart = 0;
bool buttonPreviouslyPressed = false;
bool systemEnabled = true;

unsigned long previousMillis = 0;
int animationInterval = 30;

// Flag for handling WiFi reset
bool shouldResetWifi = false;
bool shouldRestartDevice = false;
unsigned long resetRequestTime = 0;

void setup() {
  Serial.begin(115200);
  delay(100); // Give serial a moment to initialize
  
  Serial.println("\n\n");
  Serial.println("***************************************");
  Serial.println("* AmbiSense v4.0.5                    *");
  Serial.println("* Radar-Controlled LED System         *");
  Serial.println("* With WiFi & mDNS Support            *");
  Serial.println("* Created by Ravi Singh               *");
  Serial.println("* TechPosts Media                     *");
  Serial.println("* Copyright © 2025. All rights reserved *");
  Serial.println("***************************************");
  Serial.println("\n");

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)) {
    Serial.println("An error occurred while mounting SPIFFS");
  } else {
    Serial.println("SPIFFS mounted successfully");
  }

  // Initialize EEPROM before WiFi to ensure settings are loaded
  setupEEPROM();
  Serial.println("EEPROM initialized");
  
  // Initialize hardware components
  setupLEDs();
  updateLEDConfig();
  Serial.println("LEDs initialized");
  
  setupRadar();
  Serial.println("Radar initialized");
  
  // Initialize WiFi AFTER other hardware is set up
  wifiManager.begin();
  Serial.println("WiFi manager started");
  
  // Setup Web server LAST after WiFi is connected
  // Delay slightly to ensure WiFi is stable
  delay(500);
  setupWebServer();
  
  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Initialization complete. System ready.");
}

void loop() {
  // Handle button input
  handleButton();
  
  // Process pending actions that need to be done outside of web handlers
  handlePendingActions();
  
  if (!systemEnabled) {
    for (int i = 0; i < numLeds; i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    wifiManager.process();
    return;
  }

  // Process WiFi and radar in every loop iteration
  wifiManager.process();
  processRadarReading();

  // Handle animation updates at specified interval
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= animationInterval) {
    previousMillis = currentMillis;

    if (lightMode != LIGHT_MODE_STANDARD) {
      updateLEDs(currentDistance);
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
      Serial.println("Long press detected → Scheduling WiFi reset...");
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