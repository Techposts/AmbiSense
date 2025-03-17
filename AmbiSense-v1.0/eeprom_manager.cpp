#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"

// 📌 Global Variables (defined in main file, declared in config.h)
int minDistance, maxDistance, brightness, movingLightSpan, numLeds;
int redValue, greenValue, blueValue;
int currentDistance = 0;

// EEPROM address mapping
#define EEPROM_ADDR_MARKER      0  // Initialization marker
#define EEPROM_ADDR_MIN_DIST_L  1  // minDistance low byte
#define EEPROM_ADDR_MIN_DIST_H  2  // minDistance high byte
#define EEPROM_ADDR_MAX_DIST_L  3  // maxDistance low byte
#define EEPROM_ADDR_MAX_DIST_H  4  // maxDistance high byte
#define EEPROM_ADDR_BRIGHTNESS  5  // brightness
#define EEPROM_ADDR_LIGHT_SPAN  6  // movingLightSpan
#define EEPROM_ADDR_RED         7  // redValue
#define EEPROM_ADDR_GREEN       8  // greenValue
#define EEPROM_ADDR_BLUE        9  // blueValue
#define EEPROM_ADDR_NUM_LEDS_L  10 // numLeds low byte
#define EEPROM_ADDR_NUM_LEDS_H  11 // numLeds high byte
#define EEPROM_ADDR_CRC         12 // CRC checksum

void setupEEPROM() {
  Serial.println("Initializing EEPROM...");
  
  // Initialize EEPROM with specified size
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("Failed to initialize EEPROM!");
    delay(1000);
  }
  
  loadSettings();
}

// Simple CRC calculation
byte calculateCRC() {
  byte crc = 0;
  
  // XOR all setting values to form a simple checksum
  crc ^= minDistance & 0xFF;
  crc ^= (minDistance >> 8) & 0xFF;
  crc ^= maxDistance & 0xFF;
  crc ^= (maxDistance >> 8) & 0xFF;
  crc ^= brightness;
  crc ^= movingLightSpan;
  crc ^= redValue;
  crc ^= greenValue;
  crc ^= blueValue;
  crc ^= numLeds & 0xFF;
  crc ^= (numLeds >> 8) & 0xFF;
  
  return crc;
}

