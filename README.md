# AmbiSense - Radar-Controlled LED System
<p align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/Assets/AmbiSense.webp" width="300" alt="AmbiSense Logo">
</p>

> ### 🚀 v6.0.0 shipped — full ESP-IDF + FreeRTOS rewrite
>
> v6 is a ground-up rewrite from Arduino onto **ESP-IDF + FreeRTOS**: independent
> FreeRTOS tasks for radar read, motion smoothing, LED rendering, web serving,
> and ESP-NOW peer-mesh — no more cooperative `loop()` starving the LED
> render under HTTP load. v6 ships with **LD2450 multi-target tracking**, a
> **modular radar driver layer** (LD2410 / LD2412 / LD2420 / LD2450 / sim —
> swap via web UI without reflashing), a **board picker with editable pin map**
> (ESP32-C3 / ESP32 / S3 / C6), a **peer mesh** for U/L/asymmetric stairs
> (every device runs identical code; lowest-MAC wins coordinator),
> **asymmetric pairing** (click Pair on either device → both join), a
> **Kalman-based motion filter** (3 user knobs instead of v5's 5 cryptic
> gains), **OTA with rollback**, **captive portal**, **PBKDF2-SHA256 auth**,
> and a **fully responsive Preact web UI** (sidebar on desktop, bottom-tab
> nav on mobile).
>
> **Recommended hardware: ESP32-S3 (DevKitC-1 or S3-Zero).** ESP32-C3 SuperMini
> is supported for single-strip installs. See
> [docs/HARDWARE.md](docs/HARDWARE.md) for the full board recommendation
> table and known issues.
>
> | Branch | Purpose |
> |---|---|
> | `main` | v5.1.1 Arduino release — stable |
> | `v6-idf-rewrite` | v6 development branch (current) |
> | `legacy/v5-arduino` | frozen archive of the v5.x Arduino line |
> | tag `v6.0.0` | v6 release |
> | tag `v5.1.1` | last Arduino-era release |

## v6 quickstart (ESP-IDF, requires `v5.3` LTS)

```sh
# One-time
git clone https://github.com/Techposts/AmbiSense.git
cd AmbiSense/firmware
. $IDF_PATH/export.sh

# Build & flash for your board
idf.py set-target esp32c3       # or: esp32, esp32s3, esp32c6
idf.py build flash monitor
```

The skeleton boots, initialises NVS, resolves the board profile, and drives
the onboard status LED into AP-mode blink (waiting-for-Wi-Fi). Wi-Fi setup
lands in PR #2.

VSCode users: install Espressif's ESP-IDF extension to get IntelliSense for
IDF headers — without it, clangd will report `'esp_err.h' file not found`
and similar; the code still builds correctly via `idf.py`.

## v6 documentation

| Document | Read when |
|---|---|
| [docs/V6-ARCHITECTURE.md](docs/V6-ARCHITECTURE.md) | Picking up the v6 rewrite cold — this captures the locked architectural decisions (peer mesh, modular radar drivers, NVS schema, board profiles). Read first before proposing any structural change. |
| [docs/V6-ROADMAP.md](docs/V6-ROADMAP.md) | Planning what to build next — the 5-PR plan with status per PR and the tag/release cadence. |
| [docs/HARDWARE.md](docs/HARDWARE.md) | Building or debugging hardware — reference wiring for C3 SuperMini, board profiles, sensor pinouts, and a flash-fails-to-connect troubleshooting ladder. |

## Repo layout

```
firmware/                — ESP-IDF v6 source (this is where new work goes)
frontend/design-source/  — Claude-Design handoff bundle for the v6 UI (read frontend/design-source/README.md)
legacy/AmbiSense/        — v5.x Arduino source, preserved for reference
Assets/, STL Files/      — design assets, enclosures (unchanged)
```

---

## v5 (Arduino) docs

The legacy v5 documentation below applies to `main` until v6 takes over.
For new development, target the `v6-idf-rewrite` branch.

---

AmbiSense is an innovative smart lighting solution that uses radar sensing technology to create responsive ambient lighting experiences. The system detects movement and distance using an LD2410 radar sensor and dynamically controls NeoPixel LED strips in real-time, creating an interactive lighting environment.

The core of AmbiSense is built around an ESP32 microcontroller that interfaces with an LD2410 radar module and NeoPixel LED strips. The system creates a moving light pattern that responds to a person's proximity, with the illuminated section of the LED strip changing based on detected distance.

