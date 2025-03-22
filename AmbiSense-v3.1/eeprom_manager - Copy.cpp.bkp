#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"

// 📌 Global Variables (defined in main file, declared in config.h)
int minDistance, maxDistance, brightness, movingLightSpan, numLeds;
int redValue, greenValue, blueValue;
int currentDistance = 0;

void setupEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  loadSettings();
}

void saveSettings() {
  // First byte as an initialization marker
  EEPROM.write(0, EEPROM_INITIALIZED_MARKER);
  
  // Store actual values starting from address 1
  EEPROM.write(1, minDistance & 0xFF);
  EEPROM.write(2, (minDistance >> 8) & 0xFF);
  
  EEPROM.write(3, maxDistance & 0xFF);
  EEPROM.write(4, (maxDistance >> 8) & 0xFF);
  
  EEPROM.write(5, brightness);
  EEPROM.write(6, movingLightSpan);
  
  EEPROM.write(7, redValue);
  EEPROM.write(8, greenValue);
  EEPROM.write(9, blueValue);
  
  // For values that might be larger than 255, store them across multiple bytes
  EEPROM.write(10, numLeds & 0xFF);        // Low byte
  EEPROM.write(11, (numLeds >> 8) & 0xFF); // High byte
  
  EEPROM.commit();
  Serial.println("Settings saved to EEPROM:");
  Serial.println("Min Distance: " + String(minDistance));
  Serial.println("Max Distance: " + String(maxDistance));
  Serial.println("Brightness: " + String(brightness));
  Serial.println("Moving Light Span: " + String(movingLightSpan));
  Serial.println("RGB Color: " + String(redValue) + "," + String(greenValue) + "," + String(blueValue));
  Serial.println("Num LEDs: " + String(numLeds));
}

void loadSettings() {
  // Check if EEPROM has been initialized with our marker
  byte initialized = EEPROM.read(0);
  
  if (initialized == EEPROM_INITIALIZED_MARKER) {
    // EEPROM has been initialized, load values
    
    // Read minDistance (using 2 bytes for values that can exceed 255)
    minDistance = EEPROM.read(1) | (EEPROM.read(2) << 8);
    
    // Read maxDistance (using 2 bytes)
    maxDistance = EEPROM.read(3) | (EEPROM.read(4) << 8);
    
    brightness = EEPROM.read(5);
    movingLightSpan = EEPROM.read(6);
    
    redValue = EEPROM.read(7);
    greenValue = EEPROM.read(8);
    blueValue = EEPROM.read(9);
    
    // Read numLeds from two bytes
    numLeds = EEPROM.read(10) | (EEPROM.read(11) << 8);
    
    Serial.println("Settings loaded from EEPROM:");
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
    
    Serial.println("Using default settings (EEPROM not initialized)");
    
    // Save defaults to EEPROM for future use
    saveSettings();
  }
  
  // Validate values to prevent issues
  if (minDistance < 0 || minDistance > 500) minDistance = DEFAULT_MIN_DISTANCE;
  if (maxDistance < 50 || maxDistance > 1000) maxDistance = DEFAULT_MAX_DISTANCE;
  if (brightness < 0 || brightness > 255) brightness = DEFAULT_BRIGHTNESS;
  if (movingLightSpan < 1 || movingLightSpan > 300) movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
  if (numLeds < 1 || numLeds > 2000) numLeds = DEFAULT_NUM_LEDS;
  
  // Update the LED strip based on loaded settings
  updateLEDConfig();
}