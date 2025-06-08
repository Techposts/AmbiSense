#include "espnow_manager.h"
#include "radar_manager.h"
#include "led_controller.h"
#include "config.h"
#include "eeprom_manager.h"  // Add this include for LED distribution functions
#include <EEPROM.h>

// Array to store latest readings from each sensor
sensor_data_t latestSensorData[MAX_SLAVE_DEVICES + 1]; // +1 for the master's own reading

// Global variable for sensor priority mode
uint8_t sensorPriorityMode = DEFAULT_SENSOR_PRIORITY_MODE;

// LED distribution mode globals (these should be declared as extern in header)
extern int ledSegmentMode;
extern int ledSegmentStart;
extern int ledSegmentLength;
extern int totalSystemLeds;

// Connection health monitoring
struct ConnectionHealth {
  unsigned long lastReceived;
  uint32_t packetsReceived;
  uint32_t packetsLost;
  bool isHealthy;
};

static ConnectionHealth slaveHealth[MAX_SLAVE_DEVICES + 1];

// Zone-based switching state
struct ZoneSwitchingState {
  bool usingSlaveReading;
  unsigned long lastSwitchTime;
  int lastSelectedDistance;
  bool initialized;
};

static ZoneSwitchingState zoneState = {false, 0, 0, false};

// Callback function for when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (ENABLE_ESPNOW_LOGGING) {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            mac_addr[0], mac_addr[1], mac_addr[2], 
            mac_addr[3], mac_addr[4], mac_addr[5]);
    Serial.printf("ESP-NOW: Packet to %s status: %s\n", 
                 macStr, (status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed"));
  }
}

// Updated callback function for when data is received
void OnDataReceive(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
  if (len == sizeof(sensor_data_t)) {
    sensor_data_t sensorData;
    memcpy(&sensorData, data, sizeof(sensor_data_t));
    
    const uint8_t *mac_addr = recv_info->src_addr;
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            mac_addr[0], mac_addr[1], mac_addr[2], 
            mac_addr[3], mac_addr[4], mac_addr[5]);
    
    if (ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: Received sensor data from %s - ID: %d, Distance: %d cm\n", 
                   macStr, sensorData.sensorId, sensorData.distance);
    }
    
    // Update connection health
    updateConnectionHealth(sensorData.sensorId);
    
    // Process the received distance data
    processSensorData(sensorData);
  }
  else if (len == sizeof(led_segment_data_t)) {
    led_segment_data_t segmentData;
    memcpy(&segmentData, data, sizeof(led_segment_data_t));
    
    if (ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: Received LED segment data - Distance: %d, Start: %d, Total: %d\n", 
                   segmentData.distance, segmentData.startLed, segmentData.totalLeds);
    }
    
    // Process LED segment data (for distributed mode)
    processLEDSegmentData(segmentData);
  }
}

// Initialize ESP-NOW with improved error handling and channel management
void setupESPNOW() {
  Serial.println("ESP-NOW: Initializing...");
  
  // Force specific channel for all devices
  WiFi.disconnect(true);
  delay(100);
  WiFi.mode(WIFI_AP_STA);
  delay(100);
  
  // Set fixed channel for consistent communication
  WiFi.channel(ESPNOW_CHANNEL);
  
  // Print MAC address (useful for setup)
  Serial.print("ESP-NOW: Device MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.printf("ESP-NOW: Using channel %d\n", ESPNOW_CHANNEL);
  
  // Initialize ESP-NOW with retry logic
  esp_err_t initResult = esp_now_init();
  if (initResult != ESP_OK) {
    Serial.printf("ESP-NOW: Init failed with error %d, retrying...\n", initResult);
    delay(1000);
    
    // Cleanup and retry
    esp_now_deinit();
    delay(100);
    initResult = esp_now_init();
    
    if (initResult != ESP_OK) {
      Serial.printf("ESP-NOW: Critical failure - init failed after retry: %d\n", initResult);
      return;
    }
  }
  
  // Get device role from EEPROM
  deviceRole = EEPROM.read(EEPROM_ADDR_DEVICE_ROLE);
  if (deviceRole != DEVICE_ROLE_MASTER && deviceRole != DEVICE_ROLE_SLAVE) {
    deviceRole = DEFAULT_DEVICE_ROLE; // Default to master if not set
    EEPROM.write(EEPROM_ADDR_DEVICE_ROLE, deviceRole);
    EEPROM.commit();
  }
  
  // Get sensor priority mode from EEPROM
  sensorPriorityMode = EEPROM.read(EEPROM_ADDR_SENSOR_PRIORITY_MODE);
  if (sensorPriorityMode > SENSOR_PRIORITY_ZONE_BASED) {
    sensorPriorityMode = DEFAULT_SENSOR_PRIORITY_MODE;
    EEPROM.write(EEPROM_ADDR_SENSOR_PRIORITY_MODE, sensorPriorityMode);
    EEPROM.commit();
  }
  
  // Load LED distribution settings from EEPROM (function from eeprom_manager)
  loadLEDDistributionSettings();
  
  // Set up callback functions
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataReceive);
  
  // Initialize connection health monitoring
  initializeConnectionHealth();
  
  // Configure peers based on role
  if (deviceRole == DEVICE_ROLE_MASTER) {
    Serial.println("ESP-NOW: Configuring device as MASTER");
    configureMasterPeers();
  } else if (deviceRole == DEVICE_ROLE_SLAVE) {
    Serial.println("ESP-NOW: Configuring device as SLAVE");
    configureSlavePeer();
  }
  
  Serial.printf("ESP-NOW: Initialization complete. LED Mode: %s\n", 
               (ledSegmentMode == LED_SEGMENT_MODE_DISTRIBUTED) ? "Distributed" : "Continuous");
}

