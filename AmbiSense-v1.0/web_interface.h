#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

/**
 * Initialize the web server
 */
void setupWebServer();

/**
 * Process web server requests
 * Should be called regularly in the main loop
 */
void handleWebServer();

#endif // WEB_INTERFACE_H