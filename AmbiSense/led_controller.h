#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Adafruit_NeoPixel.h>
#include "config.h"

// Forward declaration for ESP-NOW LED segment data structure
struct led_segment_data_t;

// Maximum supported LEDs (can be increased based on available memory)
#define MAX_SUPPORTED_LEDS 2000

// Make the LED strip available to other modules
extern Adafruit_NeoPixel strip;
extern int numLeds;

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
 * Validate LED count against memory and hardware limits
 * @param requestedLeds Number of LEDs to validate
 * @return true if valid, false otherwise
 */
bool validateLEDCount(int requestedLeds);

/**
 * Force LED strip reinitialization with new LED count
 * @param newLedCount New number of LEDs
 */
void reinitializeLEDStrip(int newLedCount);

/**
 * Get current configured LED count
 * @return Currently configured LED count
 */
int getCurrentLEDCount();

/**
 * Test all LEDs with color sweep
 */
void testAllLEDs();

/**
 * Update LEDs based on distance reading
 * @param distance The current distance reading from the radar sensor
 */
void updateLEDs(int distance);

/**
 * Update LEDs in standard mode (based on distance)
 * @param startLed The starting LED position based on distance
 */
void updateStandardMode(int startLed);

/**
 * Update LEDs in rainbow mode
 */
void updateRainbowMode();

/**
 * Update LEDs in color wave mode
 */
void updateColorWaveMode();

/**
 * Update LEDs in breathing mode
 */
void updateBreathingMode();

/**
 * Update LEDs in solid color mode
 */
void updateSolidMode();

/**
 * Update LEDs in comet trail mode
 * @param startLed The starting LED position based on distance
 */
void updateCometMode(int startLed);

/**
 * Update LEDs in pulse waves mode
 * @param startLed The starting LED position based on distance
 */
void updatePulseMode(int startLed);

/**
 * Update LEDs in fire effect mode
 */
void updateFireMode();

/**
 * Update LEDs in theater chase mode
 */
void updateTheaterChaseMode();

/**
 * Update LEDs in dual scanning mode
 * @param startLed The starting LED position based on distance
 */
void updateDualScanMode(int startLed);

/**
 * Update LEDs in motion particles mode
 * @param startLed The starting LED position based on distance
 */
void updateMotionParticlesMode(int startLed);

/**
 * Process LED segment data (ESP-NOW distributed mode)
 * @param segmentData LED segment data structure
 */
void processLEDSegmentData(led_segment_data_t segmentData);

/**
 * Update LED segment for distributed mode
 * @param globalStartPos Global LED start position  
 * @param segmentData LED segment data
 */
void updateLEDSegment(int globalStartPos, led_segment_data_t segmentData);

/**
 * Helper function to create a color with a specific intensity
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param intensity Brightness factor (0.0-1.0)
 * @return 32-bit color value
 */
uint32_t dimColor(uint8_t r, uint8_t g, uint8_t b, float intensity);

/**
 * Generate a color from the rainbow wheel
 * @param wheelPos Position on the wheel (0-255)
 * @return 32-bit color value
 */
uint32_t wheelColor(byte wheelPos);

/**
 * Saturated subtraction to prevent underflow
 * @param a First value
 * @param b Value to subtract
 * @return Saturated result
 */
uint8_t qsub8(uint8_t a, uint8_t b);

/**
 * Saturated addition to prevent overflow
 * @param a First value
 * @param b Value to add
 * @return Saturated result
 */
uint8_t qadd8(uint8_t a, uint8_t b);

/**
 * Generate random 8-bit number
 * @return Random value 0-255
 */
uint8_t random8();

/**
 * Generate random number in range
 * @param lim Upper limit (exclusive)
 * @return Random value 0 to lim-1
 */
uint8_t random8(uint8_t lim);

/**
 * Generate random number in range
 * @param min Lower limit (inclusive)
 * @param lim Upper limit (exclusive)
 * @return Random value min to lim-1
 */
uint8_t random8(uint8_t min, uint8_t lim);

#endif // LED_CONTROLLER_H