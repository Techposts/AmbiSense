# AmbiSense - Radar-Controlled LED System

<div align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/Assets/AmbiSense.webp" width="300" alt="AmbiSense Logo">
  <br>
  <h3>Smart Lighting with Radar Motion Detection</h3>
  <p>Created by <a href="https://github.com/Techposts">Ravi Singh</a> (TechPosts Media)</p>
</div>

## 🌟 Overview

AmbiSense is an innovative smart lighting solution that uses radar sensing technology to create responsive ambient lighting experiences. The system detects movement and distance using an LD2410 radar sensor and dynamically controls NeoPixel LED strips in real-time, creating an interactive lighting environment.

Unlike traditional motion sensors, AmbiSense's radar technology:
- Works through walls and surfaces
- Detects both presence and precise distance
- Doesn't require line-of-sight
- Functions perfectly in darkness
- Has no privacy concerns (unlike camera-based solutions)

<div align="center">
  <a href="https://www.youtube.com/watch?v=_xYEh8xkq1c">
    <img src="https://img.youtube.com/vi/_xYEh8xkq1c/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
    <br>
    <i>Click to watch the video guide</i>
  </a>
</div>

<div align="center">
  <a href="https://www.youtube.com/watch?v=AcjSumdNSIs">
    <img src="https://img.youtube.com/vi/AcjSumdNSIs/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
    <br>
    <i>Click to watch the demo video</i>
  </a>
</div>

## ✨ Key Features

- **Radar-Based Motion Detection**: Uses the LD2410 24GHz radar module for accurate presence and distance sensing
- **Dynamic LED Control**: Creates moving light patterns that respond to user proximity
- **Multiple Lighting Modes**: Choose from 11 distinct lighting effects:
  - Standard (Moving Light)
  - Rainbow Effect
  - Color Wave
  - Breathing
  - Solid Color
  - Comet Trail
  - Pulse Waves
  - Fire Effect
  - Theater Chase
  - Dual Scanning
  - Motion Particles