// Configure master device peers
void configureMasterPeers() {
  // Read number of paired slaves
  numSlaveDevices = EEPROM.read(EEPROM_ADDR_PAIRED_SLAVES);
  numSlaveDevices = constrain(numSlaveDevices, 0, MAX_SLAVE_DEVICES);
  
  if (ENABLE_ESPNOW_LOGGING) {
    Serial.printf("ESP-NOW: Master configuring %d slave devices\n", numSlaveDevices);
  }
  
  // Register all slave devices as peers
  for (int i = 0; i < numSlaveDevices; i++) {
    uint8_t macAddr[6];
    for (int j = 0; j < 6; j++) {
      macAddr[j] = EEPROM.read(EEPROM_ADDR_PAIRED_SLAVES + 1 + (i * 6) + j);
      slaveAddresses[i][j] = macAddr[j]; // Store in global array
    }
    
    // Remove peer if it already exists
    esp_now_del_peer(macAddr);
    
    // Add as new peer with fixed channel
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddr, 6);
    peerInfo.channel = ESPNOW_CHANNEL;  // Use fixed channel
    peerInfo.encrypt = false;
    
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            macAddr[0], macAddr[1], macAddr[2], 
            macAddr[3], macAddr[4], macAddr[5]);
    
    esp_err_t addResult = esp_now_add_peer(&peerInfo);
    if (addResult == ESP_OK) {
      if (ENABLE_ESPNOW_LOGGING) {
        Serial.printf("ESP-NOW: Successfully added slave peer: %s\n", macStr);
      }
    } else {
      Serial.printf("ESP-NOW: Failed to add slave peer %s (error: %d)\n", macStr, addResult);
    }
  }
}

// Configure slave device peer
void configureSlavePeer() {
  // Read master MAC address
  for (int i = 0; i < 6; i++) {
    masterAddress[i] = EEPROM.read(EEPROM_ADDR_MASTER_MAC + i);
  }
  
  // Check if master MAC is valid (not all zeros)
  bool validMaster = false;
  for (int i = 0; i < 6; i++) {
    if (masterAddress[i] != 0) {
      validMaster = true;
      break;
    }
  }
  
  if (!validMaster) {
    Serial.println("ESP-NOW: No valid master MAC configured for slave");
    return;
  }
  
  // Remove any existing peer
  esp_now_del_peer(masterAddress);
  
  // Add master as peer with fixed channel
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, masterAddress, 6);
  peerInfo.channel = ESPNOW_CHANNEL;  // Use fixed channel
  peerInfo.encrypt = false;
  
  char macStr[18];
  sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
          masterAddress[0], masterAddress[1], masterAddress[2], 
          masterAddress[3], masterAddress[4], masterAddress[5]);
  
  esp_err_t addResult = esp_now_add_peer(&peerInfo);
  if (addResult == ESP_OK) {
    Serial.printf("ESP-NOW: Successfully added master peer: %s\n", macStr);
    
    // Send initial test message with delay
    delay(100);
    sendTestMessage();
  } else {
    Serial.printf("ESP-NOW: Failed to add master peer %s (error: %d)\n", macStr, addResult);
  }
}