void saveSettings() {
  // First byte as an initialization marker
  EEPROM.write(EEPROM_ADDR_MARKER, EEPROM_INITIALIZED_MARKER);
  
  // Store actual values
  EEPROM.write(EEPROM_ADDR_MIN_DIST_L, minDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MIN_DIST_H, (minDistance >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_MAX_DIST_L, maxDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAX_DIST_H, (maxDistance >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_BRIGHTNESS, brightness);
  EEPROM.write(EEPROM_ADDR_LIGHT_SPAN, movingLightSpan);
  
  EEPROM.write(EEPROM_ADDR_RED, redValue);
  EEPROM.write(EEPROM_ADDR_GREEN, greenValue);
  EEPROM.write(EEPROM_ADDR_BLUE, blueValue);
  
  // For values that might be larger than 255, store them across multiple bytes
  EEPROM.write(EEPROM_ADDR_NUM_LEDS_L, numLeds & 0xFF);
  EEPROM.write(EEPROM_ADDR_NUM_LEDS_H, (numLeds >> 8) & 0xFF);
  
  // Calculate and store CRC
  byte crc = calculateCRC();
  EEPROM.write(EEPROM_ADDR_CRC, crc);
  
  // Commit the data to flash
  // THIS IS CRITICAL FOR ESP32 - without this, data isn't actually saved to flash
  if (EEPROM.commit()) {
    Serial.println("EEPROM committed successfully");
  } else {
    Serial.println("ERROR: EEPROM commit failed");
  }
  
  Serial.println("Settings saved to EEPROM:");
  Serial.println("Min Distance: " + String(minDistance));
  Serial.println("Max Distance: " + String(maxDistance));
  Serial.println("Brightness: " + String(brightness));
  Serial.println("Moving Light Span: " + String(movingLightSpan));
  Serial.println("RGB Color: " + String(redValue) + "," + String(greenValue) + "," + String(blueValue));
  Serial.println("Num LEDs: " + String(numLeds));
  Serial.println("CRC: " + String(crc));
}

void loadSettings() {
  // Check if EEPROM has been initialized with our marker
  byte initialized = EEPROM.read(EEPROM_ADDR_MARKER);
  
  if (initialized == EEPROM_INITIALIZED_MARKER) {
    // EEPROM has been initialized, load values
    
    // Read minDistance (using 2 bytes for values that can exceed 255)
    minDistance = EEPROM.read(EEPROM_ADDR_MIN_DIST_L) | (EEPROM.read(EEPROM_ADDR_MIN_DIST_H) << 8);
    
    // Read maxDistance (using 2 bytes)
    maxDistance = EEPROM.read(EEPROM_ADDR_MAX_DIST_L) | (EEPROM.read(EEPROM_ADDR_MAX_DIST_H) << 8);
    
    brightness = EEPROM.read(EEPROM_ADDR_BRIGHTNESS);
    movingLightSpan = EEPROM.read(EEPROM_ADDR_LIGHT_SPAN);
    
    redValue = EEPROM.read(EEPROM_ADDR_RED);
    greenValue = EEPROM.read(EEPROM_ADDR_GREEN);
    blueValue = EEPROM.read(EEPROM_ADDR_BLUE);
    
    // Read numLeds from two bytes
    numLeds = EEPROM.read(EEPROM_ADDR_NUM_LEDS_L) | (EEPROM.read(EEPROM_ADDR_NUM_LEDS_H) << 8);
    
    // Read stored CRC
    byte storedCRC = EEPROM.read(EEPROM_ADDR_CRC);
    
    // Calculate CRC based on loaded values
    byte calculatedCRC = calculateCRC();
    
    // Verify CRC
    if (storedCRC == calculatedCRC) {
      Serial.println("Settings loaded from EEPROM (CRC valid):");
    } else {
      Serial.println("WARNING: CRC mismatch, possible EEPROM corruption!");
      Serial.println("Stored CRC: " + String(storedCRC) + ", Calculated: " + String(calculatedCRC));
      // Continue using the loaded values, but warn the user
    }
    
    Serial.println("Min Distance: " + String(minDistance));
    Serial.println("Max Distance: " + String(maxDistance));
    Serial.println("Brightness: " + String(brightness));
    Serial.println("Moving Light Span: " + String(movingLightSpan));
    Serial.println("RGB Color: " + String(redValue) + "," + String(greenValue) + "," + String(blueValue));
    Serial.println("Num LEDs: " + String(numLeds));
  } else {
    // EEPROM hasn't been initialized, set defaults
    minDistance = DEFAULT_MIN_DISTANCE;
    maxDistance = DEFAULT_MAX_DISTANCE;
    brightness = DEFAULT_BRIGHTNESS;
    movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
    redValue = DEFAULT_RED;
    greenValue = DEFAULT_GREEN;
    blueValue = DEFAULT_BLUE;
    numLeds = DEFAULT_NUM_LEDS;
    
    Serial.println("Using default settings (EEPROM not initialized or corrupted)");
    Serial.println("EEPROM marker: " + String(initialized) + " (expected: " + String(EEPROM_INITIALIZED_MARKER) + ")");
    
    // Save defaults to EEPROM for future use
    saveSettings();
  }
  
  // Validate values to prevent issues
  if (minDistance < 0 || minDistance > 500) minDistance = DEFAULT_MIN_DISTANCE;
  if (maxDistance < 50 || maxDistance > 1000) maxDistance = DEFAULT_MAX_DISTANCE;
  if (brightness < 0 || brightness > 255) brightness = DEFAULT_BRIGHTNESS;
  if (movingLightSpan < 1 || movingLightSpan > 300) movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
  if (numLeds < 1 || numLeds > 2000) numLeds = DEFAULT_NUM_LEDS;
  
  // DO NOT call updateLEDConfig() here as LEDs may not be initialized yet
  // That's handled in the main setup() function sequencing
}