#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

/**
 * Initializes the EEPROM module
 */
void setupEEPROM();

/**
 * Saves all settings to EEPROM
 */
void saveSettings();

/**
 * Loads all settings from EEPROM
 */
void loadSettings();

/**
 * Loads settings with selective validation
 */
void loadSettings(bool systemValid, bool advancedValid, bool motionValid, bool espnowValid);

/**
 * Validate all settings
 */
void validateAllSettings();

/**
 * Perform validation of critical settings like min/max distance
 * Used to detect and fix corrupted values
 */
void validateCriticalSettings();

/**
 * Save individual section settings
 */
void saveSystemSettings();
void saveAdvancedSettings();
void saveMotionSettings();
void saveEspnowSettings();
void saveLEDDistributionSettings();

/**
 * Reset settings to defaults
 */
void resetAllSettings();
void resetSystemSettings();
void resetAdvancedSettings();
void resetMotionSettings();
void resetEspnowSettings();
void resetLEDDistributionSettings();

/**
 * Calculate CRCs for different sections
 */
uint8_t calculateSystemCRC();
uint8_t calculateAdvancedCRC();
uint8_t calculateMotionCRC();
uint8_t calculateEspnowCRC();
uint8_t calculateLEDDistributionCRC();

/**
 * Load LED distribution settings from EEPROM
 */
void loadLEDDistributionSettings();

/**
 * Validate LED distribution settings
 */
void validateLEDDistributionSettings();

#endif // EEPROM_MANAGER_H