// Send test message from slave to master
void sendTestMessage() {
  sensor_data_t testData = {0};
  testData.sensorId = 1;
  testData.distance = 0;
  testData.timestamp = millis();
  
  esp_err_t result = esp_now_send(masterAddress, (uint8_t*)&testData, sizeof(sensor_data_t));
  if (result == ESP_OK) {
    Serial.println("ESP-NOW: Test message sent to master");
  } else {
    Serial.printf("ESP-NOW: Failed to send test message (error: %d)\n", result);
  }
}

// Initialize connection health monitoring
void initializeConnectionHealth() {
  for (int i = 0; i <= MAX_SLAVE_DEVICES; i++) {
    slaveHealth[i].lastReceived = millis();
    slaveHealth[i].packetsReceived = 0;
    slaveHealth[i].packetsLost = 0;
    slaveHealth[i].isHealthy = false;
  }
  
  // Initialize latestSensorData array
  for (int i = 0; i <= MAX_SLAVE_DEVICES; i++) {
    latestSensorData[i].sensorId = i;
    latestSensorData[i].distance = 0;
    latestSensorData[i].direction = 0;
    latestSensorData[i].timestamp = 0;
  }
  
  // Initialize zone switching state
  zoneState.usingSlaveReading = false;
  zoneState.lastSwitchTime = millis();
  zoneState.lastSelectedDistance = minDistance;
  zoneState.initialized = true;
}

// Update connection health for a sensor
void updateConnectionHealth(uint8_t sensorId) {
  if (sensorId <= MAX_SLAVE_DEVICES) {
    slaveHealth[sensorId].lastReceived = millis();
    slaveHealth[sensorId].packetsReceived++;
    slaveHealth[sensorId].isHealthy = true;
  }
}

// Check connection health and log issues
void checkConnectionHealth() {
  unsigned long currentTime = millis();
  
  for (int i = 0; i <= numSlaveDevices; i++) {
    if (currentTime - slaveHealth[i].lastReceived > CONNECTION_HEALTH_TIMEOUT) {
      if (slaveHealth[i].isHealthy) {
        Serial.printf("ESP-NOW: WARNING - Lost connection to sensor %d\n", i);
        slaveHealth[i].isHealthy = false;
      }
    }
  }
}

// Send sensor data (called by slave devices) with improved error handling
void sendSensorData(int distance, int8_t direction) {
  if (deviceRole != DEVICE_ROLE_SLAVE) return;
  
  // Check if master MAC is valid
  bool validMaster = false;
  for (int i = 0; i < 6; i++) {
    if (masterAddress[i] != 0) {
      validMaster = true;
      break;
    }
  }
  
  if (!validMaster) {
    static unsigned long lastLogTime = 0;
    if (millis() - lastLogTime > 10000) {
      Serial.println("ESP-NOW: Slave mode active but no master configured");
      lastLogTime = millis();
    }
    return;
  }
  
  // Prepare sensor data
  sensor_data_t sensorData;
  sensorData.sensorId = 1; 
  sensorData.distance = distance;
  sensorData.direction = direction;
  sensorData.battery = 100;
  sensorData.timestamp = millis();
  
  // Send with retry logic
  int retryCount = 0;
  esp_err_t result;
  
  do {
    result = esp_now_send(masterAddress, (uint8_t*)&sensorData, sizeof(sensor_data_t));
    if (result == ESP_OK) {
      break;
    }
    
    retryCount++;
    if (retryCount < ESPNOW_RETRY_COUNT) {
      delay(10); // Small delay before retry
    }
  } while (retryCount < ESPNOW_RETRY_COUNT);
  
  if (result != ESP_OK && ENABLE_ESPNOW_LOGGING) {
    static unsigned long lastErrorTime = 0;
    if (millis() - lastErrorTime > 1000) {
      Serial.printf("ESP-NOW: Failed to send data after %d retries (error: %d)\n", 
                   ESPNOW_RETRY_COUNT, result);
      lastErrorTime = millis();
    }
  }
}

