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

// LED Distribution settings - these are declared extern since they're defined in AmbiSense.ino
// Remove the definitions here to avoid multiple definition errors

// Magic marker for EEPROM validation
#define EEPROM_MAGIC_MARKER 0xA55A
#define EEPROM_DATA_VERSION 1

// EEPROM Header structure for validation
struct EEPROMHeader {
  uint16_t magicMarker;     // Fixed value (0xA55A) to identify valid data
  uint8_t  dataVersion;     // Increment when data structure changes
  uint8_t  systemSettings;  // CRC for system settings section
  uint8_t  advancedSettings;// CRC for advanced settings section
  uint8_t  motionSettings;  // CRC for motion settings section
  uint8_t  espnowSettings;  // CRC for ESP-NOW settings section
  uint8_t  ledDistSettings; // CRC for LED distribution settings section
  uint8_t  reserved[1];     // Reserved for future use
};

// Additional validation function for critical settings
void validateCriticalSettings() {
  bool settingsChanged = false;
  
  // Check for invalid min/max distance values (most critical for UI)
  if (minDistance < 0 || minDistance > 500 || maxDistance < 50 || maxDistance > 1000 || minDistance >= maxDistance) {
    Serial.println("CRITICAL: Invalid min/max distance values detected, restoring defaults");
    minDistance = DEFAULT_MIN_DISTANCE;
    maxDistance = DEFAULT_MAX_DISTANCE;
    settingsChanged = true;
  }
  
  // Check for invalid LED count
  if (numLeds <= 0 || numLeds > 2000) {
    Serial.println("CRITICAL: Invalid LED count detected, restoring default");
    numLeds = DEFAULT_NUM_LEDS;
    settingsChanged = true;
  }
  
  // Save if changes were made
  if (settingsChanged) {
    // Only update the affected fields
    EEPROM.write(EEPROM_ADDR_MIN_DIST_L, minDistance & 0xFF);
    EEPROM.write(EEPROM_ADDR_MIN_DIST_H, (minDistance >> 8) & 0xFF);
    EEPROM.write(EEPROM_ADDR_MAX_DIST_L, maxDistance & 0xFF);
    EEPROM.write(EEPROM_ADDR_MAX_DIST_H, (maxDistance >> 8) & 0xFF);
    EEPROM.write(EEPROM_ADDR_NUM_LEDS_L, numLeds & 0xFF);
    EEPROM.write(EEPROM_ADDR_NUM_LEDS_H, (numLeds >> 8) & 0xFF);
    
    // Recalculate and update CRC
    EEPROM.write(EEPROM_ADDR_CRC, calculateSystemCRC());
    
    // Commit changes
    EEPROM.commit();
    
    // Log the corrected values
    Serial.printf("Corrected settings - Min: %d, Max: %d, LEDs: %d\n", 
                 minDistance, maxDistance, numLeds);
  }
}

void setupEEPROM() {
  Serial.println("Initializing EEPROM...");
  
  // Initialize EEPROM with specified size
  if (!EEPROM.begin(EEPROM_SIZE)) {
    Serial.println("ERROR: Failed to initialize EEPROM!");
    delay(1000);
    return;
  }
  
  // Read header
  EEPROMHeader header;
  EEPROM.get(0, header);
  
  // Check if header is valid
  if (header.magicMarker != EEPROM_MAGIC_MARKER) {
    Serial.println("First time initialization or corrupted EEPROM. Setting up with defaults.");
    
    // Initialize header with magic marker
    header.magicMarker = EEPROM_MAGIC_MARKER;
    header.dataVersion = EEPROM_DATA_VERSION;
    
    // Set defaults for all settings
    resetSystemSettings();
    resetAdvancedSettings();
    resetMotionSettings();
    resetEspnowSettings();
    resetLEDDistributionSettings();
    
    // Save all settings and update CRCs
    saveSettings();
    
    // Write the header
    EEPROM.put(0, header);
    EEPROM.commit();
    
    Serial.println("EEPROM initialized with defaults");
    return;
  }
  
  // Check individual section CRCs
  bool systemValid = (header.systemSettings == calculateSystemCRC());
  bool advancedValid = (header.advancedSettings == calculateAdvancedCRC());
  bool motionValid = (header.motionSettings == calculateMotionCRC());
  bool espnowValid = (header.espnowSettings == calculateEspnowCRC());
  bool ledDistValid = (header.ledDistSettings == calculateLEDDistributionCRC());
  
  if (!systemValid || !advancedValid || !motionValid || !espnowValid || !ledDistValid) {
    Serial.println("CRC mismatch detected in one or more sections");
    Serial.printf("System: %d, Advanced: %d, Motion: %d, ESPNOW: %d, LEDDist: %d\n", 
                 systemValid, advancedValid, motionValid, espnowValid, ledDistValid);
  }
  
  // Load settings with corruption detection per section
  loadSettings(systemValid, advancedValid, motionValid, espnowValid);
  
  // Load LED distribution settings separately
  if (ledDistValid) {
    loadLEDDistributionSettings();
  } else {
    Serial.println("WARNING: LED distribution settings corrupted! Using defaults.");
    resetLEDDistributionSettings();
  }
  
  // If any section was corrupted, save the restored values
  if (!systemValid || !advancedValid || !motionValid || !espnowValid || !ledDistValid) {
    Serial.println("Saving restored values for corrupted sections");
    saveSettings();
  }
  
  // Perform final validation of key values
  validateAllSettings();
  
  Serial.println("EEPROM initialization complete");
}

