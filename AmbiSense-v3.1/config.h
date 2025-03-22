#ifndef CONFIG_H
#define CONFIG_H

/*
 * AmbiSense - Enhanced Radar-Controlled LED System
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
#define EEPROM_SIZE 512  // Increased to accommodate settings (was 64)

// Default color (white)
#define DEFAULT_RED 255
#define DEFAULT_GREEN 255
#define DEFAULT_BLUE 255

// New default settings
#define DEFAULT_CENTER_SHIFT 0
#define DEFAULT_TRAIL_LENGTH 0
#define DEFAULT_DIRECTION_LIGHT false
#define DEFAULT_BACKGROUND_MODE false
#define DEFAULT_LIGHT_MODE 0

// Light mode constants
#define LIGHT_MODE_STANDARD 0
#define LIGHT_MODE_RAINBOW 1
#define LIGHT_MODE_COLOR_WAVE 2
#define LIGHT_MODE_BREATHING 3
#define LIGHT_MODE_SOLID 4

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

// New global variables for enhanced features
extern int centerShift;
extern int trailLength;
extern bool directionLightEnabled;
extern bool backgroundMode;
extern int lightMode;

// EEPROM memory layout - explicitly define segments to avoid conflicts
// System settings section (0-99)
#define EEPROM_SYSTEM_START    0
#define EEPROM_ADDR_MARKER     (EEPROM_SYSTEM_START + 0)
#define EEPROM_ADDR_MIN_DIST_L (EEPROM_SYSTEM_START + 1)
#define EEPROM_ADDR_MIN_DIST_H (EEPROM_SYSTEM_START + 2)
#define EEPROM_ADDR_MAX_DIST_L (EEPROM_SYSTEM_START + 3)
#define EEPROM_ADDR_MAX_DIST_H (EEPROM_SYSTEM_START + 4)
#define EEPROM_ADDR_BRIGHTNESS (EEPROM_SYSTEM_START + 5)
#define EEPROM_ADDR_LIGHT_SPAN (EEPROM_SYSTEM_START + 6)
#define EEPROM_ADDR_RED        (EEPROM_SYSTEM_START + 7)
#define EEPROM_ADDR_GREEN      (EEPROM_SYSTEM_START + 8)
#define EEPROM_ADDR_BLUE       (EEPROM_SYSTEM_START + 9)
#define EEPROM_ADDR_NUM_LEDS_L (EEPROM_SYSTEM_START + 10)
#define EEPROM_ADDR_NUM_LEDS_H (EEPROM_SYSTEM_START + 11)
#define EEPROM_ADDR_CRC        (EEPROM_SYSTEM_START + 12)

// Advanced features section (20-49)
#define EEPROM_ADVANCED_START     20
#define EEPROM_ADDR_CENTER_SHIFT_L (EEPROM_ADVANCED_START + 0)
#define EEPROM_ADDR_CENTER_SHIFT_H (EEPROM_ADVANCED_START + 1)
#define EEPROM_ADDR_TRAIL_LENGTH   (EEPROM_ADVANCED_START + 2)
#define EEPROM_ADDR_DIRECTION_LIGHT (EEPROM_ADVANCED_START + 3)
#define EEPROM_ADDR_BACKGROUND_MODE (EEPROM_ADVANCED_START + 4)
#define EEPROM_ADDR_LIGHT_MODE      (EEPROM_ADVANCED_START + 5)

// WiFi credentials section (100-299)
#define EEPROM_WIFI_START        100
#define EEPROM_WIFI_SSID_ADDR    (EEPROM_WIFI_START)
#define EEPROM_WIFI_PASS_ADDR    (EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH)
#define EEPROM_DEVICE_NAME_ADDR  (EEPROM_WIFI_PASS_ADDR + MAX_PASSWORD_LENGTH)
#define EEPROM_WIFI_MODE_ADDR    (EEPROM_DEVICE_NAME_ADDR + MAX_DEVICE_NAME_LENGTH)

#endif // CONFIG_H                                 