// Send LED segment data to slaves (for distributed mode)
void sendLEDSegmentData(int distance, int globalStartPos) {
  if (deviceRole != DEVICE_ROLE_MASTER || 
      ledSegmentMode != LED_SEGMENT_MODE_DISTRIBUTED ||
      numSlaveDevices == 0) {
    return;
  }
  
  led_segment_data_t segmentData;
  segmentData.sensorId = 0; // Master
  segmentData.distance = distance;
  segmentData.direction = 0;
  segmentData.timestamp = millis();
  segmentData.totalLeds = totalSystemLeds;
  segmentData.lightMode = lightMode;
  segmentData.brightness = brightness;
  segmentData.redValue = redValue;
  segmentData.greenValue = greenValue;
  segmentData.blueValue = blueValue;
  
  // Send to all slaves with their specific segment information
  for (int i = 0; i < numSlaveDevices; i++) {
    // Calculate segment for this slave (assuming equal distribution)
    int ledsPerDevice = totalSystemLeds / (numSlaveDevices + 1); // +1 for master
    int slaveSegmentStart = (i + 1) * ledsPerDevice;
    
    segmentData.startLed = slaveSegmentStart;
    segmentData.segmentLength = ledsPerDevice;
    
    esp_err_t result = esp_now_send(slaveAddresses[i], 
                                   (uint8_t*)&segmentData, 
                                   sizeof(led_segment_data_t));
    
    if (result != ESP_OK && ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: Failed to send LED segment data to slave %d (error: %d)\n", 
                   i, result);
    }
  }
  
  if (ENABLE_ESPNOW_LOGGING) {
    Serial.printf("ESP-NOW: Sent LED segment data - Distance: %d, Global start: %d\n", 
                 distance, globalStartPos);
  }
}

// Process LED segment data (on slave devices)
void processLEDSegmentData(led_segment_data_t segmentData) {
  if (deviceRole != DEVICE_ROLE_SLAVE || 
      ledSegmentMode != LED_SEGMENT_MODE_DISTRIBUTED) {
    return;
  }
  
  // Update local settings from master
  brightness = segmentData.brightness;
  redValue = segmentData.redValue;
  greenValue = segmentData.greenValue;
  blueValue = segmentData.blueValue;
  
  // Calculate global LED position from distance
  int globalStartPos = map(segmentData.distance, minDistance, maxDistance, 
                          0, segmentData.totalLeds - movingLightSpan);
  
  // Update LED segment based on the received data
  updateLEDSegment(globalStartPos, segmentData);
}

// Update LED segment for distributed mode
void updateLEDSegment(int globalStartPos, led_segment_data_t segmentData) {
  strip.clear();
  
  // Calculate which part of the moving light belongs to this segment
  for (int i = 0; i < movingLightSpan; i++) {
    int globalLedPos = globalStartPos + i;
    int localLedPos = globalLedPos - ledSegmentStart;
    
    // Check if this LED position belongs to this device's segment
    if (localLedPos >= 0 && localLedPos < numLeds) {
      strip.setPixelColor(localLedPos, strip.Color(segmentData.redValue, 
                                                  segmentData.greenValue, 
                                                  segmentData.blueValue));
    }
  }
  
  strip.show();
  
  if (ENABLE_ESPNOW_LOGGING) {
    Serial.printf("ESP-NOW: Updated LED segment - Global start: %d, Local LEDs updated\n", 
                 globalStartPos);
  }
}

// Process received sensor data (called by master device)
void processSensorData(sensor_data_t sensorData) {
  if (deviceRole != DEVICE_ROLE_MASTER) return;
  
  // Validate incoming data
  if (sensorData.distance < 0 || sensorData.distance > 500) {
    if (ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: Discarding invalid distance %d from sensor %d\n", 
                  sensorData.distance, sensorData.sensorId);
    }
    return;
  }
  
  // Store the latest reading from this sensor
  if (sensorData.sensorId <= MAX_SLAVE_DEVICES) {
    latestSensorData[sensorData.sensorId] = sensorData;
    
    if (ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: Stored data from sensor %d: Distance = %d cm\n", 
                   sensorData.sensorId, sensorData.distance);
    }
  }
  
  // Update LEDs based on combined sensor data
  updateLEDsWithMultiSensorData();
}

