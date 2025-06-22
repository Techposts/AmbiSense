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

/**
 * Get the current smoothed distance reading
 * @return Current smoothed distance in centimeters
 */
float getSmoothedDistance();

/**
 * Get the current estimated velocity
 * @return Current estimated velocity in cm/s
 */
float getEstimatedVelocity();

/**
 * Get the predicted distance based on motion smoothing
 * @return Predicted distance in centimeters
 */
float getPredictedDistance();

#endif // RADAR_MANAGER_H