// MAINTAIN ORIGINAL FUNCTION SIGNATURES FOR WEB INTERFACE COMPATIBILITY
void loadSettings() {
  // Load all sections with full validation
  loadSettings(true, true, true, true);
  loadLEDDistributionSettings();
}

void loadSettings(bool systemValid, bool advancedValid, bool motionValid, bool espnowValid) {
  // Load system settings
  if (systemValid) {
    // Read system settings
    minDistance = EEPROM.read(EEPROM_ADDR_MIN_DIST_L) | (EEPROM.read(EEPROM_ADDR_MIN_DIST_H) << 8);
    maxDistance = EEPROM.read(EEPROM_ADDR_MAX_DIST_L) | (EEPROM.read(EEPROM_ADDR_MAX_DIST_H) << 8);
    brightness = EEPROM.read(EEPROM_ADDR_BRIGHTNESS);
    movingLightSpan = EEPROM.read(EEPROM_ADDR_LIGHT_SPAN);
    redValue = EEPROM.read(EEPROM_ADDR_RED);
    greenValue = EEPROM.read(EEPROM_ADDR_GREEN);
    blueValue = EEPROM.read(EEPROM_ADDR_BLUE);
    numLeds = EEPROM.read(EEPROM_ADDR_NUM_LEDS_L) | (EEPROM.read(EEPROM_ADDR_NUM_LEDS_H) << 8);
    
    // Debug log actual bytes for troubleshooting
    if (ENABLE_DEBUG_LOGGING) {
      Serial.println("EEPROM Raw Bytes:");
      Serial.printf("MIN_DIST: L=0x%02X, H=0x%02X => %d\n", 
                  EEPROM.read(EEPROM_ADDR_MIN_DIST_L), 
                  EEPROM.read(EEPROM_ADDR_MIN_DIST_H),
                  minDistance);
      Serial.printf("MAX_DIST: L=0x%02X, H=0x%02X => %d\n", 
                  EEPROM.read(EEPROM_ADDR_MAX_DIST_L), 
                  EEPROM.read(EEPROM_ADDR_MAX_DIST_H),
                  maxDistance);
    }
  } else {
    Serial.println("WARNING: System settings corrupted! Using defaults.");
    resetSystemSettings();
  }
  
  // Load advanced settings
  if (advancedValid) {
    // Read advanced features
    int16_t loadedCenterShift;
    EEPROM.get(EEPROM_ADDR_CENTER_SHIFT_L, loadedCenterShift);
    centerShift = loadedCenterShift;
    
    trailLength = EEPROM.read(EEPROM_ADDR_TRAIL_LENGTH);
    directionLightEnabled = EEPROM.read(EEPROM_ADDR_DIRECTION_LIGHT) == 1;
    backgroundMode = EEPROM.read(EEPROM_ADDR_BACKGROUND_MODE) == 1;
    lightMode = EEPROM.read(EEPROM_ADDR_LIGHT_MODE);
    effectSpeed = EEPROM.read(EEPROM_ADDR_EFFECT_SPEED);
    effectIntensity = EEPROM.read(EEPROM_ADDR_EFFECT_INTENSITY);
  } else {
    Serial.println("WARNING: Advanced settings corrupted! Using defaults.");
    resetAdvancedSettings();
  }
  
  // Load motion settings
  if (motionValid) {
    // Read motion smoothing settings
    motionSmoothingEnabled = EEPROM.read(EEPROM_ADDR_MOTION_SMOOTHING) == 1;
    
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
  } else {
    Serial.println("WARNING: Motion settings corrupted! Using defaults.");
    resetMotionSettings();
  }
  
  // Load ESP-NOW settings
  if (espnowValid) {
    // Read ESP-NOW settings
    deviceRole = EEPROM.read(EEPROM_ADDR_DEVICE_ROLE);
    if (deviceRole != DEVICE_ROLE_MASTER && deviceRole != DEVICE_ROLE_SLAVE) {
      deviceRole = DEFAULT_DEVICE_ROLE;
    }
    
    // Load sensor priority mode
    sensorPriorityMode = EEPROM.read(EEPROM_ADDR_SENSOR_PRIORITY_MODE);
    if (sensorPriorityMode > SENSOR_PRIORITY_ZONE_BASED) {
      sensorPriorityMode = DEFAULT_SENSOR_PRIORITY_MODE;
    }
    
    // Master address
    for (int i = 0; i < 6; i++) {
      masterAddress[i] = EEPROM.read(EEPROM_ADDR_MASTER_MAC + i);
    }
    
    // Paired slaves
    numSlaveDevices = EEPROM.read(EEPROM_ADDR_PAIRED_SLAVES);
    numSlaveDevices = constrain(numSlaveDevices, 0, MAX_SLAVE_DEVICES);
    
    for (int s = 0; s < numSlaveDevices; s++) {
      for (int i = 0; i < 6; i++) {
        slaveAddresses[s][i] = EEPROM.read(EEPROM_ADDR_PAIRED_SLAVES + 1 + (s * 6) + i);
      }
    }
  } else {
    Serial.println("WARNING: ESP-NOW settings corrupted! Using defaults.");
    resetEspnowSettings();
  }
}