// Enhanced LED update with improved zone-based switching and LED distribution
void updateLEDsWithMultiSensorData() {
  if (deviceRole != DEVICE_ROLE_MASTER) return;
  
  unsigned long currentTime = millis();
  
  // Check connection health periodically
  static unsigned long lastHealthCheck = 0;
  if (currentTime - lastHealthCheck > 5000) {
    checkConnectionHealth();
    lastHealthCheck = currentTime;
  }
  
  // Store master's own sensor reading
  latestSensorData[0].sensorId = 0;
  latestSensorData[0].distance = currentDistance;
  latestSensorData[0].timestamp = currentTime;
  
  int selectedDistance = 0;
  
  // Implementation based on the selected sensor priority mode
  switch (sensorPriorityMode) {
    case SENSOR_PRIORITY_MOST_RECENT: {
      selectedDistance = handleMostRecentPriority(currentTime);
      break;
    }
    
    case SENSOR_PRIORITY_SLAVE_FIRST: {
      selectedDistance = handleSlaveFirstPriority(currentTime);
      break;
    }
    
    case SENSOR_PRIORITY_MASTER_FIRST: {
      selectedDistance = handleMasterFirstPriority(currentTime);
      break;
    }
    
    case SENSOR_PRIORITY_ZONE_BASED: {
      selectedDistance = handleZoneBasedPriority(currentTime);
      break;
    }
    
    default: {
      selectedDistance = handleMostRecentPriority(currentTime);
      break;
    }
  }
  
  // Apply constraints and update LEDs
  selectedDistance = constrain(selectedDistance, minDistance, maxDistance);
  currentDistance = selectedDistance;
  
  if (lightMode == LIGHT_MODE_STANDARD) {
    if (ledSegmentMode == LED_SEGMENT_MODE_DISTRIBUTED) {
      // Calculate global LED position
      int globalStartPos = map(selectedDistance, minDistance, maxDistance, 
                              0, totalSystemLeds - movingLightSpan);
      
      // Update master's segment
      int localStartPos = globalStartPos - ledSegmentStart;
      localStartPos = constrain(localStartPos, 0, numLeds - movingLightSpan);
      
      updateStandardMode(localStartPos);
      
      // Send segment data to slaves
      sendLEDSegmentData(selectedDistance, globalStartPos);
    } else {
      // Continuous mode - normal LED update
      updateLEDs(selectedDistance);
    }
  }
}

// Handle most recent priority mode
int handleMostRecentPriority(unsigned long currentTime) {
  unsigned long mostRecentTime = 0;
  int selectedDistance = 0;
  bool movementDetected = false;
  
  for (int i = 0; i <= numSlaveDevices; i++) {
    if (currentTime - latestSensorData[i].timestamp > 5000) {
      continue; // Skip old data
    }
    
    if (latestSensorData[i].distance > 0) {
      movementDetected = true;
      if (latestSensorData[i].timestamp > mostRecentTime) {
        mostRecentTime = latestSensorData[i].timestamp;
        selectedDistance = latestSensorData[i].distance;
      }
    }
  }
  
  return movementDetected ? selectedDistance : currentDistance;
}

// Handle slave first priority mode
int handleSlaveFirstPriority(unsigned long currentTime) {
  bool slaveDetected = false;
  int slaveDistance = 0;
  unsigned long mostRecentSlaveTime = 0;
  
  // Check slave sensors first
  for (int i = 1; i <= numSlaveDevices; i++) {
    if (currentTime - latestSensorData[i].timestamp > 5000) {
      continue;
    }
    
    if (latestSensorData[i].distance > 0) {
      slaveDetected = true;
      if (latestSensorData[i].timestamp > mostRecentSlaveTime) {
        mostRecentSlaveTime = latestSensorData[i].timestamp;
        slaveDistance = latestSensorData[i].distance;
      }
    }
  }
  
  if (slaveDetected) {
    return slaveDistance;
  } else if (latestSensorData[0].distance > 0 && 
             currentTime - latestSensorData[0].timestamp < 5000) {
    return latestSensorData[0].distance;
  }
  
  return currentDistance;
}

// Handle master first priority mode
int handleMasterFirstPriority(unsigned long currentTime) {
  if (latestSensorData[0].distance > 0 && 
      currentTime - latestSensorData[0].timestamp < 5000) {
    return latestSensorData[0].distance;
  } else {
    // Check slaves as fallback
    return handleSlaveFirstPriority(currentTime);
  }
}

