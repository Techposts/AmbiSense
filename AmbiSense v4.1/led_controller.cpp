#include <Adafruit_NeoPixel.h>
#include "config.h"
#include "led_controller.h"

// Forward declarations
void updateStandardMode(int startLed);
void updateRainbowMode();
void updateColorWaveMode();
void updateBreathingMode();
void updateSolidMode();
void updateCometMode(int startLed);
void updatePulseMode(int startLed);
void updateFireMode();
void updateTheaterChaseMode();
void updateDualScanMode(int startLed);
void updateMotionParticlesMode(int startLed);

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

// Helper function: Subtract with 8-bit saturation
uint8_t qsub8(uint8_t a, uint8_t b) {
  int result = (int)a - (int)b;
  if (result < 0) result = 0;
  return (uint8_t)result;
}

// Helper function: Add with 8-bit saturation
uint8_t qadd8(uint8_t a, uint8_t b) {
  int result = (int)a + (int)b;
  if (result > 255) result = 255;
  return (uint8_t)result;
}

// Helper function: Generate random 8-bit number
uint8_t random8() {
  return random(256);
}

// Helper function: Generate random number in range
uint8_t random8(uint8_t lim) {
  return random(lim);
}

// Helper function: Generate random number in range
uint8_t random8(uint8_t min, uint8_t lim) {
  return random(min, lim);
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
      
    case LIGHT_MODE_COMET:
      updateCometMode(startLed);
      break;
      
    case LIGHT_MODE_PULSE:
      updatePulseMode(startLed);
      break;
      
    case LIGHT_MODE_FIRE:
      updateFireMode();
      break;
      
    case LIGHT_MODE_THEATER_CHASE:
      updateTheaterChaseMode();
      break;
      
    case LIGHT_MODE_DUAL_SCAN:
      updateDualScanMode(startLed);
      break;
      
    case LIGHT_MODE_MOTION_PARTICLES:
      updateMotionParticlesMode(startLed);
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
  int animationSpeed = map(effectSpeed, 1, 100, 1, 10);
  for (int i = 0; i < numLeds; i++) {
    int colorIndex = (i + (effectStep * animationSpeed)) % 256;
    strip.setPixelColor(i, wheelColor(colorIndex));
  }
  strip.show();
}

// Update LEDs in color wave mode
void updateColorWaveMode() {
  // Map effectSpeed (1-100) to a reasonable animation speed value (1-10)
  int animationSpeed = map(effectSpeed, 1, 100, 1, 10);
  
  for (int i = 0; i < numLeds; i++) {
    // Create sine wave pattern for intensity
    float wave = sin((i + (effectStep * animationSpeed)) * 0.1) * 0.5 + 0.5;
    
    // Create color wave
    int colorIndex = (i * 3 + (effectStep * animationSpeed)) % 256;
    uint32_t baseColor = wheelColor(colorIndex);
    
    // Extract RGB components
    uint8_t r = (baseColor >> 16) & 0xFF;
    uint8_t g = (baseColor >> 8) & 0xFF;
    uint8_t b = baseColor & 0xFF;
    
    // Apply wave intensity modulated by effectIntensity
    float intensity = wave * (effectIntensity / 100.0);
    strip.setPixelColor(i, dimColor(r, g, b, intensity));
  }
  strip.show();
}