// MOVED OUTSIDE: All the functions that were incorrectly nested inside loadSettings()
void loadLEDDistributionSettings() {
  ledSegmentMode = EEPROM.read(EEPROM_ADDR_LED_SEGMENT_MODE);
  if (ledSegmentMode != LED_SEGMENT_MODE_CONTINUOUS && 
      ledSegmentMode != LED_SEGMENT_MODE_DISTRIBUTED) {
    ledSegmentMode = LED_SEGMENT_MODE_CONTINUOUS;
  }
  
  ledSegmentStart = EEPROM.read(EEPROM_ADDR_LED_SEGMENT_START_L) | 
                    (EEPROM.read(EEPROM_ADDR_LED_SEGMENT_START_H) << 8);
  
  ledSegmentLength = EEPROM.read(EEPROM_ADDR_LED_SEGMENT_LENGTH_L) | 
                    (EEPROM.read(EEPROM_ADDR_LED_SEGMENT_LENGTH_H) << 8);
  
  totalSystemLeds = EEPROM.read(EEPROM_ADDR_TOTAL_SYSTEM_LEDS_L) | 
                    (EEPROM.read(EEPROM_ADDR_TOTAL_SYSTEM_LEDS_H) << 8);
  
  // Validate loaded values
  validateLEDDistributionSettings();
  
  Serial.printf("Loaded LED distribution - Mode: %d, Start: %d, Length: %d, Total: %d\n", 
                ledSegmentMode, ledSegmentStart, ledSegmentLength, totalSystemLeds);
}

void validateLEDDistributionSettings() {
  if (ledSegmentStart < 0 || ledSegmentStart > MAX_SUPPORTED_LEDS) {
    ledSegmentStart = 0;
  }
  
  if (ledSegmentLength < 1 || ledSegmentLength > MAX_SUPPORTED_LEDS) {
    ledSegmentLength = numLeds;
  }
  
  if (totalSystemLeds < 1 || totalSystemLeds > MAX_SUPPORTED_LEDS) {
    totalSystemLeds = numLeds;
  }
}

