#ifndef RESOURCES_H
#define RESOURCES_H

#include <Arduino.h>

// Check available memory and PSRAM status
inline void checkMemory() {
    Serial.println("Memory diagnostics:");
    Serial.printf("Total heap: %d\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
    Serial.printf("Min free heap: %d\n", ESP.getMinFreeHeap());
    Serial.printf("Max alloc heap: %d\n", ESP.getMaxAllocHeap());
    
#ifdef ESP_IDF_VERSION_MAJOR // Check if IDF version is available
    // Only for ESP32s with PSRAM
    if (ESP.getPsramSize() > 0) {
        Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
        Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
    } else {
        Serial.println("No PSRAM detected");
    }
#endif
}

// Initialize PSRAM if available
inline bool initPSRAM() {
#ifdef ESP_IDF_VERSION_MAJOR
    if (ESP.getPsramSize() > 0) {
        Serial.println("PSRAM is enabled");
        // ESP32 with PSRAM detected
        heap_caps_malloc_extmem_enable(20000); // Prefer external memory for allocations over 20KB
        return true;
    } else {
        Serial.println("No PSRAM available");
        return false;
    }
#else
    return false;
#endif
}

#endif // RESOURCES_H