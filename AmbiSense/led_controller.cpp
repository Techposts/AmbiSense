#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "led_controller.h"

// Initialize LED strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(DEFAULT_NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setupLEDs() {
  strip.begin();
  strip.setBrightness(brightness);
  strip.show();
}

void updateLEDConfig() {
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