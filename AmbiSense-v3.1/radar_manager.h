#ifndef RADAR_MANAGER_H
#define RADAR_MANAGER_H

/**
 * Initialize the LD2410 radar module
 */
void setupRadar();

/**
 * Process the radar readings and update the LED strip
 * @return true if a valid reading was obtained, false otherwise
 */
bool processRadarReading();

#endif // RADAR_MANAGER_H