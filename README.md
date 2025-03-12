# AmbiSense: Radar-Controlled LED System

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
