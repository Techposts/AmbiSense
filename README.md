# AmbiSense - Radar-Controlled LED System
<div align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/Assets/AmbiSense.webp" width="300" alt="AmbiSense Logo">
</div>

AmbiSense is an innovative smart lighting solution that uses radar sensing technology to create responsive ambient lighting experiences. The system detects movement and distance using an LD2410 radar sensor and dynamically controls NeoPixel LED strips in real-time, creating an interactive lighting environment.

The core of AmbiSense is built around an ESP32 microcontroller that interfaces with an LD2410 radar module and NeoPixel LED strips. The system creates a moving light pattern that responds to a person's proximity, with the illuminated section of the LED strip changing based on detected distance.

We also developed a [Custom Home Assistant Integration](https://github.com/Techposts/ambisense-homeassistant) allowing you to integrate and control the AmbiSense from the Home Assistant and also run powerful automations. 
<div align="center">
  <img src="https://i.imgur.com/6YUbPAu.png" width="500" alt="Custom HA Integration">
</div>

# AmbiSense: Radar-Controlled LED System
<div align="center">
  <a href="https://www.youtube.com/watch?v=1fmlwl2iujk">
    <img src="https://img.youtube.com/vi/1fmlwl2iujk/0.jpg" alt="AmbiSense v4.1 Release - Intelligent DIY Motion-Tracking Lights That Illuminate Path Dynamically" width="600">
  </a>
  <p><em>Click the image above to watch the AmbiSense v4.1 Release video</em></p>
</div>

<div align="center">
  <a href="https://www.youtube.com/watch?v=_xYEh8xkq1c">
    <img src="https://img.youtube.com/vi/_xYEh8xkq1c/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
  </a>
  <p><em>Click the image above to watch the video Guide</em></p>
</div>

<div align="center">
  <a href="https://www.youtube.com/watch?v=AcjSumdNSIs">
    <img src="https://img.youtube.com/vi/AcjSumdNSIs/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
  </a>
  <p><em>Click the image above to watch the demo video</em></p>
</div>



## Key Features

- **Radar-Based Motion Detection**: Uses the LD2410 24GHz radar module for accurate presence and distance sensing without privacy concerns of cameras
- **Dynamic LED Control**: Creates moving light patterns that respond to user proximity
- **Multiple Lighting Modes**: Choose from Standard, Rainbow, Color Wave, Breathing, and Solid Color effects
- **Directional Light Trails**: Adds trailing effects that follow movement direction with customizable trail length
- **Background Lighting**: Optional ambient background illumination when no motion is detected
- **Center Shift Adjustment**: Reposition the active LED zone relative to detected position
- **Web Interface**: Configure all settings through an intuitive tab-based web interface with modern, responsive design
- **Real-Time Distance Visualization**: Web interface displays current detected distance and provides visual feedback
- **WiFi Network Management**: 
  - Connect to existing networks or create access point
  - Scan for available networks from the web interface
  - mDNS support for easy device discovery (access via http://ambisense-[name].local)
- **Persistent Settings**: All configurations are saved to EEPROM with CRC validation and retained after power cycles

## Hardware Requirements

- ESP32 development board (recommended: ESP32-C3)
- LD2410 radar sensor module
- WS2812B (NeoPixel) compatible LED strip
- 5V power supply (adequate for your LED strip length)
- Connecting wires

## Circuit Connections

### ESP32-C3 SuperMini to LD2410 Radar

- Connect ESP32 GPIO3 (RX) to LD2410 TX
- Connect ESP32 GPIO4 (TX) to LD2410 RX
- Connect ESP32 5V to LD2410 VCC
- Connect ESP32 GND to LD2410 GND

### ESP32-C3 SuperMini to WS2812B LED Strip

- Connect ESP32 GPIO5 to WS2812B DIN (Data In)
- Connect ESP32 5V to WS2812B VCC
- Connect ESP32 GND to WS2812B GND

### Power Supply

- Connect 5V Power Supply positive (+5V) to ESP32 5V pin
- Connect 5V Power Supply ground (GND) to ESP32 GND pin

<img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/circuit-diagram.svg" width="800" alt="LD2410C to ESP32C3 SuperMini Circuit Diagram">

> **Note:** For longer LED strips, it's recommended to connect the 5V power supply directly to the WS2812B VCC to avoid overloading the ESP32's voltage regulator. Always ensure adequate current capacity for your LED strip length.

## Power Requirements

- Input: 5V DC via power jack
- Current: Depends on the number of LEDs (roughly calculate ~60mA per LED at full brightness)
- For strips movespan longer than 30 LEDs, use a power supply rated for at least 2A or more. I used a 5V 2A for demo, works fine for upto 50 LEDs.

## Software Setup

### Option 1: Using Arduino IDE

1. Download the code from the [GitHub repository](https://github.com/Techposts/AmbiSense)
   ```
   git clone https://github.com/Techposts/AmbiSense.git
   ```
   Or download the ZIP file from the repository and extract it

2. Open the Arduino IDE

3. Install required libraries:
   - Go to Sketch > Include Library > Manage Libraries
   - Search for and install the following libraries:
     - Adafruit NeoPixel
     - ArduinoJson
     - ESPAsyncWebServer
     - AsyncTCP
     - LD2410

4. Open the AmbiSense.ino file in Arduino IDE

5. Select the correct board:
   - Go to Tools > Board > ESP32 > ESP32C3 Dev Module
   - Set Flash Size to "4MB"
   - Set Partition Scheme to "Default 4MB with spiffs"
   - Set Upload Speed to "921600"

6. Select the correct port:
   - Go to Tools > Port and select the COM port where your ESP32 is connected

7. Click the Upload button to compile and flash the code to your ESP32

8. Once the upload is complete, the ESP32 will restart automatically

9. Connect to the WiFi network "AmbiSense" (password: 12345678)

10. Navigate to http://192.168.4.1 in your browser
<div align="left">
  <img src="https://i.imgur.com/qJ7NrTp.png" alt="AmbiSense - DIY Smart Staircase and Hallway Lighting: Light That Moves With You" width="300"/>
</div>

11. Use the web interface to configure:
    - Number of LEDs
    - Minimum/maximum detection distance
    - LED color
    - Brightness
    - Moving light span (Number of LEDs that will run along)
    - Light mode (Standard, Rainbow, Color Wave, Breathing, Solid)
    - Directional light trails
    - Background lighting mode
    - Center shift adjustment
    - WiFi network settings

### Option 2: Flashing Pre-compiled Binaries

You can flash the pre-compiled binaries directly to your ESP32-C3 using one of the following methods:

#### Using ESP Flash Download Tool (Recommended for beginners)

The ESP Flash Download Tool provides a user-friendly GUI for flashing ESP devices.

1. Download ESP Flash Download Tool:
   - Download from the official Espressif website: [ESP Flash Download Tool](https://dl.espressif.com/public/flash_download_tool.zip)
   - Flash tool User guide: [Flash Download Tools (ESP8266 & ESP32) Guide](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html)

2. Extract and run the tool:
   - Extract the ZIP file
   - Run "flash_download_tool_x.x.x.exe" (where x.x.x is the version number)
   - Select "ESP32-C3 RISC-V" from the chip type dropdown

3. Configure the tool as follows:
   - Set SPI SPEED to "80MHz"
   - Set SPI MODE to "DIO"
   - Set FLASH SIZE to "4MB" (or match your ESP32-C3's flash size)

4. [Download](https://github.com/Techposts/AmbiSense/releases/latest) and add the binary files with these specific addresses:
   - Click [+] and add `AmbiSense-ESP32C3-bootloader.bin` at address 0x0
   - Click [+] and add `AmbiSense-ESP32C3-partitions.bin` at address 0x8000
   - Click [+] and add `AmbiSense-ESP32C3.bin` at address 0x10000
   - Ensure all three checkboxes next to the file paths are checked

5. Select the appropriate COM port where your ESP32-C3 is connected

6. Set the baud rate to 921600 for faster flashing

7. Click the "START" button to begin flashing

8. Wait for the tool to display "FINISH" when the flashing is complete

9. Press the reset button on your ESP32-C3 board

![ESP Flash Download Tool Configuration](https://i.imgur.com/IujrnPc.png)
*(Example of ESP Flash Download Tool configuration - your file paths will be different)*

#### Using esptool.exe (Alternative method)

esptool is a command-line utility for flashing ESP devices. Here's how to use it:

1. **Install esptool** using pip (requires Python):
   ```
   pip install esptool
   ```

2. **Open Command Prompt** (cmd) and navigate to your binary files:
   ```
   cd C:\path\to\your\binaries
   ```

3. **Flash your ESP32-C3** with this command (replace COM3 with your actual port):
   ```
   esptool.exe --chip esp32c3 --port COM3 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
   ```

4. **Troubleshooting**:
   - If you have connection issues, try erasing the flash first:
     ```
     esptool.exe --chip esp32c3 --port COM3 erase_flash
     ```
   - If you still have issues, try a lower baud rate:
     ```
     esptool.exe --chip esp32c3 --port COM3 --baud 115200 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
     ```
   - For detailed logs, add the `-v` flag:
     ```
     esptool.exe -v --chip esp32c3 --port COM3 --baud 921600 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
     ```

## After Flashing

1. Power on the device
2. Connect to the WiFi network "AmbiSense" (password: 12345678)
3. Navigate to http://192.168.4.1 in your browser
<div align="left">
  <img src="https://i.imgur.com/qJ7NrTp.png" alt="AmbiSense - DIY Smart Staircase and Hallway Lighting: Light That Moves With You" width="300"/>
</div>
4. Use the web interface to configure your AmbiSense settings
5. Optionally connect to your home WiFi network via the Network tab
6. If connected to your home WiFi, access via http://ambisense-[name].local

## Web Interface Features

### Basic Tab
- Number of LEDs configuration
- Distance range settings
- RGB color selection with intuitive color picker
- Brightness control

### Advanced Tab
- Light mode selection (Standard, Rainbow, Color Wave, Breathing, Solid)
- Background lighting mode toggle
- Directional light trails toggle
- Center shift adjustment
- Trail length customization

### Effects Tab
- Moving light span adjustment
- Effect speed control
- Effect intensity settings

### Network Tab
- WiFi network scanning and selection
- Device name configuration for mDNS
- Network status monitoring
- WiFi reset option

## Physical Controls

AmbiSense comes with a physical button for easy control:

- **Short Press** (less than 2 seconds): Toggles the system ON/OFF
- **Long Press** (10 seconds or more): Resets Wi-Fi settings and restarts the device in AP mode

This physical button is connected to GPIO pin 7 (defined as `WIFI_RESET_BUTTON_PIN` in the code). It provides a convenient way to control the system without needing to access the web interface, particularly useful for turning the lights on/off or resetting network settings if you're unable to connect to the device.

## 3D Printed Case

The repository includes STL files for 3D printing a custom case for your AmbiSense device. The case is designed to house the ESP32, LD2410 radar module, and associated components.

<div align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/AmbiSense.jpg" width="600" alt="AmbiSense 3D Case">
</div>

You can find the STL files in the [STL Files folder](https://github.com/Techposts/AmbiSense/tree/main/STL%20Files) of this repository.

## Use Cases

- **Smart Home Lighting**: Hallways and staircases that illuminate as you approach
- **Ambient Room Lighting**: Dynamic mood lighting that responds to movement
- **Interactive Installations**: Art exhibits or displays that respond to viewers' proximity
- **Energy-Efficient Lighting**: Lights that activate only when needed based on presence detection
- **Accessibility Applications**: Assistive lighting for navigation in dark spaces
- **Home Theater Ambiance**: Responsive bias lighting that changes with viewer movement
- **Creative RGB Effects**: Display colorful animations and patterns for decorative purposes

## Technical Details

- Built on ESP32 platform with Arduino framework
- Communicates with LD2410 radar sensor via UART
- Controls WS2812B LED strips using the Adafruit NeoPixel library
- Creates a Wi-Fi access point with a modern web interface for configuration
- Settings persistence through ESP32's EEPROM with CRC validation
- Real-time distance visualization in the web dashboard
- Multiple animation modes with customizable parameters
- WiFi management with mDNS support for easy access

This project was created by Ravi Singh for TechPosts Media.
Copyright © 2025 TechPosts Media. All rights reserved.