void validateAllSettings() {
  // Validate system settings
  if (minDistance < 0 || minDistance > 500) minDistance = DEFAULT_MIN_DISTANCE;
  if (maxDistance < 50 || maxDistance > 1000) maxDistance = DEFAULT_MAX_DISTANCE;
  
  // Ensure min is always less than max with a reasonable gap
  if (minDistance >= maxDistance) {
    // Fix by resetting both to defaults
    minDistance = DEFAULT_MIN_DISTANCE;
    maxDistance = DEFAULT_MAX_DISTANCE;
  }
  
  if (brightness < 0 || brightness > 255) brightness = DEFAULT_BRIGHTNESS;
  if (movingLightSpan < 1 || movingLightSpan > 300) movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
  if (numLeds < 1 || numLeds > 2000) numLeds = DEFAULT_NUM_LEDS;
  
  // Validate advanced settings
  if (centerShift < -100 || centerShift > 100) centerShift = DEFAULT_CENTER_SHIFT;
  if (trailLength < 0 || trailLength > 100) trailLength = DEFAULT_TRAIL_LENGTH;
  if (lightMode < 0 || lightMode > 10) lightMode = DEFAULT_LIGHT_MODE;
  if (effectSpeed < 1 || effectSpeed > 100) effectSpeed = DEFAULT_EFFECT_SPEED;
  if (effectIntensity < 1 || effectIntensity > 100) effectIntensity = DEFAULT_EFFECT_INTENSITY;
  
  // Validate motion parameters
  positionSmoothingFactor = constrain(positionSmoothingFactor, 0.0, 1.0);
  velocitySmoothingFactor = constrain(velocitySmoothingFactor, 0.0, 1.0);
  predictionFactor = constrain(predictionFactor, 0.0, 1.0);
  positionPGain = constrain(positionPGain, 0.0, 1.0);
  positionIGain = constrain(positionIGain, 0.0, 0.1);
  
  // Validate LED distribution settings
  validateLEDDistributionSettings();
  
  // Log important values after validation
  Serial.printf("Validated Min/Max: %d/%d\n", minDistance, maxDistance);
}

void saveSettings() {
  Serial.println("Saving settings to EEPROM...");
  
  // Validate critical settings before saving
  validateAllSettings();
  
  // Save all sections with explicit commit after each section
  saveSystemSettings();
  saveAdvancedSettings();
  saveMotionSettings();
  saveEspnowSettings();
  saveLEDDistributionSettings();
  
  // Calculate CRCs
  uint8_t systemCRC = calculateSystemCRC();
  uint8_t advancedCRC = calculateAdvancedCRC();
  uint8_t motionCRC = calculateMotionCRC();
  uint8_t espnowCRC = calculateEspnowCRC();
  uint8_t ledDistCRC = calculateLEDDistributionCRC();
  
  // Update header with magic marker and CRCs
  EEPROMHeader header;
  header.magicMarker = EEPROM_MAGIC_MARKER;
  header.dataVersion = EEPROM_DATA_VERSION;
  header.systemSettings = systemCRC;
  header.advancedSettings = advancedCRC;
  header.motionSettings = motionCRC;
  header.espnowSettings = espnowCRC;
  header.ledDistSettings = ledDistCRC;
  header.reserved[0] = 0;
  
  // Write header to EEPROM
  EEPROM.put(0, header);
  
  // Explicitly commit the data to flash and verify success
  bool committed = EEPROM.commit();
  if (committed) {
    Serial.println("EEPROM committed successfully");
    
    // Additional verification for debug
    if (ENABLE_DEBUG_LOGGING) {
      int storedMinDist = EEPROM.read(EEPROM_ADDR_MIN_DIST_L) | (EEPROM.read(EEPROM_ADDR_MIN_DIST_H) << 8);
      int storedMaxDist = EEPROM.read(EEPROM_ADDR_MAX_DIST_L) | (EEPROM.read(EEPROM_ADDR_MAX_DIST_H) << 8);
      Serial.printf("Verification - Min: %d (exp:%d), Max: %d (exp:%d)\n", 
                 storedMinDist, minDistance, storedMaxDist, maxDistance);
    }
  } else {
    // Try again with a delay if the first commit fails
    Serial.println("ERROR: EEPROM commit failed! Retrying...");
    delay(100);
    if (EEPROM.commit()) {
      Serial.println("EEPROM commit succeeded on retry");
    } else {
      Serial.println("ERROR: EEPROM commit failed on retry!");
    }
  }
}

