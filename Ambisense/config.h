#ifndef CONFIG_H
#define CONFIG_H

/*
 * AmbiSense v4.3.0 - Enhanced Radar-Controlled LED System
 * Created by Ravi Singh (techPosts media)
 * Copyright © 2025 TechPosts Media. All rights reserved.
 */

// Debug logging settings - set to false to reduce serial output
#define ENABLE_DEBUG_LOGGING false
#define ENABLE_MOTION_LOGGING false
#define ENABLE_WIFI_LOGGING true
#define ENABLE_ESPNOW_LOGGING true

// 🛠 LED & Sensor Config
#define LED_PIN 5
#define DEFAULT_NUM_LEDS 300
#define DEFAULT_BRIGHTNESS 255
#define DEFAULT_MOVING_LIGHT_SPAN 40
#define DEFAULT_MIN_DISTANCE 30
#define DEFAULT_MAX_DISTANCE 300
#define EEPROM_INITIALIZED_MARKER 123
#define EEPROM_SIZE 1024

// LED Distribution modes
#define LED_SEGMENT_MODE_CONTINUOUS 0
#define LED_SEGMENT_MODE_DISTRIBUTED 1

// LED limits
#define MAX_SUPPORTED_LEDS 2000

#define ENABLE_MOCK_DEVICES false

// Default color (white)
#define DEFAULT_RED 255
#define DEFAULT_GREEN 255
#define DEFAULT_BLUE 255

// Default settings
#define DEFAULT_CENTER_SHIFT 0
#define DEFAULT_TRAIL_LENGTH 0
#define DEFAULT_DIRECTION_LIGHT false
#define DEFAULT_BACKGROUND_MODE false
#define DEFAULT_LIGHT_MODE 0

// Default LED distribution values
#define DEFAULT_LED_SEGMENT_MODE LED_SEGMENT_MODE_CONTINUOUS
#define DEFAULT_LED_SEGMENT_START 0
#define DEFAULT_LED_SEGMENT_LENGTH 300
#define DEFAULT_TOTAL_SYSTEM_LEDS 300

// SPIFFS configuration
#define FORMAT_SPIFFS_IF_FAILED true

// Light mode constants
#define LIGHT_MODE_STANDARD 0
#define LIGHT_MODE_RAINBOW 1
#define LIGHT_MODE_COLOR_WAVE 2
#define LIGHT_MODE_BREATHING 3
#define LIGHT_MODE_SOLID 4
#define LIGHT_MODE_COMET 5
#define LIGHT_MODE_PULSE 6
#define LIGHT_MODE_FIRE 7
#define LIGHT_MODE_THEATER_CHASE 8
#define LIGHT_MODE_DUAL_SCAN 9
#define LIGHT_MODE_MOTION_PARTICLES 10

#define DEFAULT_MOTION_SMOOTHING_ENABLED true
#define DEFAULT_POSITION_SMOOTHING_FACTOR 0.2
#define DEFAULT_VELOCITY_SMOOTHING_FACTOR 0.1
#define DEFAULT_PREDICTION_FACTOR 0.5
#define DEFAULT_POSITION_P_GAIN 0.1
#define DEFAULT_POSITION_I_GAIN 0.01
#define DEFAULT_EFFECT_SPEED 50
#define DEFAULT_EFFECT_INTENSITY 50

// 🎯 LD2410 Config
#define RADAR_SERIAL Serial1
#define RADAR_RX_PIN 3
#define RADAR_TX_PIN 4

// 📡 Wi-Fi Access Point
#define WIFI_AP_SSID "AmbiSense"
#define WIFI_AP_PASSWORD "12345678"

// 📡 Web Server
#define WEB_SERVER_PORT 80

// ESP-NOW Master-Slave configuration
#define DEVICE_ROLE_MASTER 1
#define DEVICE_ROLE_SLAVE 2
#define MAX_SLAVE_DEVICES 5
#define DEFAULT_DEVICE_ROLE DEVICE_ROLE_MASTER

// ESP-NOW improvements
#define ESPNOW_CHANNEL 1
#define ESPNOW_RETRY_COUNT 3
#define ESPNOW_TIMEOUT_MS 5000
#define AMBISENSE_DEVICE_PREFIX "AmbiSense"
#define CONNECTION_HEALTH_TIMEOUT 10000

// Sensor priority modes
#define SENSOR_PRIORITY_MOST_RECENT 0
#define SENSOR_PRIORITY_SLAVE_FIRST 1  
#define SENSOR_PRIORITY_MASTER_FIRST 2
#define SENSOR_PRIORITY_ZONE_BASED 3

// Default setting
#define DEFAULT_SENSOR_PRIORITY_MODE SENSOR_PRIORITY_ZONE_BASED

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

// Add motion smoothing global variables
extern bool motionSmoothingEnabled;
extern float positionSmoothingFactor;
extern float velocitySmoothingFactor;
extern float predictionFactor;
extern float positionPGain;
extern float positionIGain;
extern int effectSpeed;
extern int effectIntensity;

