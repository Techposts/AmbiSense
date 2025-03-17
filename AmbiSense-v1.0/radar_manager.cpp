#include <ld2410.h>
#include "config.h"
#include "radar_manager.h"
#include "led_controller.h"

ld2410 radar;

void setupRadar() {
  Serial.println("Initializing radar sensor...");
  RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
  radar.begin(RADAR_SERIAL);
  Serial.println("Radar sensor initialized");
}

bool processRadarReading() {
  radar.read();

  if (radar.isConnected()) {
    currentDistance = radar.movingTargetDistance();
    if (currentDistance == 0) currentDistance = radar.stationaryTargetDistance();

    if (currentDistance < minDistance) currentDistance = minDistance;
    
    // Calculate the LED start position based on current distance
    int start_led = map(currentDistance, minDistance, maxDistance, 0, numLeds - movingLightSpan);
    start_led = constrain(start_led, 0, numLeds - movingLightSpan);

    // Update the LEDs
    updateLEDs(start_led);
    return true;
  }
  
  return false;
}