// Enhanced zone-based priority with better hysteresis
int handleZoneBasedPriority(unsigned long currentTime) {
  if (!zoneState.initialized) {
    zoneState.usingSlaveReading = false;
    zoneState.lastSwitchTime = currentTime;
    zoneState.lastSelectedDistance = currentDistance;
    zoneState.initialized = true;
  }
  
  // Check for activity in slave zones (L-section)
  bool slaveDetectedMovement = false;
  int bestSlaveDistance = 0;
  unsigned long bestSlaveTimestamp = 0;
  
  for (int i = 1; i <= numSlaveDevices; i++) {
    // Skip old readings (more than 4 seconds old)
    if (currentTime - latestSensorData[i].timestamp > 4000) {
      continue;
    }
    
    if (latestSensorData[i].distance > 0) {
      slaveDetectedMovement = true;
      // If multiple slaves detect movement, use the most recent one
      if (latestSensorData[i].timestamp > bestSlaveTimestamp) {
        bestSlaveTimestamp = latestSensorData[i].timestamp;
        bestSlaveDistance = latestSensorData[i].distance;
      }
    }
  }
  
  // Check for activity in master zone (lower section)
  bool masterDetectedMovement = (latestSensorData[0].distance > 0 && 
                                 currentTime - latestSensorData[0].timestamp < 4000);
  
  // Enhanced decision logic with hysteresis and momentum
  if (slaveDetectedMovement) {
    // If slave detects movement, switch to slave control
    if (!zoneState.usingSlaveReading) {
      zoneState.usingSlaveReading = true;
      zoneState.lastSwitchTime = currentTime;
      if (ENABLE_ESPNOW_LOGGING) {
        Serial.println("ESP-NOW: Zone switch -> SLAVE sensors (L-section)");
      }
    }
    zoneState.lastSelectedDistance = bestSlaveDistance;
    return bestSlaveDistance;
  } 
  else if (!slaveDetectedMovement && masterDetectedMovement) {
    // If only master detects movement and no active slave, consider switching
    if (zoneState.usingSlaveReading && currentTime - zoneState.lastSwitchTime > 2000) {
      // Add hysteresis delay to prevent rapid switching
      zoneState.usingSlaveReading = false;
      zoneState.lastSwitchTime = currentTime;
      if (ENABLE_ESPNOW_LOGGING) {
        Serial.println("ESP-NOW: Zone switch -> MASTER sensor (lower section)");
      }
    }
    
    if (!zoneState.usingSlaveReading) {
      zoneState.lastSelectedDistance = latestSensorData[0].distance;
      return latestSensorData[0].distance;
    }
  } 
  else if (!slaveDetectedMovement && !masterDetectedMovement) {
    // No movement detected - implement gradual fade or maintain last reading
    if (currentTime - zoneState.lastSwitchTime > 3000) {
      // After 3 seconds of no detection, gradually increase distance (fade effect)
      int fadeDistance = min(maxDistance, zoneState.lastSelectedDistance + 2);
      zoneState.lastSelectedDistance = fadeDistance;
      return fadeDistance;
    }
  }
  
  // Default: maintain last selected distance
  return zoneState.lastSelectedDistance;
}

// Set the sensor priority mode
void setSensorPriorityMode(uint8_t mode) {
  if (mode <= SENSOR_PRIORITY_ZONE_BASED) {
    sensorPriorityMode = mode;
    EEPROM.write(EEPROM_ADDR_SENSOR_PRIORITY_MODE, sensorPriorityMode);
    EEPROM.commit();
    
    // Reset zone state when changing modes
    if (mode == SENSOR_PRIORITY_ZONE_BASED) {
      zoneState.initialized = false;
    }
    
    if (ENABLE_ESPNOW_LOGGING) {
      const char* modeNames[] = {"Most Recent", "Slave First", "Master First", "Zone-Based"};
      Serial.printf("ESP-NOW: Sensor priority mode set to %s (%d)\n", 
                   modeNames[mode], mode);
    }
  }
}

// Get the current sensor priority mode
uint8_t getSensorPriorityMode() {
  return sensorPriorityMode;
}

// LED Distribution Mode functions
void setLEDSegmentMode(int mode) {
  if (mode == LED_SEGMENT_MODE_CONTINUOUS || mode == LED_SEGMENT_MODE_DISTRIBUTED) {
    ledSegmentMode = mode;
    saveLEDDistributionSettings();  // This function is from eeprom_manager
    
    if (ENABLE_ESPNOW_LOGGING) {
      Serial.printf("ESP-NOW: LED segment mode set to %s\n", 
                   (mode == LED_SEGMENT_MODE_DISTRIBUTED) ? "Distributed" : "Continuous");
    }
  }
}

int getLEDSegmentMode() {
  return ledSegmentMode;
}

void setLEDSegmentInfo(int start, int length, int total) {
  ledSegmentStart = constrain(start, 0, total - 1);
  ledSegmentLength = constrain(length, 1, total);
  totalSystemLeds = constrain(total, 1, MAX_SUPPORTED_LEDS);
  
  saveLEDDistributionSettings();  // This function is from eeprom_manager
  
  Serial.printf("ESP-NOW: LED segment info set - Start: %d, Length: %d, Total: %d\n", 
                ledSegmentStart, ledSegmentLength, totalSystemLeds);
}

void getLEDSegmentInfo(int* start, int* length, int* total) {
  *start = ledSegmentStart;
  *length = ledSegmentLength;
  *total = totalSystemLeds;
}