// LED Distribution globals - declared as extern since they're defined in AmbiSense.ino
extern int ledSegmentMode;
extern int ledSegmentStart;
extern int ledSegmentLength;
extern int totalSystemLeds;

// ESP-NOW global variables
extern uint8_t deviceRole;  // Master or slave role
extern uint8_t masterAddress[6];  // MAC address of master device
extern uint8_t slaveAddresses[MAX_SLAVE_DEVICES][6];  // MAC addresses of slave devices
extern uint8_t numSlaveDevices;  // Number of paired slave devices
extern uint8_t sensorPriorityMode;  // How to prioritize sensors

// EEPROM memory layout - explicitly define segments to avoid conflicts
// System settings section (0-19)
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
#define EEPROM_ADDR_EFFECT_SPEED      (EEPROM_ADVANCED_START + 6)
#define EEPROM_ADDR_EFFECT_INTENSITY  (EEPROM_ADVANCED_START + 7)

// Motion smoothing settings in EEPROM (50-69)
#define EEPROM_MOTION_START      50
#define EEPROM_ADDR_MOTION_SMOOTHING       (EEPROM_MOTION_START + 0)
#define EEPROM_ADDR_SMOOTHING_FACTOR_L     (EEPROM_MOTION_START + 1)
#define EEPROM_ADDR_SMOOTHING_FACTOR_H     (EEPROM_MOTION_START + 2)
#define EEPROM_ADDR_VELOCITY_FACTOR_L      (EEPROM_MOTION_START + 3)
#define EEPROM_ADDR_VELOCITY_FACTOR_H      (EEPROM_MOTION_START + 4)
#define EEPROM_ADDR_PREDICTION_FACTOR_L    (EEPROM_MOTION_START + 5)
#define EEPROM_ADDR_PREDICTION_FACTOR_H    (EEPROM_MOTION_START + 6)
#define EEPROM_ADDR_POSITION_P_GAIN_L      (EEPROM_MOTION_START + 7)
#define EEPROM_ADDR_POSITION_P_GAIN_H      (EEPROM_MOTION_START + 8)
#define EEPROM_ADDR_POSITION_I_GAIN_L      (EEPROM_MOTION_START + 9)
#define EEPROM_ADDR_POSITION_I_GAIN_H      (EEPROM_MOTION_START + 10)

// ESP-NOW settings section (70-99)
#define EEPROM_ESPNOW_START     70
#define EEPROM_ADDR_DEVICE_ROLE    (EEPROM_ESPNOW_START + 0)  // 1 byte
#define EEPROM_ADDR_MASTER_MAC     (EEPROM_ESPNOW_START + 1)  // 6 bytes
#define EEPROM_ADDR_PAIRED_SLAVES  (EEPROM_ESPNOW_START + 7)  // 1 byte for count + 6*MAX_SLAVES bytes
#define EEPROM_ADDR_SENSOR_PRIORITY_MODE (EEPROM_ESPNOW_START + 50)  // 1 byte

// WiFi credentials section (100-299)
#define EEPROM_WIFI_START        100
#define EEPROM_WIFI_MARKER_ADDR  (EEPROM_WIFI_START)      // 2 bytes for marker
#define EEPROM_WIFI_SSID_ADDR    (EEPROM_WIFI_MARKER_ADDR + 2)
#define EEPROM_WIFI_PASS_ADDR    (EEPROM_WIFI_SSID_ADDR + MAX_SSID_LENGTH)
#define EEPROM_DEVICE_NAME_ADDR  (EEPROM_WIFI_PASS_ADDR + MAX_PASSWORD_LENGTH)
#define EEPROM_WIFI_MODE_ADDR    (EEPROM_DEVICE_NAME_ADDR + MAX_DEVICE_NAME_LENGTH)
#define EEPROM_WIFI_USE_STATIC_IP (EEPROM_WIFI_MODE_ADDR + 1)
#define EEPROM_WIFI_STATIC_IP    (EEPROM_WIFI_USE_STATIC_IP + 1)  // 4 bytes

// LED Distribution settings section (300-319)
#define EEPROM_LED_DIST_START         300
#define EEPROM_ADDR_LED_SEGMENT_MODE     (EEPROM_LED_DIST_START + 0)
#define EEPROM_ADDR_LED_SEGMENT_START_L  (EEPROM_LED_DIST_START + 1)
#define EEPROM_ADDR_LED_SEGMENT_START_H  (EEPROM_LED_DIST_START + 2)
#define EEPROM_ADDR_LED_SEGMENT_LENGTH_L (EEPROM_LED_DIST_START + 3)
#define EEPROM_ADDR_LED_SEGMENT_LENGTH_H (EEPROM_LED_DIST_START + 4)
#define EEPROM_ADDR_TOTAL_SYSTEM_LEDS_L  (EEPROM_LED_DIST_START + 5)
#define EEPROM_ADDR_TOTAL_SYSTEM_LEDS_H  (EEPROM_LED_DIST_START + 6)

#endif // CONFIG_H