void saveSystemSettings() {
  // Store system values
  EEPROM.write(EEPROM_ADDR_MARKER, EEPROM_INITIALIZED_MARKER);
  
  // Ensure min is always less than max with clear debugging
  if (minDistance >= maxDistance) {
    Serial.printf("WARNING: Invalid min/max detected before save: %d/%d\n", minDistance, maxDistance);
    minDistance = DEFAULT_MIN_DISTANCE;
    maxDistance = DEFAULT_MAX_DISTANCE;
  }
  
  // Save min/max distance with explicit byte ordering
  EEPROM.write(EEPROM_ADDR_MIN_DIST_L, minDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MIN_DIST_H, (minDistance >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_MAX_DIST_L, maxDistance & 0xFF);
  EEPROM.write(EEPROM_ADDR_MAX_DIST_H, (maxDistance >> 8) & 0xFF);
  
  EEPROM.write(EEPROM_ADDR_BRIGHTNESS, brightness);
  EEPROM.write(EEPROM_ADDR_LIGHT_SPAN, movingLightSpan);
  
  EEPROM.write(EEPROM_ADDR_RED, redValue);
  EEPROM.write(EEPROM_ADDR_GREEN, greenValue);
  EEPROM.write(EEPROM_ADDR_BLUE, blueValue);
  
  EEPROM.write(EEPROM_ADDR_NUM_LEDS_L, numLeds & 0xFF);
  EEPROM.write(EEPROM_ADDR_NUM_LEDS_H, (numLeds >> 8) & 0xFF);
  
  // Add CRC for integrity check
  EEPROM.write(EEPROM_ADDR_CRC, calculateSystemCRC());
  
  // Ensure this section is committed
  EEPROM.commit();
  
  // Debug log for troubleshooting
  if (ENABLE_DEBUG_LOGGING) {
    Serial.printf("Saving Min: %d (0x%02X,0x%02X), Max: %d (0x%02X,0x%02X)\n", 
                minDistance, 
                minDistance & 0xFF, (minDistance >> 8) & 0xFF,
                maxDistance,
                maxDistance & 0xFF, (maxDistance >> 8) & 0xFF);
  }
}

void saveAdvancedSettings() {
  // Save advanced features
  EEPROM.put(EEPROM_ADDR_CENTER_SHIFT_L, (int16_t)centerShift);
  EEPROM.write(EEPROM_ADDR_TRAIL_LENGTH, trailLength);
  EEPROM.write(EEPROM_ADDR_DIRECTION_LIGHT, directionLightEnabled ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_BACKGROUND_MODE, backgroundMode ? 1 : 0);
  EEPROM.write(EEPROM_ADDR_LIGHT_MODE, lightMode);
  EEPROM.write(EEPROM_ADDR_EFFECT_SPEED, effectSpeed);
  EEPROM.write(EEPROM_ADDR_EFFECT_INTENSITY, effectIntensity);
  
  // Add explicit commit
  EEPROM.commit();
}

void saveMotionSettings() {
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
  
  // Add explicit commit
  EEPROM.commit();
}

void saveEspnowSettings() {
  // Save ESP-NOW settings
  EEPROM.write(EEPROM_ADDR_DEVICE_ROLE, deviceRole);
  
  // Save sensor priority mode
  EEPROM.write(EEPROM_ADDR_SENSOR_PRIORITY_MODE, sensorPriorityMode);
  
  // Save master MAC address
  for (int i = 0; i < 6; i++) {
    EEPROM.write(EEPROM_ADDR_MASTER_MAC + i, masterAddress[i]);
  }
  
  // Save paired slaves
  EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES, numSlaveDevices);
  for (int s = 0; s < numSlaveDevices; s++) {
    for (int i = 0; i < 6; i++) {
      EEPROM.write(EEPROM_ADDR_PAIRED_SLAVES + 1 + (s * 6) + i, slaveAddresses[s][i]);
    }
  }
  
  // Add explicit commit
  EEPROM.commit();
}

