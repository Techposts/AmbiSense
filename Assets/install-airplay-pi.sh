#!/bin/bash

# ===================================================================================
# Shairport-Sync AirPlay 2 ROBUST Installer
#
# Tailored for: Raspberry Pi (Zero/3/4/5) with one or more USB DACs
# Version: 2.0
# Features: Interactive setup, device auto-detection, user choices, error checking.
# ===================================================================================

# --- Configuration ---
# Exit immediately if a command exits with a non-zero status.
set -e

# --- Helper Functions for User Interface ---
cecho() {
    local code="\033["
    case "$1" in
        "red")    color="${code}1;31m" ;;
        "green")  color="${code}1;32m" ;;
        "yellow") color="${code}1;33m" ;;
        "blue")   color="${code}1;34m" ;;
        *)        color="${code}0m" ;; # Default
    esac
    local message="$2"
    local reset="${code}0m"
    echo -e "${color}${message}${reset}"
}

# --- Pre-flight Checks ---
pre_flight_checks() {
    cecho "blue" "Running pre-flight checks..."

    # Check for root user
    if [ "$EUID" -eq 0 ]; then
        cecho "red" "Error: This script should not be run as root. Please run it as a normal user."
        exit 1
    fi

    # Check for internet connection
    if ! ping -c 1 -W 3 google.com &> /dev/null; then
        cecho "red" "Error: No internet connection detected. Please connect to the internet and try again."
        exit 1
    fi
    cecho "green" "Checks passed."
}

# --- Main Script ---
clear
cecho "green" "======================================================"
cecho "green" "      Welcome to the Robust AirPlay 2 Installer       "
cecho "green" "======================================================"
echo
cecho "blue" "This script will guide you through setting up your"
cecho "blue" "Raspberry Pi as a high-quality AirPlay 2 receiver."
echo

# Run pre-flight checks first
pre_flight_checks
echo

# --- STEP 1: GATHER USER INPUT ---

# Get AirPlay Device Name
cecho "yellow" "1. Let's name your AirPlay device."
read -p "Enter the name for your speaker (e.g., Living Room): " airplay_name
if [ -z "$airplay_name" ]; then
    airplay_name="Raspberry Pi AirPlay"
fi
cecho "green" "AirPlay name will be: '$airplay_name'"
echo

# Detect and Select USB DAC
cecho "yellow" "2. Detecting external audio devices..."
# Find all audio cards that are NOT the internal one (bcm2835)
mapfile -t devices < <(aplay -l | grep -i 'card [0-9]:' | grep -iv 'bcm2835')

