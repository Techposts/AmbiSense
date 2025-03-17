# AmbiSense - Radar-Controlled LED System

## Project Description

AmbiSense is an innovative smart lighting solution that uses radar sensing technology to create responsive ambient lighting experiences. The system detects movement and distance using an LD2410 radar sensor and dynamically controls NeoPixel LED strips in real-time, creating an interactive lighting environment.

The core of AmbiSense is built around an ESP32 microcontroller that interfaces with an LD2410 radar module and NeoPixel LED strips. The system creates a moving light pattern that responds to a person's proximity, with the illuminated section of the LED strip changing based on detected distance.

## Key Features

- **Radar-Based Motion Detection**: Uses the LD2410 24GHz radar module for accurate presence and distance sensing without privacy concerns of cameras
- **Dynamic LED Control**: Creates moving light patterns that respond to user proximity
- **Customizable Settings**: Configure colors, brightness, sensitivity, and behavior through an intuitive web interface
- **Wi-Fi Configuration**: Connect to the device's access point to adjust all parameters without reprogramming
- **Distance-Based Interaction**: Light patterns change based on detected distance between minimum and maximum thresholds
- **Persistent Settings**: All configurations are saved to EEPROM and retained after power cycles
- **Real-Time Feedback**: Web interface displays current detected distance and provides visual feedback

## Hardware Requirements

- ESP32 development board
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

1. Upload the provided code to your ESP32 using the Arduino IDE
2. Power on the device
3. Connect to the WiFi network "AmbiSense" (password: 12345678)
4. Navigate to http://192.168.4.1 in your browser
<div align="left">
  <img src="https://i.imgur.com/qJ7NrTp.png" alt="AmbiSense - DIY Smart Staircase and Hallway Lighting: Light That Moves With You" width="300"/>
</div>
6. Use the web interface to configure:
   - Number of LEDs
   - Minimum/maximum detection distance
   - LED color
   - Brightness
   - Moving light span (Number of lEDs that will run along)

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

4. [Download](https://github.com/Techposts/AmbiSense/releases/tag/V1.0) and add the binary files with these specific addresses:
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

## Use Cases

- **Smart Home Lighting**: Hallways and staircases that illuminate as you approach
- **Ambient Room Lighting**: Dynamic mood lighting that responds to movement
- **Interactive Installations**: Art exhibits or displays that respond to viewers' proximity
- **Energy-Efficient Lighting**: Lights that activate only when needed based on presence detection
- **Accessibility Applications**: Assistive lighting for navigation in dark spaces

## Technical Details

- Built on ESP32 platform with Arduino framework
- Communicates with LD2410 radar sensor via UART
- Controls WS2812B LED strips using the Adafruit NeoPixel library
- Creates a Wi-Fi access point with a modern web interface for configuration
- Settings persistence through ESP32's EEPROM
- Real-time distance visualization in the web dashboard

This project was created by Ravi Singh for TechPosts Media.
