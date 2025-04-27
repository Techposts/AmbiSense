/**
 * AmbiSense v4.1.1 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (TechPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * It has a custom Home Assistant integration that works for HA.
 * Hardware: ESP32 + LD2410 Radar Sensor + NeoPixel LED Strip
 *
 * Organization:
 * - config.h: Global constants and configuration
 * - eeprom_manager: Handles saving/loading settings
 * - led_controller: Controls NeoPixel LED strip
 * - radar_manager: Handles LD2410 radar sensor
 * - web_interface: Web server and UI
 * - wifi_manager: WiFi access point setup and mDNS support
 *
 * Features:
 * - Radar-controlled LED lighting
 * - Directional light trails
 * - Multiple lighting effects
 * - Background lighting mode
 * - Web interface for configuration
 * - WiFi network connection with mDNS support
 * - Physical button: Short press toggles system ON/OFF, Long press resets Wi-Fi
 */

#include <Arduino.h>
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

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n");
  Serial.println("***************************************");
  Serial.println("* AmbiSense v4.1.1                    *");
  Serial.println("* Radar-Controlled LED System         *");
  Serial.println("* With WiFi & mDNS Support            *");
  Serial.println("* Created by Ravi Singh               *");
  Serial.println("* TechPosts Media                     *");
  Serial.println("* Copyright © 2025. All rights reserved *");
  Serial.println("***************************************");
  Serial.println("\n");

  setupEEPROM();
  Serial.println("EEPROM initialized");
  wifiManager.begin();
  Serial.println("WiFi manager started");
  setupLEDs();
  updateLEDConfig();
  Serial.println("LEDs initialized");
  setupRadar();
  Serial.println("Radar initialized");
  setupWebServer();
  Serial.println("Web server started");

  pinMode(WIFI_RESET_BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Initialization complete. System ready.");
}

void loop() {
  // Handle button press detection
  bool buttonPressed = digitalRead(WIFI_RESET_BUTTON_PIN) == LOW;

  if (buttonPressed && !buttonPreviouslyPressed) {
    buttonPressStart = millis();
    buttonPreviouslyPressed = true;
  }

  if (!buttonPressed && buttonPreviouslyPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;

    if (pressDuration >= LONG_PRESS_TIME) {
      Serial.println("Long press detected → Resetting Wi-Fi...");
      wifiManager.resetWifiSettings();
      delay(100); // Short delay
      ESP.restart(); // Safer restart than long blocking delay
    } else if (pressDuration >= 50 && pressDuration <= SHORT_PRESS_TIME) {
      systemEnabled = !systemEnabled;
      Serial.println(systemEnabled ? "System ON" : "System OFF");
    }

    buttonPreviouslyPressed = false;
  }

  if (!systemEnabled) {
    for (int i = 0; i < numLeds; i++) {
      strip.setPixelColor(i, 0);
    }
    strip.show();
    handleWebServer();
    wifiManager.process();
    return;
  }

  // Process WiFi events less frequently to reduce overhead
  static unsigned long lastWifiProcessTime = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastWifiProcessTime >= 100) { // Every 100ms is sufficient
    lastWifiProcessTime = currentMillis;
    wifiManager.process();
  }
  
  // Handle web server more frequently
  handleWebServer();
  
  // Process radar readings at highest priority
  processRadarReading();

  // Update LEDs at animation interval
  if (currentMillis - previousMillis >= animationInterval) {
    previousMillis = currentMillis;

    if (lightMode != LIGHT_MODE_STANDARD) {
      updateLEDs(currentDistance);
    }
  }
}
