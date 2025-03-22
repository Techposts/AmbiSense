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
    int previousDistance = currentDistance;
    currentDistance = radar.movingTargetDistance();
    
    // If no moving target, check for stationary target
    if (currentDistance == 0) currentDistance = radar.stationaryTargetDistance();

    // Apply constraints
    if (currentDistance < minDistance) currentDistance = minDistance;
    
    // Update the LEDs with the current distance
    updateLEDs(currentDistance);
    
    return true;
  }
  
  return false;
}