// Get connection health status for diagnostics
bool getSensorHealth(uint8_t sensorId) {
  if (sensorId <= MAX_SLAVE_DEVICES) {
    return slaveHealth[sensorId].isHealthy;
  }
  return false;
}

// Get packet statistics for diagnostics
uint32_t getSensorPacketCount(uint8_t sensorId) {
  if (sensorId <= MAX_SLAVE_DEVICES) {
    return slaveHealth[sensorId].packetsReceived;
  }
  return 0;
}

// Get last received time for diagnostics
unsigned long getSensorLastReceived(uint8_t sensorId) {
  if (sensorId <= MAX_SLAVE_DEVICES) {
    return slaveHealth[sensorId].lastReceived;
  }
  return 0;
}

// Reset ESP-NOW and reinitialize (useful for recovery)
void resetESPNOW() {
  Serial.println("ESP-NOW: Resetting and reinitializing...");
  
  esp_now_deinit();
  delay(100);
  
  setupESPNOW();
}

// Enhanced diagnostic information
void printESPNOWDiagnostics() {
  Serial.println("\n=== ESP-NOW Diagnostics ===");
  Serial.printf("Device Role: %s\n", 
                (deviceRole == DEVICE_ROLE_MASTER) ? "Master" : "Slave");
  Serial.printf("Priority Mode: %d\n", sensorPriorityMode);
  Serial.printf("Channel: %d\n", ESPNOW_CHANNEL);
  Serial.printf("LED Mode: %s\n", 
                (ledSegmentMode == LED_SEGMENT_MODE_DISTRIBUTED) ? "Distributed" : "Continuous");
  
  if (ledSegmentMode == LED_SEGMENT_MODE_DISTRIBUTED) {
    Serial.printf("LED Segment: Start=%d, Length=%d, Total=%d\n", 
                  ledSegmentStart, ledSegmentLength, totalSystemLeds);
  }
  
  if (deviceRole == DEVICE_ROLE_MASTER) {
    Serial.printf("Paired Slaves: %d\n", numSlaveDevices);
    for (int i = 0; i <= numSlaveDevices; i++) {
      Serial.printf("Sensor %d: %s, Packets: %d, Last: %lu ms ago\n",
                    i,
                    slaveHealth[i].isHealthy ? "Healthy" : "Disconnected",
                    slaveHealth[i].packetsReceived,
                    millis() - slaveHealth[i].lastReceived);
    }
    
    if (sensorPriorityMode == SENSOR_PRIORITY_ZONE_BASED) {
      Serial.printf("Zone State: Using %s, Last switch: %lu ms ago\n",
                    zoneState.usingSlaveReading ? "Slave" : "Master",
                    millis() - zoneState.lastSwitchTime);
    }
  } else {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", 
            masterAddress[0], masterAddress[1], masterAddress[2], 
            masterAddress[3], masterAddress[4], masterAddress[5]);
    Serial.printf("Master MAC: %s\n", macStr);
  }
  
  // Memory usage
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
  
  Serial.println("========================\n");
}

// Force synchronize all devices (master only)
void synchronizeAllDevices() {
  if (deviceRole != DEVICE_ROLE_MASTER) return;
  
  Serial.println("ESP-NOW: Synchronizing all devices...");
  
  // Send current settings to all slaves
  led_segment_data_t syncData;
  syncData.sensorId = 0;
  syncData.distance = currentDistance;
  syncData.direction = 0;
  syncData.timestamp = millis();
  syncData.totalLeds = totalSystemLeds;
  syncData.lightMode = lightMode;
  syncData.brightness = brightness;
  syncData.redValue = redValue;
  syncData.greenValue = greenValue;
  syncData.blueValue = blueValue;
  
  for (int i = 0; i < numSlaveDevices; i++) {
    // Calculate segment for this slave
    int ledsPerDevice = totalSystemLeds / (numSlaveDevices + 1);
    syncData.startLed = (i + 1) * ledsPerDevice;
    syncData.segmentLength = ledsPerDevice;
    
    esp_err_t result = esp_now_send(slaveAddresses[i], 
                                    (uint8_t*)&syncData, 
                                    sizeof(led_segment_data_t));
    
    if (result == ESP_OK) {
      Serial.printf("ESP-NOW: Sync data sent to slave %d\n", i);
    } else {
      Serial.printf("ESP-NOW: Failed to sync slave %d (error: %d)\n", i, result);
    }
    
    delay(10); // Small delay between sends
  }
  
  Serial.println("ESP-NOW: Synchronization complete");
}