void saveLEDDistributionSettings() {
  // Use additional EEPROM addresses for LED distribution settings
  EEPROM.write(EEPROM_ADDR_LED_SEGMENT_MODE, ledSegmentMode);
  EEPROM.write(EEPROM_ADDR_LED_SEGMENT_START_L, ledSegmentStart & 0xFF);
  EEPROM.write(EEPROM_ADDR_LED_SEGMENT_START_H, (ledSegmentStart >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_LED_SEGMENT_LENGTH_L, ledSegmentLength & 0xFF);
  EEPROM.write(EEPROM_ADDR_LED_SEGMENT_LENGTH_H, (ledSegmentLength >> 8) & 0xFF);
  EEPROM.write(EEPROM_ADDR_TOTAL_SYSTEM_LEDS_L, totalSystemLeds & 0xFF);
  EEPROM.write(EEPROM_ADDR_TOTAL_SYSTEM_LEDS_H, (totalSystemLeds >> 8) & 0xFF);
  
  EEPROM.commit();
}

void resetAllSettings() {
  // Reset all settings to defaults
  resetSystemSettings();
  resetAdvancedSettings();
  resetMotionSettings();
  resetEspnowSettings();
  resetLEDDistributionSettings();
  
  // Save the defaults to EEPROM
  saveSettings();
}

void resetSystemSettings() {
  minDistance = DEFAULT_MIN_DISTANCE;
  maxDistance = DEFAULT_MAX_DISTANCE;
  brightness = DEFAULT_BRIGHTNESS;
  movingLightSpan = DEFAULT_MOVING_LIGHT_SPAN;
  redValue = DEFAULT_RED;
  greenValue = DEFAULT_GREEN;
  blueValue = DEFAULT_BLUE;
  numLeds = DEFAULT_NUM_LEDS;
}

void resetAdvancedSettings() {
  centerShift = DEFAULT_CENTER_SHIFT;
  trailLength = DEFAULT_TRAIL_LENGTH;
  directionLightEnabled = DEFAULT_DIRECTION_LIGHT;
  backgroundMode = DEFAULT_BACKGROUND_MODE;
  lightMode = DEFAULT_LIGHT_MODE;
  effectSpeed = DEFAULT_EFFECT_SPEED;
  effectIntensity = DEFAULT_EFFECT_INTENSITY;
}

void resetMotionSettings() {
  motionSmoothingEnabled = DEFAULT_MOTION_SMOOTHING_ENABLED;
  positionSmoothingFactor = DEFAULT_POSITION_SMOOTHING_FACTOR;
  velocitySmoothingFactor = DEFAULT_VELOCITY_SMOOTHING_FACTOR;
  predictionFactor = DEFAULT_PREDICTION_FACTOR;
  positionPGain = DEFAULT_POSITION_P_GAIN;
  positionIGain = DEFAULT_POSITION_I_GAIN;
}

void resetEspnowSettings() {
  deviceRole = DEFAULT_DEVICE_ROLE;
  sensorPriorityMode = DEFAULT_SENSOR_PRIORITY_MODE;
  
  // Clear master address
  for (int i = 0; i < 6; i++) {
    masterAddress[i] = 0;
  }
  
  // Clear slave addresses
  numSlaveDevices = 0;
  for (int s = 0; s < MAX_SLAVE_DEVICES; s++) {
    for (int i = 0; i < 6; i++) {
      slaveAddresses[s][i] = 0;
    }
  }
}

void resetLEDDistributionSettings() {
  ledSegmentMode = DEFAULT_LED_SEGMENT_MODE;
  ledSegmentStart = DEFAULT_LED_SEGMENT_START;
  ledSegmentLength = DEFAULT_LED_SEGMENT_LENGTH;
  totalSystemLeds = DEFAULT_TOTAL_SYSTEM_LEDS;
}

// CRC calculation functions
uint8_t calculateSystemCRC() {
  uint8_t crc = 0;
  
  for (int i = EEPROM_SYSTEM_START; i < EEPROM_ADVANCED_START; i++) {
    if (i != EEPROM_ADDR_MARKER) { // Skip marker address for CRC calculation
      crc ^= EEPROM.read(i);
    }
  }
  
  return crc;
}

uint8_t calculateAdvancedCRC() {
  uint8_t crc = 0;
  
  for (int i = EEPROM_ADVANCED_START; i < EEPROM_MOTION_START; i++) {
    crc ^= EEPROM.read(i);
  }
  
  return crc;
}

uint8_t calculateMotionCRC() {
  uint8_t crc = 0;
  
  for (int i = EEPROM_MOTION_START; i < EEPROM_ESPNOW_START; i++) {
    crc ^= EEPROM.read(i);
  }
  
  return crc;
}

uint8_t calculateEspnowCRC() {
  uint8_t crc = 0;
  
  for (int i = EEPROM_ESPNOW_START; i < EEPROM_WIFI_START; i++) {
    crc ^= EEPROM.read(i);
  }
  
  return crc;
}

uint8_t calculateLEDDistributionCRC() {
  uint8_t crc = 0;
  
  for (int i = EEPROM_LED_DIST_START; i < EEPROM_LED_DIST_START + 7; i++) {
    crc ^= EEPROM.read(i);
  }
  
  return crc;
}