#include <ld2410.h>
#include "config.h"
#include "radar_manager.h"
#include "led_controller.h"
#include "espnow_manager.h"

// LD2410 radar sensor instance
ld2410 radar;

// Detailed motion smoothing state
struct MotionState {
    float smoothedDistance = 0.0;
    float smoothedVelocity = 0.0;
    float predictedDistance = 0.0;
    float positionError = 0.0;
    float positionErrorIntegral = 0.0;
    unsigned long lastUpdateTime = 0;
} motionState;

void setupRadar() {
    Serial.println("Initializing radar sensor...");
    RADAR_SERIAL.begin(256000, SERIAL_8N1, RADAR_RX_PIN, RADAR_TX_PIN);
    radar.begin(RADAR_SERIAL);
    
    // Initialize motion state
    motionState.lastUpdateTime = millis();
    motionState.smoothedDistance = (float)minDistance;
    
    if (ENABLE_MOTION_LOGGING) {
        Serial.println("Motion smoothing configuration:");
        Serial.printf("Motion Smoothing: %s\n", motionSmoothingEnabled ? "Enabled" : "Disabled");
        Serial.printf("Position Factor: %.2f\n", positionSmoothingFactor);
        Serial.printf("Velocity Factor: %.2f\n", velocitySmoothingFactor);
        Serial.printf("Prediction Factor: %.2f\n", predictionFactor);
    }
}

