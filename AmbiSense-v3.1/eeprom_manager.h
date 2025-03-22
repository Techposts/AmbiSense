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
 * If EEPROM has not been initialized, loads default settings
 */
void loadSettings();

#endif // EEPROM_MANAGER_H