if [ ${#devices[@]} -eq 0 ]; then
    cecho "red" "ERROR: No external USB DAC detected!"
    cecho "red" "Please make sure your USB DAC is connected and recognized, then re-run the script."
    exit 1
elif [ ${#devices[@]} -eq 1 ]; then
    cecho "green" "Found one external audio device, auto-selecting it:"
    echo " -> ${devices[0]}"
    selected_index=0
else
    cecho "yellow" "Found multiple external audio devices. Please choose one:"
    for i in "${!devices[@]}"; do
        echo "  [$i] ${devices[$i]}"
    done
    read -p "Enter the number of the device you want to use: " device_choice
    # Validate input
    if ! [[ "$device_choice" =~ ^[0-9]+$ ]] || [ "$device_choice" -ge "${#devices[@]}" ]; then
        cecho "red" "Invalid selection. Exiting."
        exit 1
    fi
    selected_index=$device_choice
fi

# Extract card number from the chosen device
card_number=$(echo "${devices[$selected_index]}" | awk '{print $2}' | sed 's/://')
device_string="plughw:$card_number,0"
cecho "green" "Audio output will be set to: '$device_string'"
echo

# Ask about Wi-Fi Power Management
cecho "yellow" "3. Disable Wi-Fi Power Management?"
cecho "blue" "(This is highly recommended to prevent audio dropouts and stuttering)"
read -p "Disable this feature for maximum stability? (Y/n): " wifi_choice
# Default to Yes if user presses Enter or types 'y' or 'Y'
if [[ -z "$wifi_choice" || "$wifi_choice" =~ ^[Yy]$ ]]; then
    disable_wifi_power_management=true
    cecho "green" "Wi-Fi Power Management will be disabled."
else
    disable_wifi_power_management=false
    cecho "green" "Wi-Fi Power Management will be left at its default state."
fi
echo

# --- STEP 2: CONFIRMATION ---
cecho "yellow" "--- CONFIGURATION SUMMARY ---"
cecho "yellow" "  - AirPlay Name: $airplay_name"
cecho "yellow" "  - Audio Output: $device_string"
cecho "yellow" "  - Disable Wi-Fi Power Management: $disable_wifi_power_management"
cecho "yellow" "-----------------------------"
echo
read -p "Press Enter to begin the installation, or Ctrl+C to cancel."

# --- STEP 3: INSTALLATION ---
cecho "blue" "\nUpdating system and installing dependencies. This may take a few minutes..."
sudo apt-get update && sudo apt-get -y upgrade
sudo apt-get install -y build-essential git autoconf automake libtool libpopt-dev libconfig-dev libasound2-dev avahi-daemon libavahi-client-dev libssl-dev libsoxr-dev libplist-dev libsodium-dev libavutil-dev libavcodec-dev libavformat-dev uuid-dev libgcrypt-dev xxd

cecho "blue" "\nInstalling NQPTP (AirPlay 2 Clock)..."
cd /tmp
git clone https://github.com/mikebrady/nqptp.git
cd nqptp
autoreconf -fi
./configure --with-systemd-startup
make && sudo make install
sudo systemctl enable nqptp && sudo systemctl start nqptp
cecho "green" "NQPTP installed successfully."

cecho "blue" "\nInstalling Shairport-Sync (AirPlay Receiver)..."
cecho "yellow" "This next step (compiling) is slow, especially on a Pi Zero (10-15 mins)."
cd /tmp
git clone https://github.com/mikebrady/shairport-sync.git
cd shairport-sync
autoreconf -fi
./configure --sysconfdir=/etc --with-alsa --with-avahi --with-ssl=openssl --with-soxr --with-systemd --with-airplay-2
make && sudo make install
cecho "green" "Shairport-Sync installed successfully."

# --- STEP 4: CONFIGURATION & SERVICES ---
cecho "blue" "\nApplying your custom configuration..."
# Create a backup
[ -f /etc/shairport-sync.conf ] && sudo mv /etc/shairport-sync.conf /etc/shairport-sync.conf.bak
# Create new config file with user-defined values
sudo tee /etc/shairport-sync.conf > /dev/null <<EOF
// Shairport-Sync configuration file, generated by robust install script
general = {
  name = "$airplay_name";
  interpolation = "soxr"; // Use high-quality interpolation
  volume_max_db = 3.0;
  default_airplay_volume = -6.0;
  high_volume_idle_timeout_in_minutes = 1;
};
alsa = {
  output_device = "$device_string";
  mixer_control_name = "PCM";
};
EOF
cecho "green" "Shairport-Sync configuration applied."

cecho "blue" "\nCreating and enabling the system service..."
sudo tee /lib/systemd/system/shairport-sync.service > /dev/null <<EOF
[Unit]
Description=Shairport Sync - AirPlay Audio Receiver
After=sound.target network-online.target
Requires=nqptp.service
After=nqptp.service
[Service]
ExecStart=/usr/local/bin/shairport-sync
Restart=on-failure
RestartSec=5
[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable shairport-sync
sudo systemctl start shairport-sync
cecho "green" "Shairport-Sync service is now active."

# Disable Wi-Fi Power Management if user chose to
if [ "$disable_wifi_power_management" = true ]; then
    cecho "blue" "\nDisabling Wi-Fi Power Management..."
    sudo tee /etc/rc.local > /dev/null <<EOF
#!/bin/bash
/sbin/iw dev wlan0 set power_save off
exit 0
EOF
    sudo chmod +x /etc/rc.local
    sudo tee /etc/systemd/system/rc-local.service > /dev/null <<EOF
[Unit]
Description=/etc/rc.local Compatibility
ConditionFileIsExecutable=/etc/rc.local
After=network.target
[Service]
Type=forking
ExecStart=/etc/rc.local start
TimeoutSec=0
[Install]
WantedBy=multi-user.target
EOF
    sudo systemctl enable rc-local.service
    cecho "green" "Wi-Fi Power Management disabled."
fi

# --- STEP 5: CLEANUP & FINISH ---
cecho "blue" "\nCleaning up installation files..."
rm -rf /tmp/nqptp
rm -rf /tmp/shairport-sync
cecho "green" "Cleanup complete."

cecho "green" "\n======================================================"
cecho "green" "            ✅ INSTALLATION COMPLETE! ✅            "
cecho "green" "======================================================"
cecho "blue" "\nYour Raspberry Pi is now an AirPlay 2 receiver named:"
cecho "yellow" "\n  -->  $airplay_name  <--\n"
cecho "blue" "It should appear on your Apple devices shortly."
cecho "blue" "A reboot is recommended to ensure all changes are applied correctly."
read -p "Press Enter to reboot now, or Ctrl+C to reboot later."
sudo reboot
