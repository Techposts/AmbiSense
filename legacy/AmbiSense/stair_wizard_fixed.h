#ifndef STAIR_WIZARD_FIXED_H
#define STAIR_WIZARD_FIXED_H

/*
 * AmbiSense Stair Configuration Wizard - FIXED VERSION
 * Addresses critical compilation, memory, and integration issues
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>

// Forward declarations to avoid circular dependencies
class WebServer;

// Stair layout types
enum StairLayout {
  LAYOUT_STRAIGHT = 0,
  LAYOUT_L_SHAPED = 1,
  LAYOUT_U_SHAPED = 2,
  LAYOUT_CUSTOM = 3
};

// Device position in stair layout
enum DevicePosition {
  POSITION_BOTTOM = 0,
  POSITION_CORNER = 1, 
  POSITION_TOP = 2,
  POSITION_MIDDLE = 3
};

// Simplified wizard state with memory management
struct SimpleWizardState {
  StairLayout selectedLayout;
  int totalLeds;
  int discoveredDeviceCount;
  unsigned long sessionStart;
  bool isActive;
  
  // Constructor
  SimpleWizardState() : selectedLayout(LAYOUT_STRAIGHT), totalLeds(300), 
                       discoveredDeviceCount(0), sessionStart(0), isActive(false) {}
};

// Global wizard state
extern SimpleWizardState wizardState;

/**
 * Initialize the wizard system
 */
void initWizard();

/**
 * Generate wizard start page (memory-safe version)
 */
String generateWizardStartPage();

/**
 * Process layout selection step
 */
String processLayoutStep(const String& layout);

/**
 * Handle device discovery with proper error handling
 */
String handleDeviceDiscovery();

/**
 * Generate device discovery results JSON
 */
String generateDiscoveryResults();

/**
 * Apply simplified configuration based on layout
 */
bool applyLayoutConfiguration(StairLayout layout, int deviceCount);

/**
 * Validate wizard state and configuration
 */
bool validateWizardConfiguration();

/**
 * Reset wizard state (cleanup)
 */
void resetWizardState();

/**
 * Check if wizard session is valid (timeout protection)
 */
bool isWizardSessionValid();

/**
 * Generate error response JSON
 */
String generateErrorResponse(const String& message);

/**
 * Generate wizard error page HTML (instead of JSON for better UX)
 */
String generateWizardErrorPage(const String& message);

/**
 * Generate success response JSON
 */
String generateSuccessResponse(const String& message);

/**
 * Memory-safe string builder for large HTML
 */
class SafeStringBuilder {
private:
  String content;
  size_t maxSize;
  bool overflowed;
  
public:
  SafeStringBuilder(size_t max = 8192) : maxSize(max), overflowed(false) {}
  
  bool append(const String& str) {
    if (overflowed || (content.length() + str.length()) > maxSize) {
      overflowed = true;
      return false;
    }
    content += str;
    return true;
  }
  
  String build() { return overflowed ? String("") : content; }
  bool hasOverflowed() { return overflowed; }
  size_t length() { return content.length(); }
};

#endif // STAIR_WIZARD_FIXED_H