- **Directional Light Trails**: Adds trailing effects that follow movement direction with customizable trail length
- **Background Lighting**: Optional ambient background illumination when no motion is detected
- **Center Shift Adjustment**: Reposition the active LED zone relative to detected position
- **Motion Smoothing**: Enhanced motion prediction and tracking with adjustable parameters
- **WiFi Network Management**:
  - Connect to existing networks or create access point
  - Scan for available networks from the web interface
  - mDNS support for easy device discovery (access via http://ambisense-[name].local)
- **Web Interface**: Configure all settings through an intuitive tab-based web interface with modern, responsive design
- **Home Assistant Integration**: Custom integration for smart home control
- **Persistent Settings**: All configurations are saved to EEPROM with CRC validation and retained after power cycles

## 🧰 Hardware Requirements

- ESP32 development board (recommended: ESP32-C3 SuperMini)
- LD2410 radar sensor module
- WS2812B (NeoPixel) compatible LED strip
- 5V power supply (adequate for your LED strip length)
- Connecting wires

## 📡 Circuit Connections

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

<div align="center">
  <img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/circuit-diagram.svg" width="800" alt="LD2410C to ESP32C3 SuperMini Circuit Diagram">
</div>

**Note**: For longer LED strips, it's recommended to connect the 5V power supply directly to the WS2812B VCC to avoid overloading the ESP32's voltage regulator. Always ensure adequate current capacity for your LED strip length.

## ⚡ Power Requirements

- Input: 5V DC via power jack
- Current: Depends on the number of LEDs (roughly calculate ~60mA per LED at full brightness)
- For strips longer than 30 LEDs, use a power supply rated for at least 2A or more. I used a 5V 2A for demo, works fine for up to 50 LEDs

## 🔧 Software Setup

### Option 1: Using Arduino IDE

1. Clone this repository or download the source code
2. Open the `AmbiSense-v4.1.ino` file in Arduino IDE
3. Install the required libraries:
   - Adafruit NeoPixel
   - LD2410 Library
4. Select the appropriate board (ESP32-C3)
5. Upload the code to your ESP32
6. Power on the device
7. Connect to the WiFi network "AmbiSense" (password: 12345678)
8. Navigate to http://192.168.4.1 in your browser
<div align="left">
  <img src="https://i.imgur.com/qJ7NrTp.png" alt="AmbiSense - DIY Smart Staircase and Hallway Lighting: Light That Moves With You" width="300"/>
</div>
9. Use the web interface to configure:
   - Number of LEDs
   - Minimum/maximum detection distance
   - LED color
   - Brightness
   - Moving light span (Number of LEDs that will run along)
   - Light mode
   - Directional light trails
   - Background lighting mode
   - Center shift adjustment
   - WiFi network settings

### Option 2: Flashing Pre-compiled Binaries

You can flash the pre-compiled binaries directly to your ESP32-C3 using one of the following methods:

#### Using ESP Flash Download Tool (Recommended for beginners)

1. Download ESP Flash Download Tool:
   - Download from the official Espressif website: [ESP Flash Download Tool](https://dl.espressif.com/public/flash_download_tool.zip)
   - Flash tool User guide: [Flash Download Tools (ESP8266 & ESP32) Guide](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html)
2. Extract and run the tool
3. Select "ESP32-C3 RISC-V" from the chip type dropdown
4. Configure the tool as follows:
   - Set SPI SPEED to "80MHz"
   - Set SPI MODE to "DIO"
   - Set FLASH SIZE to "4MB" (or match your ESP32-C3's flash size)
5. Download and add the binary files with these specific addresses:
   - Add `AmbiSense-ESP32C3-bootloader.bin` at address 0x0
   - Add `AmbiSense-ESP32C3-partitions.bin` at address 0x8000
   - Add `AmbiSense-ESP32C3.bin` at address 0x10000
6. Ensure all three checkboxes next to the file paths are checked
7. Select the appropriate COM port where your ESP32-C3 is connected
8. Set the baud rate to 921600 for faster flashing
9. Click the "START" button to begin flashing
10. Wait for the tool to display "FINISH" when the flashing is complete
11. Press the reset button on your ESP32-C3 board

![ESP Flash Download Tool Configuration](https://i.imgur.com/IujrnPc.png)
*(Example of ESP Flash Download Tool configuration - your file paths will be different)*

#### Using esptool.py (Alternative method)

1. Install esptool using pip (requires Python):
   ```
   pip install esptool
   ```
2. Open Command Prompt (cmd) and navigate to your binary files:
   ```
   cd C:\path\to\your\binaries
   ```
3. Flash your ESP32-C3 with this command (replace COM3 with your actual port):
   ```
   esptool.py --chip esp32c3 --port COM3 --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
   ```

##### Troubleshooting:
- If you have connection issues, try erasing the flash first:
  ```
  esptool.py --chip esp32c3 --port COM3 erase_flash
  ```
- If you still have issues, try a lower baud rate:
  ```
  esptool.py --chip esp32c3 --port COM3 --baud 115200 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
  ```
- For detailed logs, add the -v flag:
  ```
  esptool.py -v --chip esp32c3 --port COM3 --baud 921600 write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 AmbiSense-ESP32C3-bootloader.bin 0x8000 AmbiSense-ESP32C3-partitions.bin 0x10000 AmbiSense-ESP32C3.bin
  ```

### After Flashing

1. Power on the device
2. Connect to the WiFi network "AmbiSense" (password: 12345678)
3. Navigate to http://192.168.4.1 in your browser
<div align="left">
  <img src="https://i.imgur.com/qJ7NrTp.png" alt="AmbiSense - DIY Smart Staircase and Hallway Lighting: Light That Moves With You" width="300"/>
</div>
4. Use the web interface to configure your AmbiSense settings
5. Optionally connect to your home WiFi network via the Network tab
6. If connected to your home WiFi, access via http://ambisense-[name].local

## 🖥️ Web Interface Features

### Basic Tab
- Number of LEDs configuration
- Distance range settings
- RGB color selection with intuitive color picker
- Brightness control

### Advanced Tab
- Light mode selection (Standard, Rainbow, Color Wave, Breathing, Solid, Comet Trail, Pulse Waves, Fire Effect, Theater Chase, Dual Scanning, Motion Particles)
- Background lighting mode toggle
- Directional light trails toggle
- Center shift adjustment
- Trail length customization
- Motion smoothing settings with adjustable parameters

### Effects Tab
- Moving light span adjustment
- Effect speed control
- Effect intensity settings

### Network Tab
- WiFi network scanning and selection
- Device name configuration for mDNS
- Network status monitoring
- WiFi reset option

## 🏠 Home Assistant Integration

AmbiSense includes a [custom Home Assistant integration](https://github.com/Techposts/ambisense-homeassistant), allowing you to:
- Control lighting modes from Home Assistant
- Create automations based on motion detection
- Adjust brightness and colors remotely
- Integrate with other smart home devices and scenes

## 🎯 Use Cases

- **Smart Home Lighting**: Hallways and staircases that illuminate as you approach
- **Ambient Room Lighting**: Dynamic mood lighting that responds to movement
- **Interactive Installations**: Art exhibits or displays that respond to viewers' proximity
- **Energy-Efficient Lighting**: Lights that activate only when needed based on presence detection
- **Accessibility Applications**: Assistive lighting for navigation in dark spaces
- **Home Theater Ambiance**: Responsive bias lighting that changes with viewer movement
- **Creative RGB Effects**: Display colorful animations and patterns for decorative purposes

## 📚 Technical Details

- Built on ESP32 platform with Arduino framework
- Communicates with LD2410 radar sensor via UART
- Controls WS2812B LED strips using the Adafruit NeoPixel library
- Creates a Wi-Fi access point with a modern web interface for configuration
- Settings persistence through ESP32's EEPROM with CRC validation
- Real-time distance visualization in the web dashboard
- Multiple animation mode

## 📁 Project Structure

- `AmbiSense-v4.1.ino`: Main Arduino sketch
- `config.h`: Global constants and configuration
- `eeprom_manager.cpp/h`: Handles saving/loading settings
- `led_controller.cpp/h`: Controls NeoPixel LED strip and lighting effects
- `radar_manager.cpp/h`: Handles LD2410 radar sensor and motion smoothing
- `web_interface.cpp/h`: Web server and UI implementation
- `wifi_manager.cpp/h`: WiFi access point setup and mDNS support

## 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📧 Contact

Created by Ravi Singh for TechPosts Media.  
Copyright © 2025 TechPosts Media. All rights reserved.

For questions or support, please open an issue on this repository.
