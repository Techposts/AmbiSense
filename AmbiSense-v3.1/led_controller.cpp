#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "led_controller.h"

// Forward declarations
void updateStandardMode(int startLed);
void updateRainbowMode();
void updateColorWaveMode();
void updateBreathingMode();
void updateSolidMode();

// Initialize LED strip - make it global and accessible from other modules
Adafruit_NeoPixel strip = Adafruit_NeoPixel(DEFAULT_NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Variables for tracking light effects and animations
unsigned long lastUpdateTime = 0;
int effectStep = 0;
int lastDirection = 0;
int lastPosition = 0;

void setupLEDs() {
  Serial.println("Initializing LED strip...");
  strip.begin();
  strip.setBrightness(brightness);
  strip.show(); // Initialize all pixels to 'off'
  Serial.println("LED strip initialized with " + String(numLeds) + " LEDs and brightness " + String(brightness));
}

void updateLEDConfig() {
  Serial.println("Updating LED configuration: numLeds=" + String(numLeds) + ", brightness=" + String(brightness));
  strip.updateLength(numLeds);
  strip.setBrightness(brightness);
  strip.show();
}

// Helper function to create a color with a specific intensity
uint32_t dimColor(uint8_t r, uint8_t g, uint8_t b, float intensity) {
  return strip.Color(
    (uint8_t)(r * intensity),
    (uint8_t)(g * intensity),
    (uint8_t)(b * intensity)
  );
}

// Rainbow color wheel function
uint32_t wheelColor(byte wheelPos) {
  wheelPos = 255 - wheelPos;
  if (wheelPos < 85) {
    return strip.Color(255 - wheelPos * 3, 0, wheelPos * 3);
  }
  if (wheelPos < 170) {
    wheelPos -= 85;
    return strip.Color(0, wheelPos * 3, 255 - wheelPos * 3);
  }
  wheelPos -= 170;
  return strip.Color(wheelPos * 3, 255 - wheelPos * 3, 0);
}

// Update LEDs based on distance reading and current mode
void updateLEDs(int distance) {
  // Update direction based on distance change
  int direction = 0;
  if (distance < lastPosition) {
    direction = -1; // Moving closer
  } else if (distance > lastPosition) {
    direction = 1;  // Moving away
  }
  
  // Update last position if there's a significant change
  if (abs(distance - lastPosition) > 5) {
    lastPosition = distance;
    lastDirection = direction;
  }
  
  // Calculate the LED start position based on current distance
  int startLed = map(distance, minDistance, maxDistance, 0, numLeds - movingLightSpan);
  startLed = constrain(startLed, 0, numLeds - movingLightSpan);
  
  // Apply center shift adjustment
  startLed += centerShift;
  
  // Handle different light modes
  switch (lightMode) {
    case LIGHT_MODE_STANDARD:
      updateStandardMode(startLed);
      break;
    
    case LIGHT_MODE_RAINBOW:
      updateRainbowMode();
      break;
    
    case LIGHT_MODE_COLOR_WAVE:
      updateColorWaveMode();
      break;
    
    case LIGHT_MODE_BREATHING:
      updateBreathingMode();
      break;
    
    case LIGHT_MODE_SOLID:
      updateSolidMode();
      break;
    
    default:
      updateStandardMode(startLed);
      break;
  }
  
  // Update effect step for animations
  effectStep = (effectStep + 1) % 256;
}

// Update LEDs in standard mode (based on distance)
void updateStandardMode(int startLed) {
  strip.clear();
  
  // If background mode is enabled, set all LEDs to a dim background color
  if (backgroundMode) {
    float backgroundIntensity = 0.05; // Low background light
    for (int i = 0; i < numLeds; i++) {
      strip.setPixelColor(i, dimColor(redValue, greenValue, blueValue, backgroundIntensity));
    }
  }
  
  // Set moving light
  for (int i = startLed; i < startLed + movingLightSpan; i++) {
    if (i >= 0 && i < numLeds) {
      strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));
    }
  }
  
  // Add directional trailing effect if enabled
  if (directionLightEnabled && trailLength > 0 && lastDirection != 0) {
    int trailStartPos = (lastDirection > 0) ? startLed - trailLength : startLed + movingLightSpan;
    int trailEndPos = (lastDirection > 0) ? startLed : startLed + movingLightSpan + trailLength;
    
    for (int i = trailStartPos; i < trailEndPos; i++) {
      if (i >= 0 && i < numLeds) {
        // Calculate fade based on position in trail
        float fadePercent = (float)abs(i - (lastDirection > 0 ? startLed : startLed + movingLightSpan)) / trailLength;
        fadePercent = constrain(fadePercent, 0.0, 1.0);
        fadePercent = 1.0 - fadePercent; // Invert so it fades away from main light
        
        strip.setPixelColor(i, dimColor(redValue, greenValue, blueValue, fadePercent * 0.8));
      }
    }
  }
  
  strip.show();
}

// Update LEDs in rainbow mode
void updateRainbowMode() {
  for (int i = 0; i < numLeds; i++) {
    int colorIndex = (i + effectStep) % 256;
    strip.setPixelColor(i, wheelColor(colorIndex));
  }
  strip.show();
}

// Update LEDs in color wave mode
void updateColorWaveMode() {
  for (int i = 0; i < numLeds; i++) {
    // Create sine wave pattern
    float wave = sin((i + effectStep) * 0.1) * 0.5 + 0.5;
    strip.setPixelColor(i, dimColor(redValue, greenValue, blueValue, wave));
  }
  strip.show();
}

// Update LEDs in breathing mode
void updateBreathingMode() {
  // Calculate breathing effect using sine wave
  float breath = sin(effectStep * 0.05) * 0.5 + 0.5;
  
  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, dimColor(redValue, greenValue, blueValue, breath));
  }
  strip.show();
}

// Update LEDs in solid color mode
void updateSolidMode() {
  for (int i = 0; i < numLeds; i++) {
    strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));
  }
  strip.show();
}