// Update LEDs in breathing mode
void updateBreathingMode() {
  // Map effectSpeed (1-100) to breathing speed (1-10)
  int breathSpeed = map(effectSpeed, 1, 100, 1, 10);
  
  // Map effectIntensity (1-100) to intensity multiplier (0.1-1.0)
  float intensityMultiplier = map(effectIntensity, 1, 100, 10, 100) / 100.0;
  
  // Calculate breathing effect using sine wave
  float breath = sin(effectStep * 0.05 * breathSpeed) * 0.5 + 0.5;
  breath *= intensityMultiplier; // Apply intensity multiplier
  
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

// Comet effect: trailing gradient that follows motion
void updateCometMode(int startLed) {
  // Map effectSpeed (1-100) to a reasonable tail length (10-50)
  int tailLength = map(effectSpeed, 1, 100, 10, 50);
  
  // Map effectIntensity (1-100) to a fade factor (0.75-0.98)
  float fadeFactor = map(effectIntensity, 1, 100, 75, 98) / 100.0;
  
  // First, dim all LEDs slightly (creates the tail fade effect)
  for (int i = 0; i < numLeds; i++) {
    uint32_t color = strip.getPixelColor(i);
    
    // Extract RGB components
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    
    // Apply fade factor
    r = r * fadeFactor;
    g = g * fadeFactor;
    b = b * fadeFactor;
    
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  
  // Add the new "head" of the comet
  if (startLed >= 0 && startLed < numLeds) {
    strip.setPixelColor(startLed, strip.Color(redValue, greenValue, blueValue));
  }
  
  strip.show();
}

// Pulse effect: pulses emanate from the motion point
void updatePulseMode(int startLed) {
  strip.clear();
  
  // Map effectSpeed (1-100) to pulse speed (1-10)
  int pulseSpeed = map(effectSpeed, 1, 100, 1, 10);
  
  // Map effectIntensity (1-100) to maximum pulse radius (10-50)
  int maxRadius = map(effectIntensity, 1, 100, 10, 50);
  
  // Create multiple pulses
  for (int pulse = 0; pulse < 3; pulse++) {
    // Calculate pulse radius based on effectStep
    int radius = (effectStep * pulseSpeed + (pulse * 85)) % (maxRadius * 2);
    if (radius > maxRadius) radius = (maxRadius * 2) - radius; // Reflect back
    
    // Draw the pulse
    for (int i = startLed - radius; i <= startLed + radius; i++) {
      if (i >= 0 && i < numLeds) {
        // Fade intensity based on distance from center
        float distFactor = 1.0 - abs(i - startLed) / (float)radius;
        distFactor = distFactor * distFactor; // Square for nicer falloff
        
        uint8_t r = redValue * distFactor;
        uint8_t g = greenValue * distFactor;
        uint8_t b = blueValue * distFactor;
        
        strip.setPixelColor(i, strip.Color(r, g, b));
      }
    }
  }
  
  strip.show();
}

// Fire effect: simulates flickering flames
void updateFireMode() {
  // Map effectSpeed (1-100) to fire speed (1-20)
  int fireSpeed = map(effectSpeed, 1, 100, 1, 20);
  
  // Map effectIntensity (1-100) to cooling rate (20-100)
  int cooling = map(effectIntensity, 1, 100, 20, 100);
  
  // Array to hold heat values
  static uint8_t heat[300]; // Assuming max LEDs
  if (numLeds > 300) return; // Safety check
  
  // Step 1: Cool down every cell a little
  for (int i = 0; i < numLeds; i++) {
    heat[i] = qsub8(heat[i], random(0, ((cooling * 10) / numLeds) + 2));
  }
  
  // Step 2: Heat rises - transfer heat from each cell to the one above
  for (int k = numLeds - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
  
  // Step 3: Randomly ignite new sparks near the bottom
  if (random8() < fireSpeed) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  
  // Step 4: Convert heat to LED colors
  for (int j = 0; j < numLeds; j++) {
    // Map heat level to color palette
    uint8_t t = heat[j];
    uint32_t color;
    
    if (t > 240) { // White (hottest)
      color = strip.Color(255, 255, 255);
    } else if (t > 210) { // Yellow
      color = strip.Color(255, 255, t - 210);
    } else if (t > 140) { // Orange
      color = strip.Color(255, map(t, 140, 210, 0, 255), 0);
    } else { // Red (coolest fire)
      color = strip.Color(map(t, 0, 140, 0, 255), 0, 0);
    }
    
    strip.setPixelColor(j, color);
  }
  
  strip.show();
}

// Theater chase effect: alternating on/off lights that move
void updateTheaterChaseMode() {
  // Map effectSpeed (1-100) to chase speed (1-10)
  int chaseSpeed = map(effectSpeed, 1, 100, 1, 10);
  
  // Map effectIntensity (1-100) to gap size (1-5)
  int gapSize = map(effectIntensity, 1, 100, 1, 5);
  
  strip.clear();
  
  // Calculate chase position based on effectStep
  int pos = (effectStep * chaseSpeed) % (gapSize * 2);
  
  // Set every nth LED based on the current chase position
  for (int i = 0; i < numLeds; i++) {
    if ((i + pos) % (gapSize * 2) < gapSize) {
      strip.setPixelColor(i, strip.Color(redValue, greenValue, blueValue));
    }
  }
  
  strip.show();
}

// Dual scan effect: two scanning lights that move in opposite directions
void updateDualScanMode(int startLed) {
  // Map effectSpeed (1-100) to scan speed (1-10)
  int scanSpeed = map(effectSpeed, 1, 100, 1, 10);
  
  // Map effectIntensity (1-100) to scan width (1-20)
  int scanWidth = map(effectIntensity, 1, 100, 1, 20);
  
  strip.clear();
  
  // Calculate scan position based on effectStep
  int pos1 = (effectStep * scanSpeed) % numLeds;
  int pos2 = numLeds - 1 - pos1; // Opposite direction
  
  // Create two moving scan beams
  for (int i = 0; i < scanWidth; i++) {
    int p1 = pos1 + i;
    int p2 = pos2 - i;
    
    if (p1 >= 0 && p1 < numLeds) {
      float fade = 1.0 - (float)i / scanWidth;
      strip.setPixelColor(p1, dimColor(redValue, greenValue, blueValue, fade));
    }
    
    if (p2 >= 0 && p2 < numLeds) {
      float fade = 1.0 - (float)i / scanWidth;
      strip.setPixelColor(p2, dimColor(redValue, greenValue, blueValue, fade));
    }
  }
  
  // Add brighter point at the tracked motion position
  if (startLed >= 0 && startLed < numLeds) {
    strip.setPixelColor(startLed, strip.Color(redValue, greenValue, blueValue));
  }
  
  strip.show();
}

// Motion particles effect: particles that spawn from the motion point
void updateMotionParticlesMode(int startLed) {
  // Define particle structure
  struct Particle {
    float position;
    float velocity;
    float brightness;
    bool active;
  };
  
  // Static array to hold particles
  static Particle particles[20]; // Up to 20 particles
  static bool initialized = false;
  
  // Initialize particles first time
  if (!initialized) {
    for (int i = 0; i < 20; i++) {
      particles[i].active = false;
    }
    initialized = true;
  }
  
  // Map effectSpeed (1-100) to particle speed (0.5-3.0)
  float particleMaxSpeed = map(effectSpeed, 1, 100, 50, 300) / 100.0;
  
  // Map effectIntensity (1-100) to spawn rate (1-10)
  int spawnRate = map(effectIntensity, 1, 100, 1, 10);
  
  // Spawn new particles from the motion point
  for (int i = 0; i < spawnRate; i++) {
    if (random(100) < 30) { // 30% chance to spawn a new particle
      // Find an inactive particle slot
      for (int j = 0; j < 20; j++) {
        if (!particles[j].active) {
          particles[j].position = startLed;
          particles[j].velocity = (random(100) / 100.0 * 2.0 - 1.0) * particleMaxSpeed; // Random direction
          particles[j].brightness = 1.0;
          particles[j].active = true;
          break;
        }
      }
    }
  }
  
  // Clear the strip
  strip.clear();
  
  // Update and draw particles
  for (int i = 0; i < 20; i++) {
    if (particles[i].active) {
      // Update position
      particles[i].position += particles[i].velocity;
      particles[i].brightness -= 0.02; // Fade out
      
      // Deactivate if off strip or too dim
      if (particles[i].position < 0 || particles[i].position >= numLeds || particles[i].brightness <= 0) {
        particles[i].active = false;
        continue;
      }
      
      // Draw particle
      int pos = (int)particles[i].position;
      if (pos >= 0 && pos < numLeds) {
        strip.setPixelColor(pos, dimColor(redValue, greenValue, blueValue, particles[i].brightness));
      }
    }
  }
  
  strip.show();
}