bool processRadarReading() {
    radar.read();

    if (radar.isConnected()) {
        int rawDistance = radar.movingTargetDistance();
        
        // If no moving target, check for stationary target
        if (rawDistance == 0) rawDistance = radar.stationaryTargetDistance();
        
        // Ensure rawDistance is in valid range
        if (rawDistance < minDistance) rawDistance = minDistance;
        if (rawDistance > maxDistance) rawDistance = maxDistance;

        // Calculate direction
        int8_t direction = 0;
        static int lastRawDistance = rawDistance;
        if (rawDistance < lastRawDistance - 5) direction = -1; // Moving closer
        else if (rawDistance > lastRawDistance + 5) direction = 1; // Moving away
        lastRawDistance = rawDistance;

        // Motion smoothing logic
        if (motionSmoothingEnabled) {
            // Time since last update (for velocity calculation)
            unsigned long currentTime = millis();
            float deltaTime = (currentTime - motionState.lastUpdateTime) / 1000.0; // Convert to seconds
            motionState.lastUpdateTime = currentTime;
            
            // If this is the first time after enabling, log it
            static bool firstRun = true;
            if (firstRun && ENABLE_MOTION_LOGGING) {
                Serial.printf("[Motion] Initial raw distance: %d cm\n", rawDistance);
                firstRun = false;
            }

            // Prevent divide-by-zero and extreme delta times
            if (deltaTime <= 0.001) deltaTime = 0.001; // Minimum 1ms
            if (deltaTime > 1.0) deltaTime = 1.0;      // Cap at 1 second
            
            // If this is the first reading or smoothedDistance is not initialized
            if (motionState.smoothedDistance <= 0) {
                motionState.smoothedDistance = (float)rawDistance;
                motionState.predictedDistance = (float)rawDistance;
                motionState.smoothedVelocity = 0.0;
                motionState.positionError = 0.0;
                motionState.positionErrorIntegral = 0.0;
            }

            // Low-pass filter for position
            motionState.smoothedDistance = 
                (1.0f - positionSmoothingFactor) * motionState.smoothedDistance + 
                positionSmoothingFactor * (float)rawDistance;

            // Velocity estimation - calculate difference between actual and predicted
            float instantVelocity = (motionState.smoothedDistance - motionState.predictedDistance) / deltaTime;
            
            // Apply constraints to velocity (prevent extreme values)
            instantVelocity = constrain(instantVelocity, -200.0f, 200.0f); // Maximum 2 m/s
            
            // Apply low-pass filter to velocity
            motionState.smoothedVelocity = 
                (1.0f - velocitySmoothingFactor) * motionState.smoothedVelocity + 
                velocitySmoothingFactor * instantVelocity;

            // Position prediction based on velocity
            motionState.predictedDistance = 
                motionState.smoothedDistance + 
                motionState.smoothedVelocity * predictionFactor;

            // Simple PI controller for position
            motionState.positionError = motionState.predictedDistance - motionState.smoothedDistance;
            motionState.positionErrorIntegral += motionState.positionError * deltaTime;

            // Anti-windup for integral term
            motionState.positionErrorIntegral = constrain(
                motionState.positionErrorIntegral, 
                -100.0f, 
                100.0f
            );

            // Calculate control output
            float controlOutput = 
                positionPGain * motionState.positionError + 
                positionIGain * motionState.positionErrorIntegral;

            // Final smoothed and predicted distance with constraints
            float calculatedDistance = motionState.predictedDistance + controlOutput;
            
            // Apply final constraints to ensure it's in valid range
            currentDistance = constrain((int)calculatedDistance, minDistance, maxDistance);
            
            // Debug output only occasionally to avoid flooding serial
            static unsigned long lastDebugTime = 0;
            if (ENABLE_MOTION_LOGGING && currentTime - lastDebugTime > 3000) {  // Only print every 3 seconds
                lastDebugTime = currentTime;
                // Only log if there's significant change
                static int lastLoggedDistance = 0;
                if (abs(rawDistance - lastLoggedDistance) > 10) {
                    Serial.printf("[Motion] Raw: %d, Smoothed: %.1f, Final: %d\n", 
                               rawDistance, motionState.smoothedDistance, currentDistance);
                    lastLoggedDistance = rawDistance;
                }
            }
        } else {
            // Standard mode without smoothing
            currentDistance = rawDistance;
        }

        // Handle device role-specific logic
        if (deviceRole == DEVICE_ROLE_SLAVE) {
            // SLAVES: Only send data to master, NO LED control in master-slave setup
            bool validMaster = false;
            for (int i = 0; i < 6; i++) {
                if (masterAddress[i] != 0) {
                    validMaster = true;
                    break;
                }
            }
            
            if (validMaster) {
                sendSensorData(currentDistance, direction);
                // REMOVED: LED updates for slaves in master-slave mode
                // Slaves should not control LEDs when connected to a master
            } else {
                // Only log occasionally to prevent flooding
                static unsigned long lastNoMasterLog = 0;
                if (millis() - lastNoMasterLog > 10000) { // Only log every 10 seconds
                    Serial.println("INFO: Slave mode active but no master configured yet");
                    lastNoMasterLog = millis();
                }
                
                // If no master configured, slave can control its own LEDs for testing
                if (lightMode == LIGHT_MODE_STANDARD) {
                    updateLEDs(currentDistance);
                }
            }
            return true;
        }
        
        // MASTER DEVICE: Handle LED updates appropriately
        if (deviceRole == DEVICE_ROLE_MASTER) {
            if (numSlaveDevices == 0) {
                // No slaves configured, update LEDs directly for standard mode
                if (lightMode == LIGHT_MODE_STANDARD) {
                    updateLEDs(currentDistance);
                }
            }
            // If slaves are configured, updateLEDsWithMultiSensorData() will handle LED updates
            // This is called automatically from ESP-NOW when data is received
        }
        
        // STANDALONE MODE: If no role is set or unknown role, act as standalone
        if (deviceRole != DEVICE_ROLE_MASTER && deviceRole != DEVICE_ROLE_SLAVE) {
            if (lightMode == LIGHT_MODE_STANDARD) {
                updateLEDs(currentDistance);
            }
        }
        
        return true;
    }
    
    return false;
}

// Helper functions to access motion state
float getSmoothedDistance() {
    return motionState.smoothedDistance;
}

float getEstimatedVelocity() {
    return motionState.smoothedVelocity;
}

float getPredictedDistance() {
    return motionState.predictedDistance;
}