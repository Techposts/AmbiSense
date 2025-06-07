#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>
#include "config.h"

// Data structure for ESP-NOW communication
typedef struct sensor_data_t {
  uint8_t sensorId;     // Identifier for the sending sensor
  int distance;         // Distance reading in cm
  int8_t direction;     // -1: moving closer, 0: stationary, 1: moving away
  uint8_t battery;      // Battery level (if applicable)
  uint32_t timestamp;   // Milliseconds since boot
} sensor_data_t;

typedef struct led_segment_data_t {
  uint8_t sensorId;
  int distance;
  int8_t direction;
  uint32_t timestamp;
  
  // LED segment control
  int startLed;      // Which LED this device should start from
  int segmentLength; // How many LEDs this device controls
  int totalLeds;     // Total LEDs in the system
  uint8_t lightMode; // Current light mode
  uint8_t brightness; // Current brightness
  uint8_t redValue;
  uint8_t greenValue;
  uint8_t blueValue;
} led_segment_data_t;

extern sensor_data_t latestSensorData[MAX_SLAVE_DEVICES + 1];

/**
 * Initialize ESP-NOW communication with improved error handling
 * Sets up callbacks and configures peers based on device role
 */
void setupESPNOW();

/**
 * Configure master device peers with channel handling
 */
void configureMasterPeers();

/**
 * Configure slave device peer with channel handling
 */
void configureSlavePeer();

/**
 * Send test message from slave to master
 */
void sendTestMessage();

/**
 * Initialize connection health monitoring
 */
void initializeConnectionHealth();

/**
 * Update connection health for a sensor
 * @param sensorId The sensor ID to update
 */
void updateConnectionHealth(uint8_t sensorId);

/**
 * Check connection health and log issues
 */
void checkConnectionHealth();

/**
 * Send sensor data from slave to master with retry logic
 * @param distance The current distance reading
 * @param direction The detected direction of movement
 */
void sendSensorData(int distance, int8_t direction);

/**
 * Process received sensor data (called by master device)
 * @param sensorData The sensor data structure received
 */
void processSensorData(sensor_data_t sensorData);

/**
 * Update LEDs with combined sensor data using enhanced algorithms
 */
void updateLEDsWithMultiSensorData();

/**
 * Priority mode handlers
 */
int handleMostRecentPriority(unsigned long currentTime);
int handleSlaveFirstPriority(unsigned long currentTime);
int handleMasterFirstPriority(unsigned long currentTime);
int handleZoneBasedPriority(unsigned long currentTime);

/**
 * Set the sensor priority mode
 * @param mode The priority mode to set (0-3)
 */
void setSensorPriorityMode(uint8_t mode);

/**
 * Get the current sensor priority mode
 * @return The current priority mode
 */
uint8_t getSensorPriorityMode();

/**
 * LED Distribution Mode functions
 * Note: These call functions from eeprom_manager for persistence
 */
void setLEDSegmentMode(int mode);
int getLEDSegmentMode();
void setLEDSegmentInfo(int start, int length, int total);
void getLEDSegmentInfo(int* start, int* length, int* total);

/**
 * Send LED segment data to slaves (for distributed mode)
 * @param distance Current distance reading
 * @param globalStartPos Global LED position
 */
void sendLEDSegmentData(int distance, int globalStartPos);

/**
 * Process LED segment data (on slave devices)
 * @param segmentData LED segment data structure
 */
void processLEDSegmentData(struct led_segment_data_t segmentData);

/**
 * Update LED segment for distributed mode
 * @param globalStartPos Global LED start position
 * @param segmentData LED segment data
 */
void updateLEDSegment(int globalStartPos, struct led_segment_data_t segmentData);

/**
 * Get connection health status for diagnostics
 * @param sensorId The sensor ID to check
 * @return True if sensor is healthy
 */
bool getSensorHealth(uint8_t sensorId);

/**
 * Get packet statistics for diagnostics
 * @param sensorId The sensor ID to check
 * @return Number of packets received
 */
uint32_t getSensorPacketCount(uint8_t sensorId);

/**
 * Get last received time for diagnostics
 * @param sensorId The sensor ID to check
 * @return Last received timestamp
 */
unsigned long getSensorLastReceived(uint8_t sensorId);

/**
 * Reset ESP-NOW and reinitialize
 */
void resetESPNOW();

/**
 * Print diagnostic information
 */
void printESPNOWDiagnostics();

/**
 * Synchronize all devices (master only)
 */
void synchronizeAllDevices();

/**
 * Get system status for web interface
 * @param statusJson Buffer to store JSON status
 * @param maxLength Maximum buffer length
 */
void getSystemStatus(char* statusJson, size_t maxLength);

/**
 * Emergency stop all LEDs across the network
 */
void emergencyStopAllLEDs();

/**
 * Process emergency stop (for slaves)
 */
void processEmergencyStop();

/**
 * Enhanced packet loss detection
 */
void checkPacketLoss();

/**
 * Auto-discovery mode for new slaves
 * @param duration Discovery duration in milliseconds
 */
void startSlaveDiscovery(unsigned long duration);

/**
 * Network performance metrics
 * @param totalPacketsReceived Total packets received across all sensors
 * @param totalPacketsLost Total packets lost across all sensors
 * @param averageRSSI Average signal strength
 */
void getNetworkMetrics(uint32_t* totalPacketsReceived, uint32_t* totalPacketsLost, 
                      float* averageRSSI);

/**
 * Periodic maintenance function - call this from main loop
 */
void espnowMaintenance();

/**
 * Get current ESP-NOW channel
 * @return Current channel number
 */
int getCurrentESPNowChannel();

/**
 * Check if ESP-NOW is properly initialized
 * @return True if initialized, false otherwise
 */
bool isESPNowInitialized();

// Callback for ESP-NOW data receive
void OnDataReceive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len);

// Callback for ESP-NOW data sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

// Channel change callback
void onWiFiChannelChange(int newChannel);

#endif // ESPNOW_MANAGER_H