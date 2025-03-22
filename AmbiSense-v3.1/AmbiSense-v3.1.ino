/*
 * AmbiSense v3.1 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (TechPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * 
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
 */

#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"
#include "radar_manager.h"
#include "web_interface.h"
#include "wifi_manager.h"

// For managing animation timing
unsigned long previousMillis = 0;
int animationInterval = 30; // milliseconds between animation frames

void setup() {
  // Initialize serial first for debugging
  Serial.begin(115200);
  Serial.println("\n\n");
  Serial.println("***************************************");
  Serial.println("* AmbiSense v3.0                      *");
  Serial.println("* Radar-Controlled LED System         *");
  Serial.println("* With WiFi & mDNS Support            *");
  Serial.println("* Created by Ravi Singh               *");
  Serial.println("* TechPosts Media                     *");
  Serial.println("* Copyright © 2025. All rights reserved *");
  Serial.println("***************************************");
  Serial.println("\n");
  
  // Critical initialization sequence:
  // 1. EEPROM first to load settings
  setupEEPROM();     
  
  // 2. Initialize WiFi manager (early for network setup)
  wifiManager.begin();
  
  // 3. LED strip after settings are loaded
  setupLEDs();       
  
  // 4. Update LED configuration with the loaded settings
  updateLEDConfig();
  
  // 5. Initialize radar sensor
  setupRadar();      
  
  // 6. Start web server
  setupWebServer();  
  
  Serial.println("Initialization complete. System ready.");
}

void loop() {
  // Process WiFi events and maintain connection
  wifiManager.process();
  
  // Handle web server requests
  handleWebServer();
  
  // Process radar readings and update LEDs
  processRadarReading();
  
  // Handle animation updates for effect modes
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= animationInterval) {
    previousMillis = currentMillis;
    
    // For light modes that need continuous updates even without motion
    if (lightMode != LIGHT_MODE_STANDARD) {
      updateLEDs(currentDistance);
    }
  }
}