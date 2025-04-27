#include <EEPROM.h>
#include <Arduino.h>
#include "config.h"
#include "eeprom_manager.h"
#include "led_controller.h"

// 📌 Global Variables (defined in main file, declared in config.h)
int minDistance, maxDistance, brightness, movingLightSpan, numLeds;
int redValue, greenValue, blueValue;
int currentDistance = 0;

// New global variables for enhanced features
int centerShift;
int trailLength;
bool directionLightEnabled;
bool backgroundMode;
int lightMode;
int effectSpeed = DEFAULT_EFFECT_SPEED;
int effectIntensity = DEFAULT_EFFECT_INTENSITY;

// Motion smoothing settings
bool motionSmoothingEnabled = DEFAULT_MOTION_SMOOTHING_ENABLED;
float positionSmoothingFactor = DEFAULT_POSITION_SMOOTHING_FACTOR;
float velocitySmoothingFactor = DEFAULT_VELOCITY_SMOOTHING_FACTOR;
float predictionFactor = DEFAULT_PREDICTION_FACTOR;
float positionPGain = DEFAULT_POSITION_P_GAIN;
float positionIGain = DEFAULT_POSITION_I_GAIN;

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

// New EEPROM addresses for enhanced features
#define EEPROM_ADDR_CENTER_SHIFT_L 13 // centerShift low byte
#define EEPROM_ADDR_CENTER_SHIFT_H 14 // centerShift high byte
#define EEPROM_ADDR_TRAIL_LENGTH   15 // trailLength
#define EEPROM_ADDR_DIRECTION_LIGHT 16 // directionLightEnabled
#define EEPROM_ADDR_BACKGROUND_MODE 17 // backgroundMode
#define EEPROM_ADDR_LIGHT_MODE      18 // lightMode

void setupEEPROM() {
  Serial.println("\nInitializing EEPROM...");
  Serial.println("EEPROM Size: " + String(EEPROM_SIZE) + " bytes");
  
  // Initialize EEPROM with specified size
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("ERROR: Failed to initialize EEPROM!");
    delay(1000);
    return;
  }
  
  // Check if EEPROM is already initialized
  byte marker = EEPROM.read(EEPROM_ADDR_MARKER);
  Serial.println("Current EEPROM marker: " + String(marker) + " (Expected: " + String(EEPROM_INITIALIZED_MARKER) + ")");
  
  // Load settings (this will set defaults if not initialized)
  loadSettings();
  
  Serial.println("EEPROM initialization complete");
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
  
  // Add new settings to CRC
  crc ^= centerShift & 0xFF;
  crc ^= (centerShift >> 8) & 0xFF;
  crc ^= trailLength;
  crc ^= directionLightEnabled ? 1 : 0;
  crc ^= backgroundMode ? 1 : 0;
  crc ^= lightMode;
  
  // Add motion smoothing to CRC
  crc ^= motionSmoothingEnabled ? 1 : 0;
  crc ^= (int)(positionSmoothingFactor * 100) & 0xFF;
  crc ^= (int)(velocitySmoothingFactor * 100) & 0xFF;
  crc ^= (int)(predictionFactor * 100) & 0xFF;
  crc ^= (int)(positionPGain * 1000) & 0xFF;
  crc ^= (int)(positionIGain * 1000) & 0xFF;
  crc ^= effectSpeed;
  crc ^= effectIntensity;
  
  return crc;
}

