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
            
            // Prevent divide-by-zero and extreme delta times
            if (deltaTime <= 0.001) deltaTime = 0.001; // Minimum 1ms
            if (deltaTime > 0.5) deltaTime = 0.5;      // Cap at 0.5 second - reduced from 1.0 for faster response
            
            // If this is the first reading or smoothedDistance is not initialized
            if (motionState.smoothedDistance <= 0) {
                motionState.smoothedDistance = (float)rawDistance;
                motionState.predictedDistance = (float)rawDistance;
                motionState.smoothedVelocity = 0.0;
                motionState.positionError = 0.0;
                motionState.positionErrorIntegral = 0.0;
            }

            // Low-pass filter for position - reduce smoothing factor for faster response
            float adjustedSmoothing = positionSmoothingFactor * 1.5; // Increase responsiveness
            if (adjustedSmoothing > 0.9) adjustedSmoothing = 0.9; // Cap at 0.9
            
            motionState.smoothedDistance = 
                (1.0f - adjustedSmoothing) * motionState.smoothedDistance + 
                adjustedSmoothing * (float)rawDistance;

            // Velocity estimation with more responsive parameters
            float instantVelocity = (motionState.smoothedDistance - motionState.predictedDistance) / deltaTime;
            instantVelocity = constrain(instantVelocity, -300.0f, 300.0f); // Extended range from -200/200
            
            // More responsive velocity smoothing
            float adjustedVelocitySmoothing = velocitySmoothingFactor * 1.3; // More responsive
            if (adjustedVelocitySmoothing > 0.8) adjustedVelocitySmoothing = 0.8; // Cap at 0.8
            
            motionState.smoothedVelocity = 
                (1.0f - adjustedVelocitySmoothing) * motionState.smoothedVelocity + 
                adjustedVelocitySmoothing * instantVelocity;

            // More responsive prediction
            motionState.predictedDistance = 
                motionState.smoothedDistance + 
                motionState.smoothedVelocity * (predictionFactor * 0.8); // Reduce prediction influence

            // Position correction
            motionState.positionError = motionState.predictedDistance - motionState.smoothedDistance;
            motionState.positionErrorIntegral += motionState.positionError * deltaTime;
            motionState.positionErrorIntegral = constrain(motionState.positionErrorIntegral, -100.0f, 100.0f);

            float controlOutput = 
                positionPGain * motionState.positionError + 
                positionIGain * motionState.positionErrorIntegral;

            // Calculate final distance with more responsiveness to raw inputs
            float calculatedDistance = (motionState.predictedDistance * 0.6) + (rawDistance * 0.4) + controlOutput;
            
            // Apply final constraints
            currentDistance = constrain((int)calculatedDistance, minDistance, maxDistance);
            
            // Debug output less frequently to reduce serial processing overhead
            static unsigned long lastDebugTime = 0;
            if (currentTime - lastDebugTime > 2000) {  // Reduced from every 500ms to every 2s
                lastDebugTime = currentTime;
                Serial.printf("[Motion] Raw: %d, Smoothed: %.1f, Final: %d\n", 
                           rawDistance, motionState.smoothedDistance, currentDistance);
            }
        } else {
            // No smoothing - direct assignment
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