#include <ld2410.h>
#include "config.h"
#include "radar_manager.h"
#include "led_controller.h"

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
    
    // Print motion smoothing configuration
    Serial.println("Radar sensor initialized with motion smoothing:");
    Serial.print("Motion Smoothing Enabled: ");
    Serial.println(motionSmoothingEnabled ? "Yes" : "No");
    Serial.print("Position Smoothing Factor: ");
    Serial.println(positionSmoothingFactor);
    Serial.print("Velocity Smoothing Factor: ");
    Serial.println(velocitySmoothingFactor);
    Serial.print("Prediction Factor: ");
    Serial.println(predictionFactor);
    Serial.print("Position P Gain: ");
    Serial.println(positionPGain);
    Serial.print("Position I Gain: ");
    Serial.println(positionIGain);
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

        // Motion smoothing logic
        if (motionSmoothingEnabled) {
            // Time since last update (for velocity calculation)
            unsigned long currentTime = millis();
            float deltaTime = (currentTime - motionState.lastUpdateTime) / 1000.0; // Convert to seconds
            motionState.lastUpdateTime = currentTime;
            
            // If this is the first time after enabling, log it
            static bool firstRun = true;
            if (firstRun) {
                Serial.println("[Motion] Motion smoothing active - starting calculations");
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
            
            // Debug output every 500ms to avoid flooding serial
            static unsigned long lastDebugTime = 0;
            if (currentTime - lastDebugTime > 500) {  // Only print every 500ms
                lastDebugTime = currentTime;
                Serial.printf("[Motion] Raw: %d, Smoothed: %.1f, Velocity: %.1f, Predicted: %.1f, Final: %d\n", 
                           rawDistance, motionState.smoothedDistance, motionState.smoothedVelocity, 
                           motionState.predictedDistance, currentDistance);
            }
        } else {
            // Standard mode without smoothing
            currentDistance = rawDistance;
        }

        // Update the LEDs with the current distance
        updateLEDs(currentDistance);
        
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