void saveSettings() {
  Serial.println("Saving settings to EEPROM...");
  
  // First byte as an initialization marker
  EEPROM.write(EEPROM_ADDR_MARKER, EEPROM_INITIALIZED_MARKER);
  
  // Store actual values - USING THE CORRECT ADDRESSES FROM CONFIG.H
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
  
  // Save advanced features - USE CORRECT ADDRESSES FROM CONFIG.H
  EEPROM.put(EEPROM_ADDR_CENTER_SHIFT_L, (int16_t)centerShift);
  EEPROM.write(EEPROM_ADDR_TRAIL_LENGTH, trailLength);
  EEPROM.write(EEPROM_ADDR_DIRECTION_LIGHT, directionLightEnabled ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_BACKGROUND_MODE, backgroundMode ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_LIGHT_MODE, lightMode);
  EEPROM.write(EEPROM_ADDR_EFFECT_SPEED, effectSpeed);
  EEPROM.write(EEPROM_ADDR_EFFECT_INTENSITY, effectIntensity);
  
  // Save motion smoothing settings
  EEPROM.write(EEPROM_ADDR_MOTION_SMOOTHING, motionSmoothingEnabled ? 1 : 0);
  
  // Store smoothing factors (16-bit values)
  EEPROM.write(EEPROM_ADDR_SMOOTHING_FACTOR_L, (int)(positionSmoothingFactor * 100) & 0xFF);
  EEPROM.write(EEPROM_ADDR_SMOOTHING_FACTOR_H, ((int)(positionSmoothingFactor * 100) >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_VELOCITY_FACTOR_L, (int)(velocitySmoothingFactor * 100) & 0xFF);
  EEPROM.write(EEPROM_ADDR_VELOCITY_FACTOR_H, ((int)(velocitySmoothingFactor * 100) >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_PREDICTION_FACTOR_L, (int)(predictionFactor * 100) & 0xFF);
  EEPROM.write(EEPROM_ADDR_PREDICTION_FACTOR_H, ((int)(predictionFactor * 100) >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_POSITION_P_GAIN_L, (int)(positionPGain * 1000) & 0xFF);
  EEPROM.write(EEPROM_ADDR_POSITION_P_GAIN_H, ((int)(positionPGain * 1000) >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_POSITION_I_GAIN_L, (int)(positionIGain * 1000) & 0xFF);
  EEPROM.write(EEPROM_ADDR_POSITION_I_GAIN_H, ((int)(positionIGain * 1000) >> 8) & 0xFF);
  
  // Calculate and store CRC
  byte crc = calculateCRC();
  EEPROM.write(EEPROM_ADDR_CRC, crc);
  
  // Commit the data to flash
  bool committed = EEPROM.commit();
  if (committed) {
    Serial.println("EEPROM committed successfully");
    
    // Verify key values were written correctly
    byte verifyMarker = EEPROM.read(EEPROM_ADDR_MARKER);
    int verifyMinDist = EEPROM.read(EEPROM_ADDR_MIN_DIST_L) | (EEPROM.read(EEPROM_ADDR_MIN_DIST_H) << 8);
    
    if (verifyMarker == EEPROM_INITIALIZED_MARKER && verifyMinDist == minDistance) {
      Serial.println("EEPROM values verified successfully");
    } else {
      Serial.println("WARNING: EEPROM verification failed!");
      Serial.println("Expected marker: " + String(EEPROM_INITIALIZED_MARKER) + ", Got: " + String(verifyMarker));
      Serial.println("Expected minDistance: " + String(minDistance) + ", Got: " + String(verifyMinDist));
    }
  } else {
    Serial.println("ERROR: EEPROM commit failed!");
  }
  
  // Print all saved values for verification
  Serial.println("\nSettings saved to EEPROM:");
  Serial.println("Min Distance: " + String(minDistance));
  Serial.println("Max Distance: " + String(maxDistance));
  Serial.println("Brightness: " + String(brightness));
  Serial.println("Moving Light Span: " + String(movingLightSpan));
  Serial.println("RGB Color: " + String(redValue) + "," + String(greenValue) + "," + String(blueValue));
  Serial.println("Num LEDs: " + String(numLeds));
  Serial.println("Center Shift: " + String(centerShift));
  Serial.println("Trail Length: " + String(trailLength));
  Serial.println("Direction Light: " + String(directionLightEnabled ? "Enabled" : "Disabled"));
  Serial.println("Background Mode: " + String(backgroundMode ? "Enabled" : "Disabled"));
  Serial.println("Light Mode: " + String(lightMode));
  Serial.println("Motion Smoothing: " + String(motionSmoothingEnabled ? "Enabled" : "Disabled"));
  Serial.println("Position Smoothing Factor: " + String(positionSmoothingFactor));
  Serial.println("Velocity Smoothing Factor: " + String(velocitySmoothingFactor));
  Serial.println("Prediction Factor: " + String(predictionFactor));
  Serial.println("Position P Gain: " + String(positionPGain));
  Serial.println("Position I Gain: " + String(positionIGain));
  Serial.println("Effect Speed: " + String(effectSpeed));
  Serial.println("Effect Intensity: " + String(effectIntensity));
  Serial.println("CRC: " + String(crc));
}

void loadSettings() {
  // Check if EEPROM has been initialized with our marker
  byte initialized = EEPROM.read(EEPROM_ADDR_MARKER);
  Serial.println("EEPROM Marker: " + String(initialized) + " (Expected: " + String(EEPROM_INITIALIZED_MARKER) + ")");
  
  if (initialized == EEPROM_INITIALIZED_MARKER) {
    Serial.println("EEPROM has been initialized, loading values...");
    
    // Read system settings - USING CORRECT ADDRESSES FROM CONFIG.H
    minDistance = EEPROM.read(EEPROM_ADDR_MIN_DIST_L) | (EEPROM.read(EEPROM_ADDR_MIN_DIST_H) << 8);
    maxDistance = EEPROM.read(EEPROM_ADDR_MAX_DIST_L) | (EEPROM.read(EEPROM_ADDR_MAX_DIST_H) << 8);
    brightness = EEPROM.read(EEPROM_ADDR_BRIGHTNESS);
    movingLightSpan = EEPROM.read(EEPROM_ADDR_LIGHT_SPAN);
    redValue = EEPROM.read(EEPROM_ADDR_RED);
    greenValue = EEPROM.read(EEPROM_ADDR_GREEN);
    blueValue = EEPROM.read(EEPROM_ADDR_BLUE);
    numLeds = EEPROM.read(EEPROM_ADDR_NUM_LEDS_L) | (EEPROM.read(EEPROM_ADDR_NUM_LEDS_H) << 8);
    
    // Read advanced features - USE CORRECT ADDRESSES FROM CONFIG.H
    int16_t loadedCenterShift;
      EEPROM.get(EEPROM_ADDR_CENTER_SHIFT_L, loadedCenterShift);
      centerShift = loadedCenterShift;

    trailLength = EEPROM.read(EEPROM_ADDR_TRAIL_LENGTH);
    directionLightEnabled = EEPROM.read(EEPROM_ADDR_DIRECTION_LIGHT) == 1;
    backgroundMode = EEPROM.read(EEPROM_ADDR_BACKGROUND_MODE) == 1;
    lightMode = EEPROM.read(EEPROM_ADDR_LIGHT_MODE);
    effectSpeed = EEPROM.read(EEPROM_ADDR_EFFECT_SPEED);
    effectIntensity = EEPROM.read(EEPROM_ADDR_EFFECT_INTENSITY);
    
    // Read motion smoothing settings
    motionSmoothingEnabled = EEPROM.read(EEPROM_ADDR_MOTION_SMOOTHING) == 1;
    
    // Read smoothing factors (16-bit encoded)
    int positionSmoothingRaw = EEPROM.read(EEPROM_ADDR_SMOOTHING_FACTOR_L) | 
                              (EEPROM.read(EEPROM_ADDR_SMOOTHING_FACTOR_H) << 8);
    positionSmoothingFactor = positionSmoothingRaw / 100.0;
    
    int velocitySmoothingRaw = EEPROM.read(EEPROM_ADDR_VELOCITY_FACTOR_L) | 
                              (EEPROM.read(EEPROM_ADDR_VELOCITY_FACTOR_H) << 8);
    velocitySmoothingFactor = velocitySmoothingRaw / 100.0;
    
    int predictionRaw = EEPROM.read(EEPROM_ADDR_PREDICTION_FACTOR_L) | 
                        (EEPROM.read(EEPROM_ADDR_PREDICTION_FACTOR_H) << 8);
    predictionFactor = predictionRaw / 100.0;
    
    int pGainRaw = EEPROM.read(EEPROM_ADDR_POSITION_P_GAIN_L) | 
                  (EEPROM.read(EEPROM_ADDR_POSITION_P_GAIN_H) << 8);
    positionPGain = pGainRaw / 1000.0;
    
    int iGainRaw = EEPROM.read(EEPROM_ADDR_POSITION_I_GAIN_L) | 
                  (EEPROM.read(EEPROM_ADDR_POSITION_I_GAIN_H) << 8);
    positionIGain = iGainRaw / 1000.0;
    
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
    }
    
    // Print loaded values
    Serial.println("Min Distance: " + String(minDistance));
    Serial.println("Max Distance: " + String(maxDistance));
    Serial.println("Brightness: " + String(brightness));
    Serial.println("Moving Light Span: " + String(movingLightSpan));
    Serial.println("RGB Color: " + String(redValue) + "," + String(greenValue) + "," + String(blueValue));
    Serial.println("Num LEDs: " + String(numLeds));
    Serial.println("Center Shift: " + String(centerShift));
    Serial.println("Trail Length: " + String(trailLength));
    Serial.println("Direction Light: " + String(directionLightEnabled ? "Enabled" : "Disabled"));
    Serial.println("Background Mode: " + String(backgroundMode ? "Enabled" : "Disabled"));
    Serial.println("Light Mode: " + String(lightMode));
    Serial.println("Motion Smoothing: " + String(motionSmoothingEnabled ? "Enabled" : "Disabled"));
    Serial.println("Position Smoothing Factor: " + String(positionSmoothingFactor));
    Serial.println("Velocity Smoothing Factor: " + String(velocitySmoothingFactor));
    Serial.println("Prediction Factor: " + String(predictionFactor));
    Serial.println("Position P Gain: " + String(positionPGain));
    Serial.println("Position I Gain: " + String(positionIGain));
    Serial.println("Effect Speed: " + String(effectSpeed));
    Serial.println("Effect Intensity: " + String(effectIntensity));
  } else {
    Serial.println("EEPROM not initialized or corrupted. Setting defaults...");
    // Set default values
    minDistance = DEFAULT_MIN_DISTANCE;
    maxDistance = DEFAULT_MAX_DISTANCE;
    brightness = DEFAULT_BRIGHTNESS;
    movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
    redValue = DEFAULT_RED;
    greenValue = DEFAULT_GREEN;
    blueValue = DEFAULT_BLUE;
    numLeds = DEFAULT_NUM_LEDS;
    centerShift = DEFAULT_CENTER_SHIFT;
    trailLength = DEFAULT_TRAIL_LENGTH;
    directionLightEnabled = DEFAULT_DIRECTION_LIGHT;
    backgroundMode = DEFAULT_BACKGROUND_MODE;
    lightMode = DEFAULT_LIGHT_MODE;
    motionSmoothingEnabled = DEFAULT_MOTION_SMOOTHING_ENABLED;
    positionSmoothingFactor = DEFAULT_POSITION_SMOOTHING_FACTOR;
    velocitySmoothingFactor = DEFAULT_VELOCITY_SMOOTHING_FACTOR;
    predictionFactor = DEFAULT_PREDICTION_FACTOR;
    positionPGain = DEFAULT_POSITION_P_GAIN;
    positionIGain = DEFAULT_POSITION_I_GAIN;
    effectSpeed = DEFAULT_EFFECT_SPEED;
    effectIntensity = DEFAULT_EFFECT_INTENSITY;
    
    Serial.println("Using default settings (EEPROM not initialized or corrupted)");
    Serial.println("EEPROM marker: " + String(initialized) + " (expected: " + String(EEPROM_INITIALIZED_MARKER) + ")");
    
    // Save defaults to EEPROM for future use
    saveSettings();
  }
  
  // Validate values to prevent issues
  if (minDistance < 0 || minDistance > 500) {
    Serial.println("Invalid minDistance, using default");
    minDistance = DEFAULT_MIN_DISTANCE;
  }
  if (maxDistance < 50 || maxDistance > 1000) {
    Serial.println("Invalid maxDistance, using default");
    maxDistance = DEFAULT_MAX_DISTANCE;
  }
  if (brightness < 0 || brightness > 255) {
    Serial.println("Invalid brightness, using default");
    brightness = DEFAULT_BRIGHTNESS;
  }
  if (movingLightSpan < 1 || movingLightSpan > 300) {
    Serial.println("Invalid movingLightSpan, using default");
    movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
  }
  if (numLeds < 1 || numLeds > 2000) {
    Serial.println("Invalid numLeds, using default");
    numLeds = DEFAULT_NUM_LEDS;
  }
  if (centerShift < -100 || centerShift > 100) {
    Serial.println("Invalid centerShift, using default");
    centerShift = DEFAULT_CENTER_SHIFT;
  }
  if (trailLength < 0 || trailLength > 100) {
    Serial.println("Invalid trailLength, using default");
    trailLength = DEFAULT_TRAIL_LENGTH;
  }
  if (lightMode < 0 || lightMode > 10) {
    Serial.println("Invalid lightMode, using default");
    lightMode = DEFAULT_LIGHT_MODE;
  }
  if (effectSpeed < 1 || effectSpeed > 100) {
    Serial.println("Invalid effectSpeed, using default");
    effectSpeed = DEFAULT_EFFECT_SPEED;
  }
  if (effectIntensity < 1 || effectIntensity > 100) {
    Serial.println("Invalid effectIntensity, using default");
    effectIntensity = DEFAULT_EFFECT_INTENSITY;
  }
}