We also developed a [Custom Home Assistant Integration](https://github.com/Techposts/ambisense-homeassistant) allowing you to integrate and control the AmbiSense from the Home Assistant and also run powerful automations.
<p align="center">
  <img src="https://i.imgur.com/6YUbPAu.png" width="500" alt="Custom HA Integration">
</p> 

# AmbiSense: Radar-Controlled LED System
<p align="center">
  <a href="https://www.youtube.com/watch?v=1fmlwl2iujk">
    <img src="https://img.youtube.com/vi/1fmlwl2iujk/0.jpg" alt="AmbiSense v4.1 Release - Intelligent DIY Motion-Tracking Lights That Illuminate Path Dynamically" width="600">
  </a>
  <br>
  <em>Click the image above to watch the AmbiSense v4.1 Release video</em>
</p>

<p align="center">
  <a href="https://www.youtube.com/watch?v=_xYEh8xkq1c">
    <img src="https://img.youtube.com/vi/_xYEh8xkq1c/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
  </a>
  <br>
  <em>Click the image above to watch the video Guide</em>
</p>

<p align="center">
  <a href="https://www.youtube.com/watch?v=AcjSumdNSIs">
    <img src="https://img.youtube.com/vi/AcjSumdNSIs/0.jpg" alt="AmbiSense - Radar-Controlled LED System" width="600">
  </a>
  <br>
  <em>Click the image above to watch the demo video</em>
</p>

---

## 📋 Table of Contents

- [Version History](#-version-history)
- [What's New in v5.1](#-whats-new-in-v43)
- [Key Features](#key-features)
- [Hardware Requirements](#hardware-requirements)
- [Circuit Connections](#circuit-connections)
- [Software Setup](#software-setup)
- [Multi-Sensor Setup Guide](#-multi-sensor-setup-guide)
- [Web Interface Features](#web-interface-features)
- [Physical Controls](#physical-controls)
- [Use Cases](#use-cases)
- [Technical Details](#technical-details)

---

## 📚 Version History

### **Current Version: v5.1** *(Latest)*
**Released:** June 2025  
**Status:** Stable Release  

<p align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/Assets/AmbiSense-Mesh.jpg" width="300" alt="AmbiSense Logo">
</p>

<p align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/Assets/AmbISense-mesh-2.jpg" width="300" alt="AmbiSense Logo">
</p>

#### 🚀 What's New in v5.1
- **🔗 Multi-Sensor ESP-NOW Support**: Connect multiple AmbiSense devices for complex layouts
- **📊 Enhanced Diagnostics**: Real-time monitoring and troubleshooting tools
- **🛠️ Critical Bug Fixes**: Resolved compilation and stability issues
- **🌐 Improved Network Management**: Better WiFi handling and connection reliability
- **🎨 Advanced LED Features**: Enhanced background mode and directional trails

#### Previous Versions
- **v4.1** *(April 2024)*: Added expanded light effects, motion smoothing, Home Assistant compatibility
- **v4.0** *(Initial Release)*: Core radar-controlled LED functionality

> **💡 Upgrade Recommendation**: If you're using v4.1 or earlier, upgrading to v5.1 is strongly recommended for bug fixes and new multi-sensor capabilities.

---

## 🆕 What's New in v5.1

### 🔗 Multi-Sensor ESP-NOW Support
AmbiSense v5.1 introduces support for multiple devices working together using ESP-NOW wireless communication. Perfect for:

- **L-shaped Staircases**: Place sensors at each turn for seamless lighting transitions
- **U-shaped Staircases**: Multiple sensors ensure complete coverage without dead zones
- **Long Hallways**: Distribute LED strips across multiple devices for extended coverage
- **Complex Layouts**: Handle any architectural configuration with intelligent sensor switching

<p align="center">
  <img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/multi-sensor-diagram.png" width="600" alt="Multi-Sensor Setup Diagram">
  <br>
  <em>Example: L-shaped staircase with master and slave sensors working together</em>
</p>

#### 🎯 Sensor Priority Modes
Choose how your multi-sensor system prioritizes readings:

- **🧠 Zone-Based** *(Recommended)*: Intelligent switching perfect for L-shaped layouts
- **⏱️ Most Recent**: Uses whichever sensor detected motion most recently
- **🔝 Slave First**: Prioritizes slave sensors over master for upper-level priority
- **🏠 Master First**: Prioritizes master sensor for main-area control

### 📊 Enhanced Diagnostics & Monitoring

The new **Diagnostics Tab** provides comprehensive system insights:

- **📡 Real-time Sensor Data**: Live readings from all connected devices
- **💚 Connection Health**: Monitor ESP-NOW signal strength and packet success rates
- **🧠 System Performance**: Track memory usage, uptime, and processing efficiency
- **🗺️ Network Topology**: Visual representation of your multi-sensor setup
- **🔧 Troubleshooting Tools**: Identify and resolve connectivity issues instantly

<p align="center">
  <img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/diagnostics-interface.png" width="400" alt="Diagnostics Interface">
  <br>
  <em>Real-time diagnostics showing multi-sensor network status</em>
</p>

### 🛠️ Critical Improvements

- **✅ Fixed Compilation Issues**: Resolved linker errors that prevented building from source
- **🧠 Better Memory Management**: Improved handling of different LED strip configurations
- **📡 ESP-NOW Stability**: Enhanced wireless communication reliability
- **💾 EEPROM Validation**: Robust settings storage with automatic corruption recovery

---

## Key Features

### 🎯 Core Functionality
- **Radar-Based Motion Detection**: Uses the LD2410 24GHz radar module for accurate presence and distance sensing without privacy concerns of cameras
- **Dynamic LED Control**: Creates moving light patterns that respond to user proximity
- **🌈 Multiple Lighting Modes**: Choose from 10+ effects including Standard, Rainbow, Color Wave, Breathing, Fire, Theater Chase, and more
- **🎨 Directional Light Trails**: Adds trailing effects that follow movement direction with customizable trail length
- **🌙 Background Lighting**: Optional ambient background illumination when no motion is detected
- **📍 Center Shift Adjustment**: Reposition the active LED zone relative to detected position

### 🌐 Advanced Connectivity *(New in v5.1)*
- **🔗 Multi-Sensor Networks**: Connect up to 5 slave devices to one master for complex layouts
- **📡 ESP-NOW Communication**: Low-latency wireless coordination between devices
- **🎛️ Distributed LED Control**: Split long LED strips across multiple devices
- **🧠 Intelligent Sensor Fusion**: Smart algorithms combine data from multiple sensors

### 💻 Web Interface & Management
- **📱 Modern Web Interface**: Intuitive tab-based configuration with responsive design
- **📊 Real-Time Visualization**: Live distance detection and LED response preview
- **🌐 WiFi Network Management**: 
  - Connect to existing networks or create access point
  - Scan for available networks with signal strength indicators
  - mDNS support for easy device discovery (access via `http://ambisense-[name].local`)
- **💾 Persistent Settings**: All configurations saved to EEPROM with CRC validation

### 🏠 Smart Home Integration
- **🏡 Home Assistant Ready**: Full compatibility with our [custom integration](https://github.com/Techposts/ambisense-homeassistant)
- **📊 Enhanced API**: Improved endpoints for external automation systems
- **🔄 Multi-Device Support**: Complete support for master-slave configurations

---

## Hardware Requirements

### Essential Components
- **ESP32 development board** (recommended: ESP32-C3 SuperMini)
- **LD2410 radar sensor module** (24GHz frequency)
- **WS2812B (NeoPixel) compatible LED strip**
- **5V power supply** (adequate for your LED strip length)
- **Connecting wires** and breadboard/PCB for prototyping

### Multi-Sensor Setup *(Additional)*
- **Additional ESP32 + LD2410 modules** (up to 5 slaves per master)
- **Individual power supplies** for each sensor location
- **Strategic placement** at turns, landings, or coverage gaps

---

## Circuit Connections

### ESP32-C3 SuperMini to LD2410 Radar

| ESP32-C3 Pin | LD2410 Pin | Function             |
|--------------|------------|----------------------|
| GPIO3 (RX)   | TX         | Serial Communication |
| GPIO4 (TX)   | RX         | Serial Communication |
| 5V           | VCC        | Power Supply         |
| GND          | GND        | Ground               |

### ESP32-C3 SuperMini to WS2812B LED Strip

| ESP32-C3 Pin | WS2812B Pin | Function    |
|--------------|-------------|-------------|
| GPIO5        | DIN         | Data Signal |
| 5V           | VCC         | Power Supply|
| GND          | GND         | Ground      |

### Power Supply Connections

| Power Supply | ESP32-C3 Pin | Function        |
|--------------|--------------|-----------------|
| +5V          | 5V           | Positive Supply |
| GND          | GND          | Ground          |

<p align="center">
  <img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/circuit-diagram.svg" width="800" alt="LD2410C to ESP32C3 SuperMini Circuit Diagram">
  <br>
  <em>Complete wiring diagram for single AmbiSense device</em>
</p>

<p align="center">
  <img src="https://github.com/Techposts/AmbiSense/blob/main/Assets/TTP%20Touch%20sensor.png" width="800" alt="TTP TouchSensor Short A to GND">
  <br>
  <em>Short the A to GND on the Touch Sensor</em>
</p>

> **⚠️ Power Supply Notes:**
> - For LED strips longer than 30 LEDs, connect 5V power supply directly to WS2812B VCC
> - Calculate power needs: ~60mA per LED at full brightness
> - Use 5V 2A+ power supply for strips with 50+ LEDs
> - Ensure adequate current capacity for your specific LED count

---

## Software Setup

### Option 1: Using Arduino IDE *(Recommended for Customization)*

1.  **Download the Code**
    ```bash
    git clone [https://github.com/Techposts/AmbiSense.git](https://github.com/Techposts/AmbiSense.git)
    ```
    Or download the ZIP file and extract.
2.  **Install Arduino IDE Libraries**
    Go to `Sketch > Include Library > Manage Libraries`
    Install these libraries:
    - Adafruit NeoPixel (LED control)
    - ArduinoJson (Configuration handling)
    - WebServer (ESP32 built-in web server)
    - WiFi (ESP32 built-in WiFi)
    - LD2410 (Radar sensor communication)
3.  **Configure Arduino IDE**
    - Board: `Tools > Board > ESP32 > ESP32C3 Dev Module`
    - Flash Size: `4MB`
    - Partition Scheme: `Default 4MB with spiffs`
    - Upload Speed: `921600`
4.  **Upload and Configure**
    - Select correct COM port
    - Click `Upload`
    - Connect to "AmbiSense" WiFi (password: `12345678`)
    - Navigate to `http://192.168.4.1`

### Option 2: Pre-compiled Binaries *(Recommended for Quick Setup)*

#### Using ESP Flash Download Tool
1.  **Download Tools**
    - ESP Flash Download Tool
    - Latest AmbiSense Binaries
2.  **Configure Flash Tool**
    - Chip Type: `ESP32-C3 RISC-V`
    - SPI Speed: `80MHz`
    - SPI Mode: `DIO`
    - Flash Size: `4MB`
3.  **Add Binary Files with these addresses:**
    ```
    AmbiSense-ESP32C3-v5.1-bootloader.bin    → 0x0
    AmbiSense-ESP32C3-v5.1-partitions.bin    → 0x8000
    AmbiSense-ESP32C3-v5.1.bin               → 0x10000
    ```
4.  **Flash Device**
    - Select COM port and set baud to `921600`
    - Click `START` and wait for `FINISH`

#### Using esptool (Command Line)
```bash
# Install esptool
pip install esptool

# Flash firmware (replace COM3 with your port)
esptool.exe --chip esp32c3 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 4MB \
  0x0 AmbiSense-ESP32C3-v5.1-bootloader.bin \
  0x8000 AmbiSense-ESP32C3-v5.1-partitions.bin \
  0x10000 AmbiSense-ESP32C3-v5.1.bin
```

# Troubleshooting Multi-Sensor Issues

**Connection Problems:**
* Check all devices are on the same WiFi channel
* Verify MAC addresses are entered correctly
* Use Diagnostics tab to monitor connection health
* Ensure devices are within ESP-NOW range (~100m line of sight)

**LED Synchronization Issues:**
* Verify master device has adequate power supply
* Check LED distribution settings match physical setup
* Monitor packet loss in diagnostics
* Consider reducing number of connected slaves

# Web Interface Features

## 🏠 Basic Tab
* **LED Count Configuration**: Set total number of LEDs (1-2000)
* **Distance Range Settings**: Customize min/max detection distances
* **🎨 RGB Color Selection**: Intuitive color picker with live preview
* **💡 Brightness Control**: Global brightness adjustment (0-255)
* **🔄 Quick Reset**: Reset distance values to factory defaults

## ⚙️ Advanced Tab
* **🧠 Motion Smoothing**: Enable/disable intelligent motion tracking
* **📍 Position Controls**: Fine-tune position smoothing and prediction
* **🎯 Center Shift**: Adjust LED positioning relative to detected motion
* **✨ Trail Effects**: Configure directional light trails
* **🌙 Background Mode**: Set ambient lighting when no motion detected
* **🎚️ Control Sensitivity**: Adjust motion tracking parameters

## 🎆 Effects Tab
* **🌈 Light Mode Selection**: Choose from 10+ visual effects:
    * Standard (motion tracking)
    * Rainbow (flowing spectrum)
    * Color Wave (rippling colors)
    * Breathing (fade in/out)
    * Fire Effect (realistic flames)
    * Theater Chase (marquee lights)
    * And more!
* **⚡ Effect Speed**: Control animation speed (1-100)
* **🔥 Effect Intensity**: Adjust effect strength and brightness

## 🔗 Multi-Sensor Tab (New in v5.1)
* **👑 Device Role Selection**: Configure as Master or Slave
* **🔍 Device Discovery**: Automatic scanning for nearby AmbiSense devices
* **📡 Network Management**: Add/remove slave devices
* **🎯 Priority Mode Selection**: Choose sensor prioritization strategy
* **📊 LED Distribution**: Configure distributed LED control across devices
* **🩺 Connection Health**: Monitor ESP-NOW network status

## 📊 Diagnostics Tab (New in v5.1)
* **📡 Live Sensor Data**: Real-time readings from all connected sensors
* **💚 Connection Status**: ESP-NOW health and signal strength monitoring
* **📈 Performance Metrics**: Memory usage, packet statistics, system uptime
* **🗺️ Network Topology**: Visual representation of your sensor network
* **🔧 Troubleshooting**: Tools for identifying and resolving issues

## 🌐 Network Tab
* **📶 WiFi Scanning**: Discover and connect to available networks
* **🏷️ Device Naming**: Set custom mDNS hostname for easy access
* **📊 Status Monitoring**: Real-time connection status and IP information
* **🔄 Network Reset**: Reset WiFi settings to factory defaults

# Physical Controls

AmbiSense includes intuitive physical controls for convenient operation:

* **🔘 Main Control Button (GPIO7)**
    * **Short Press (< 2 seconds)**: Toggle System ON/OFF
        * Instantly enables/disables all lighting effects
        * Useful for quick control without web interface access
    * **Long Press (≥ 10 seconds)**: Factory WiFi Reset
        * Clears all saved WiFi credentials
        * Restarts device in Access Point mode
        * LED indicator confirms reset completion
* **💡 LED Status Indicators**
    * **Solid Blue**: System active and functioning normally
    * **Blinking Blue**: Motion detected and tracking
    * **Red Flash**: WiFi reset initiated
    * **No Light**: System disabled or error state

> **💡 Pro Tip**: The physical button provides essential backup control when web interface is inaccessible due to network issues.

# 3D Printed Case

Professional enclosure designs for your AmbiSense installation:

<div align="center">
  <img src="https://raw.githubusercontent.com/Techposts/AmbiSense/refs/heads/main/AmbiSense.jpg" width="600" alt="AmbiSense 3D Case">
  <p><em>Custom 3D printed case housing ESP32 and LD2410 radar module</em></p>
</div>

* **📁 Available Files**
    * **Main Enclosure**: Houses ESP32-C3 and LD2410 radar sensor
    * **Mounting Bracket**: Wall/ceiling mount options
    * **Cable Management**: Organized wire routing solutions
    * **Download STL files**: [STL Files Folder](https://github.com/Techposts/AmbiSense/tree/main/STL%20Files) * **🖨️ Printing Recommendations**
    * **Layer Height**: 0.2mm for optimal detail
    * **Infill**: 20% for structural strength
    * **Support**: Required for mounting bracket overhangs
    * **Material**: PLA or PETG for indoor use

# Use Cases

## 🏠 Residential Applications
* **🚶 Smart Staircases**: Automatic illumination following your path up/down stairs
* **🏃 Hallway Lighting**: Responsive corridor lighting that activates as you approach
* **🛏️ Bedroom Ambiance**: Gentle nighttime navigation lighting
* **🎭 Home Theater**: Dynamic bias lighting that responds to viewer movement
* **♿ Accessibility Aid**: Assistive lighting for visually impaired navigation

## 🏢 Commercial & Creative Uses
* **🎨 Interactive Art**: Installations that respond to viewer proximity and movement
* **🏪 Retail Displays**: Eye-catching product showcases with motion-responsive lighting
* **🏛️ Museum Exhibits**: Engaging displays that activate as visitors approach
* **🎪 Event Lighting**: Dynamic stage or venue lighting that follows performers
* **🏥 Healthcare Facilities**: Gentle wayfinding assistance in medical environments

## 🌱 Energy Efficiency Benefits
* **💚 Motion-Only Activation**: Lights only when needed, reducing energy waste
* **⏰ Automatic Shutoff**: Configurable timeout when no motion detected
* **🎯 Targeted Illumination**: Only lights necessary areas, not entire spaces
* **📊 Usage Analytics**: Monitor activation patterns for optimization

# Technical Details

## 🔧 Hardware Specifications
* **Microcontroller**: ESP32-C3 (RISC-V, 160MHz, WiFi + Bluetooth)
* **Radar Sensor**: LD2410 (24GHz, 0-6m range, ±60° detection angle)
* **LED Support**: WS2812B/NeoPixel compatible strips (up to 2000 LEDs)
* **Communication**: ESP-NOW for multi-sensor coordination
* **Power**: 5V DC input, automatic LED power management

## 💻 Software Architecture
* **Framework**: Arduino Core for ESP32
* **Web Server**: Built-in ESP32 WebServer (no external dependencies)
* **Data Storage**: EEPROM with CRC validation and corruption recovery
* **Communication Protocols**:
    * UART for LD2410 radar communication
    * ESP-NOW for inter-device coordination
    * WiFi for web interface and network connectivity

## 📡 Network Capabilities
* **WiFi Modes**: Station (client) and Access Point modes
* **mDNS Support**: Easy device discovery via `hostname.local`
* **ESP-NOW Range**: Up to 100 meters line-of-sight between devices
* **Network Security**: WPA2 WiFi encryption, no external cloud dependencies

## 🔒 Security & Privacy
* **Local Processing**: All motion detection handled on-device
* **No Cloud Dependencies**: Complete offline operation capability
* **Privacy-First Design**: Radar sensing provides presence detection without cameras
* **Secure Communication**: Encrypted ESP-NOW and WiFi protocols

## ⚡ Performance Metrics
* **Response Time**: <50ms motion detection to LED update
* **Update Rate**: 20Hz radar sensor polling
* **ESP-NOW Latency**: <10ms between master and slave devices
* **Memory Usage**: Optimized for 4MB flash, <80KB RAM usage
* **Power Consumption**: <500mA at 5V with 30 LEDs active

# Troubleshooting & Support

## 🔧 Common Issues
* **Compilation Errors**:
    * Ensure you're using AmbiSense v5.1 or later
    * Verify all required libraries are installed
    * Check ESP32 board package is up to date
* **WiFi Connection Problems**:
    * Use physical button to reset WiFi settings
    * Check network password and signal strength
    * Try manual configuration via Access Point mode
* **Multi-Sensor Connectivity**:
    * Verify all devices are running the same firmware version
    * Check ESP-NOW range limitations (~100m)
    * Use Diagnostics tab to monitor connection health

## 📞 Getting Help
* **Issues & Bugs**: [GitHub Issues](https://github.com/Techposts/AmbiSense/issues) * **Discussions**: [GitHub Discussions](https://github.com/Techposts/AmbiSense/discussions) * **Documentation**: [Wiki](https://github.com/Techposts/AmbiSense/wiki) # 📜 License & Credits
* **Created by**: Ravi Singh for TechPosts Media
* **Copyright**: © 2025 TechPosts Media. All rights reserved.
* **License**: MIT License - see `LICENSE` file for details

## 🙏 Acknowledgments
* Espressif Systems: ESP32 platform and development tools
* Adafruit: NeoPixel library and hardware ecosystem
* HiLink: LD2410 radar sensor technology
* Community Contributors: Bug reports, feature suggestions, and feedback

<div align="center">
  <strong> ⭐ If you find AmbiSense useful, please star this repository! ⭐ </strong>
  <br><br>
  <a href="https://github.com/Techposts/AmbiSense/stargazers">
    <img src="https://img.shields.io/github/stars/Techposts/AmbiSense?style=social" alt="GitHub Stars">
  </a>
  <a href="https://github.com/Techposts/AmbiSense/network/members">
    <img src="https://img.shields.io/github/forks/Techposts/AmbiSense?style=social" alt="GitHub Forks">
  </a>
  <a href="https://github.com/Techposts/AmbiSense/issues">
    <img src="https://img.shields.io/github/issues/Techposts/AmbiSense" alt="GitHub Issues">
  </a>
</div>

---