// Get system status for web interface
void getSystemStatus(char* statusJson, size_t maxLength) {
  snprintf(statusJson, maxLength,
    "{"
    "\"role\":%d,"
    "\"priorityMode\":%d,"
    "\"ledMode\":%d,"
    "\"ledSegmentStart\":%d,"
    "\"ledSegmentLength\":%d,"
    "\"totalSystemLeds\":%d,"
    "\"numSlaves\":%d,"
    "\"channel\":%d,"
    "\"currentDistance\":%d,"
    "\"usingSlaveReading\":%s,"
    "\"freeHeap\":%d"
    "}",
    deviceRole,
    sensorPriorityMode,
    ledSegmentMode,
    ledSegmentStart,
    ledSegmentLength,
    totalSystemLeds,
    numSlaveDevices,
    ESPNOW_CHANNEL,
    currentDistance,
    (zoneState.usingSlaveReading ? "true" : "false"),
    ESP.getFreeHeap()
  );
}

// Emergency stop all LEDs across the network
void emergencyStopAllLEDs() {
  if (deviceRole == DEVICE_ROLE_MASTER) {
    Serial.println("ESP-NOW: Emergency stop - turning off all LEDs");
    
    // Turn off local LEDs
    strip.clear();
    strip.show();
    
    // Send emergency stop to all slaves
    led_segment_data_t stopData = {0};
    stopData.sensorId = 255; // Special ID for emergency stop
    stopData.distance = 0;
    stopData.timestamp = millis();
    stopData.brightness = 0;
    
    for (int i = 0; i < numSlaveDevices; i++) {
      esp_now_send(slaveAddresses[i], (uint8_t*)&stopData, sizeof(led_segment_data_t));
      delay(5);
    }
  } else {
    // Slave emergency stop
    strip.clear();
    strip.show();
  }
}

// Process emergency stop (for slaves)
void processEmergencyStop() {
  Serial.println("ESP-NOW: Emergency stop received");
  strip.clear();
  strip.show();
}

// Enhanced packet loss detection
void checkPacketLoss() {
  if (deviceRole != DEVICE_ROLE_MASTER) return;
  
  unsigned long currentTime = millis();
  static unsigned long lastPacketLossCheck = 0;
  
  if (currentTime - lastPacketLossCheck > 30000) { // Check every 30 seconds
    lastPacketLossCheck = currentTime;
    
    for (int i = 1; i <= numSlaveDevices; i++) {
      unsigned long timeSinceLastPacket = currentTime - slaveHealth[i].lastReceived;
      
      if (timeSinceLastPacket > 60000) { // No packet for 1 minute
        Serial.printf("ESP-NOW: WARNING - Sensor %d has been offline for %lu seconds\n", 
                      i, timeSinceLastPacket / 1000);
        
        // Mark as unhealthy
        slaveHealth[i].isHealthy = false;
        
        // Consider automatic slave removal after extended offline period
        if (timeSinceLastPacket > 300000) { // 5 minutes
          Serial.printf("ESP-NOW: Considering removal of sensor %d (offline for 5+ minutes)\n", i);
        }
      }
    }
  }
}

// Auto-discovery mode for new slaves
void startSlaveDiscovery(unsigned long duration) {
  if (deviceRole != DEVICE_ROLE_MASTER) return;
  
  Serial.printf("ESP-NOW: Starting slave discovery for %lu seconds\n", duration / 1000);
  
  // Implementation would involve listening for discovery packets
  // and automatically adding responsive slaves
  // This is a framework for future enhancement
}

// Network performance metrics
void getNetworkMetrics(uint32_t* totalPacketsReceived, uint32_t* totalPacketsLost, 
                      float* averageRSSI) {
  *totalPacketsReceived = 0;
  *totalPacketsLost = 0;
  *averageRSSI = 0;
  
  int activeSensors = 0;
  
  for (int i = 0; i <= numSlaveDevices; i++) {
    *totalPacketsReceived += slaveHealth[i].packetsReceived;
    *totalPacketsLost += slaveHealth[i].packetsLost;
    
    if (slaveHealth[i].isHealthy) {
      activeSensors++;
    }
  }
  
  // RSSI would need to be tracked separately in real implementation
  *averageRSSI = -50.0; // Placeholder
}

// Periodic maintenance function - call this from main loop
void espnowMaintenance() {
  static unsigned long lastMaintenance = 0;
  unsigned long currentTime = millis();
  
  if (currentTime - lastMaintenance > 5000) { // Every 5 seconds
    lastMaintenance = currentTime;
    
    checkConnectionHealth();
    checkPacketLoss();
    
    // Additional maintenance tasks can be added here
  }
}