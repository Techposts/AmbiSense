#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

/**
 * Initialize the LED strip
 */
void setupLEDs();

/**
 * Update LED strip configuration (number of LEDs, brightness)
 * Called after settings are loaded or changed
 */
void updateLEDConfig();

/**
 * Update LEDs based on distance reading
 * @param start_led The index of the first LED to light
 */
void updateLEDs(int start_led);

#endif // LED_CONTROLLER_H