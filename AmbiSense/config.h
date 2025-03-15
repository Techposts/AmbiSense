#ifndef CONFIG_H
#define CONFIG_H

/*
 * AmbiSense - Radar-Controlled LED System
 * Created by Ravi Singh (techPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 * 
 * Hardware: ESP32 + LD2410 Radar Sensor + NeoPixel LED Strip
 */

// 🛠 LED & Sensor Config
#define LED_PIN 5
#define DEFAULT_NUM_LEDS 300
#define DEFAULT_BRIGHTNESS 255
#define DEFAULT_MOVING_LIGHT_SPAN 40
#define DEFAULT_MIN_DISTANCE 30
#define DEFAULT_MAX_DISTANCE 300
#define EEPROM_INITIALIZED_MARKER 123
#define EEPROM_SIZE 32

// Default color (white)
#define DEFAULT_RED 255
#define DEFAULT_GREEN 255
#define DEFAULT_BLUE 255

// 🎯 LD2410 Config
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 3
#define RADAR_TX_PIN 4

// 📡 Wi-Fi Access Point
#define WIFI_AP_SSID "AmbiSense"
#define WIFI_AP_PASSWORD "12345678"

// 📡 Web Server
#define WEB_SERVER_PORT 80

// Global variables
extern int minDistance, maxDistance, brightness, movingLightSpan, numLeds;
extern int redValue, greenValue, blueValue;
extern int currentDistance;

#endif // CONFIG_H