#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "led_controller.h"

// Initialize LED strip - make it global and accessible from other modules
Adafruit_NeoPixel strip = Adafruit_NeoPixel(DEFAULT_NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

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

void updateLEDs(int start_led) {
  strip.clear();
  for (int i = start_led; i < start_led + movingLightSpan; i++) {
    if (i < numLeds) { // Safety check to prevent writing beyond LED strip length
      strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));
    }
  }
  strip.show();
}