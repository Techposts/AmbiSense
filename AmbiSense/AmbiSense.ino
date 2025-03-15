/*
 * AmbiSense - Radar-Controlled LED System
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
 * - wifi_manager: WiFi access point setup
 */

 #include <Arduino.h>
 #include "config.h"
 #include "eeprom_manager.h"
 #include "led_controller.h"
 #include "radar_manager.h"
 #include "web_interface.h"
 #include "wifi_manager.h"
 
 void setup() {
   Serial.begin(115200);
   Serial.println("\n\n");
   Serial.println("***************************************");
   Serial.println("* AmbiSense                           *");
   Serial.println("* Radar-Controlled LED System         *");
   Serial.println("* Created by Ravi Singh               *");
   Serial.println("* TechPosts Media                     *");
   Serial.println("* Copyright © 2025. All rights reserved *");
   Serial.println("***************************************");
   Serial.println("\n");
   
   // Initialize all modules
   setupEEPROM();     // Must be first to load settings
   setupLEDs();       // Initialize LED strip
   setupRadar();      // Initialize radar sensor
   setupWiFi();       // Setup WiFi access point
   setupWebServer();  // Start web server
 }
 
 void loop() {
   // Handle web server requests
   handleWebServer();
   
   // Process radar readings and update LEDs
   processRadarReading();
 }