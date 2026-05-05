#ifndef MEMORY_ANALYSIS_H
#define MEMORY_ANALYSIS_H

#include <Arduino.h>

// Memory analysis utilities for AmbiSense
// Use these functions to monitor flash and RAM usage

void printMemoryUsage() {
  Serial.println("=== Memory Usage Analysis ===");
  
  // Heap Memory
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Min Free Heap: %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("Max Alloc Heap: %d bytes\n", ESP.getMaxAllocHeap());
  Serial.printf("Heap Usage: %.1f%%\n", 
    (float)(ESP.getHeapSize() - ESP.getFreeHeap()) / ESP.getHeapSize() * 100);
  
  // Flash Memory
  Serial.printf("Flash Chip Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("Sketch Size: %d bytes\n", ESP.getSketchSize());
  Serial.printf("Free Sketch Space: %d bytes\n", ESP.getFreeSketchSpace());
  Serial.printf("Flash Usage: %.1f%%\n", 
    (float)ESP.getSketchSize() / ESP.getFlashChipSize() * 100);
  
  // PSRAM (if available)
  if (ESP.getPsramSize() > 0) {
    Serial.printf("Total PSRAM: %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
    Serial.printf("PSRAM Usage: %.1f%%\n", 
      (float)(ESP.getPsramSize() - ESP.getFreePsram()) / ESP.getPsramSize() * 100);
  } else {
    Serial.println("No PSRAM available");
  }
  
  Serial.println("=============================");
}

// Estimate memory savings from compression
void printCompressionBenefits() {
  Serial.println("=== HTML Compression Benefits ===");
  
  // Estimated sizes (you can measure these during compilation)
  const size_t uncompressed_html_size = 8500;  // Estimated original HTML size
  const size_t compressed_html_size = 3200;    // Estimated compressed size
  const size_t css_reduction = 2800;           // CSS minification savings
  const size_t js_reduction = 1500;            // JS minification savings
  
  size_t total_savings = uncompressed_html_size - compressed_html_size;
  float compression_ratio = (float)total_savings / uncompressed_html_size * 100;
  
  Serial.printf("Original HTML size: ~%d bytes\n", uncompressed_html_size);
  Serial.printf("Compressed size: ~%d bytes\n", compressed_html_size);
  Serial.printf("Memory saved: %d bytes (%.1f%%)\n", total_savings, compression_ratio);
  Serial.printf("CSS minification: ~%d bytes saved\n", css_reduction);
  Serial.printf("JS minification: ~%d bytes saved\n", js_reduction);
  Serial.println("=================================");
}

// Call this during setup to see memory impact
void analyzeMemoryOnStartup() {
  Serial.println("\n=== Startup Memory Analysis ===");
  printMemoryUsage();
  printCompressionBenefits();
}

#endif // MEMORY_ANALYSIS_H