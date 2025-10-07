#!/bin/bash

# ===================================================================================
# Shairport-Sync AirPlay 2 ROBUST Installer - ENHANCED VERSION
#
# Tailored for: Raspberry Pi (Zero/3/4/5) with USB DAC
# Version: 2.1 Enhanced
# Features: Better error handling, audio testing, mixer detection, validation
# ===================================================================================

set -e  # Exit on error

# --- Helper Functions ---
cecho() {
    local code="\033["
    case "$1" in
        "red")    color="${code}1;31m" ;;
        "green")  color="${code}1;32m" ;;
        "yellow") color="${code}1;33m" ;;
        "blue")   color="${code}1;34m" ;;
        "magenta") color="${code}1;35m" ;;
        *)        color="${code}0m" ;;
    esac
    echo -e "${color}$2\033[0m"
}

# Show spinner during long operations
show_spinner() {
    local pid=$1
    local delay=0.1
    local spinstr='|/-\'
    while [ "$(ps a | awk '{print $1}' | grep $pid)" ]; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    printf "    \b\b\b\b"
}

# Check if a service is running properly
check_service() {
    local service_name=$1
    if systemctl is-active --quiet "$service_name"; then
        cecho "green" "✓ $service_name is running"
        return 0
    else
        cecho "red" "✗ $service_name failed to start"
        cecho "yellow" "Debug info:"
        sudo systemctl status "$service_name" --no-pager -l
        return 1
    fi
}

# --- Pre-flight Checks ---
pre_flight_checks() {
    cecho "blue" "═══════════════════════════════════════"
    cecho "blue" "   Running Pre-Flight Checks..."
    cecho "blue" "═══════════════════════════════════════"
    echo

    # Check for root user
    if [ "$EUID" -eq 0 ]; then
        cecho "red" "❌ Error: Don't run this script with sudo or as root."
        cecho "yellow" "   Just run: ./install_airplay.sh"
        exit 1
    fi

    # Check for internet connection
    cecho "yellow" "Checking internet connection..."
    if ! ping -c 1 -W 2 8.8.8.8 &> /dev/null; then
        cecho "red" "❌ No internet connection detected."
        cecho "yellow" "   Please connect to Wi-Fi and try again."
        exit 1
    fi
    cecho "green" "✓ Internet connection OK"

    # Check if running on Raspberry Pi
    if [ ! -f /proc/device-tree/model ]; then
        cecho "yellow" "⚠ Warning: This doesn't appear to be a Raspberry Pi."
        read -p "Continue anyway? (y/N): " continue_choice
        [[ ! "$continue_choice" =~ ^[Yy]$ ]] && exit 1
    else
        local pi_model=$(tr -d '\0' < /proc/device-tree/model)
        cecho "green" "✓ Detected: $pi_model"
    fi

    # Check available disk space (need at least 500MB)
    local available_space=$(df / | tail -1 | awk '{print $4}')
    if [ "$available_space" -lt 500000 ]; then
        cecho "red" "❌ Not enough disk space. Need at least 500MB free."
        exit 1
    fi
    cecho "green" "✓ Sufficient disk space available"
    echo
}

# --- Detect and Select USB DAC ---
select_audio_device() {
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 1: Audio Device Selection"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo

    # Get list of all audio cards
    cecho "blue" "Scanning for audio devices..."
    local all_cards=$(aplay -l 2>/dev/null | grep '^card' || true)
    
    if [ -z "$all_cards" ]; then
        cecho "red" "❌ No audio devices detected at all!"
        cecho "yellow" "   Make sure your USB DAC is properly connected."
        exit 1
    fi

    # Show all detected devices
    cecho "green" "Found these audio devices:"
    echo "$all_cards" | nl -w2 -s'. '
    echo

    # Filter out built-in audio (bcm2835, Headphones)
    mapfile -t external_devices < <(echo "$all_cards" | grep -iv 'bcm2835\|Headphones' || true)
    
    if [ ${#external_devices[@]} -eq 0 ]; then
        cecho "red" "❌ No external USB DAC detected!"
        cecho "yellow" "   I can only see the Pi's built-in audio."
        cecho "yellow" "   Please:"
        cecho "yellow" "   1. Connect your USB DAC"
        cecho "yellow" "   2. Wait 5 seconds"
        cecho "yellow" "   3. Run this script again"
        exit 1
    fi

    # Auto-select if only one external device
    if [ ${#external_devices[@]} -eq 1 ]; then
        cecho "green" "✓ Found one USB DAC, auto-selecting:"
        cecho "magenta" "  → ${external_devices[0]}"
        selected_device="${external_devices[0]}"
    else
        # Multiple devices - let user choose
        cecho "yellow" "Found ${#external_devices[@]} external audio devices:"
        for i in "${!external_devices[@]}"; do
            echo "  [$i] ${external_devices[$i]}"
        done
        echo
        read -p "Enter the number [0-$((${#external_devices[@]}-1))]: " device_choice
        
        # Validate input
        if ! [[ "$device_choice" =~ ^[0-9]+$ ]] || [ "$device_choice" -ge "${#external_devices[@]}" ]; then
            cecho "red" "❌ Invalid selection."
            exit 1
        fi
        selected_device="${external_devices[$device_choice]}"
    fi

    # Extract card and device numbers more reliably
    card_number=$(echo "$selected_device" | grep -oP 'card \K\d+' || echo "")
    device_number=$(echo "$selected_device" | grep -oP 'device \K\d+' || echo "0")
    
    if [ -z "$card_number" ]; then
        cecho "red" "❌ Failed to extract card number from: $selected_device"
        exit 1
    fi

    audio_device="hw:$card_number,$device_number"
    audio_device_plug="plughw:$card_number,$device_number"
    
    cecho "green" "✓ Audio device set to: $audio_device_plug"
    
    # Detect available mixer controls for this card
    cecho "blue" "Detecting volume controls..."
    mapfile -t mixers < <(amixer -c "$card_number" scontrols 2>/dev/null | grep -oP "Simple mixer control '\K[^']+" || echo "")
    
    if [ ${#mixers[@]} -eq 0 ]; then
        cecho "yellow" "⚠ No mixer controls found. Volume control will be disabled."
        mixer_control=""
    else
        # Try to find the best mixer control
        for preferred in "PCM" "Master" "Speaker" "Headphone"; do
            for mixer in "${mixers[@]}"; do
                if [[ "$mixer" == "$preferred" ]]; then
                    mixer_control="$mixer"
                    break 2
                fi
            done
        done
        
        # If no preferred mixer found, use the first one
        if [ -z "$mixer_control" ]; then
            mixer_control="${mixers[0]}"
        fi
        
        cecho "green" "✓ Volume control: $mixer_control"
    fi
    echo
}

# --- Get AirPlay Name ---
get_airplay_name() {
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 2: Name Your AirPlay Device"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    
    local hostname=$(hostname)
    cecho "blue" "This is the name that will appear on your iPhone/iPad."
    cecho "blue" "Examples: Living Room, Bedroom Speaker, Kitchen Audio"
    echo
    read -p "Enter a name (or press Enter for '$hostname AirPlay'): " airplay_name
    
    if [ -z "$airplay_name" ]; then
        airplay_name="$hostname AirPlay"
    fi
    
    cecho "green" "✓ AirPlay name: '$airplay_name'"
    echo
}

# --- Wi-Fi Power Management ---
configure_wifi() {
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 3: Wi-Fi Optimization"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    
    cecho "blue" "Wi-Fi power saving can cause audio stuttering and dropouts."
    cecho "blue" "Disabling it ensures smooth, uninterrupted playback."
    echo
    read -p "Disable Wi-Fi power saving? (Y/n): " wifi_choice
    
    if [[ -z "$wifi_choice" || "$wifi_choice" =~ ^[Yy]$ ]]; then
        disable_wifi_pm=true
        cecho "green" "✓ Wi-Fi power management will be disabled"
    else
        disable_wifi_pm=false
        cecho "yellow" "⚠ Keeping default Wi-Fi settings (may cause dropouts)"
    fi
    echo
}

# --- Main Installation ---
clear
cecho "green" "╔═════════════════════════════════════════════════════╗"
cecho "green" "║                                                     ║"
cecho "green" "║     AirPlay 2 Installer for Raspberry Pi + DAC     ║"
cecho "green" "║                  Version 2.1                        ║"
cecho "green" "║                                                     ║"
cecho "green" "╚═════════════════════════════════════════════════════╝"
echo
cecho "blue" "This installer will turn your Raspberry Pi into a"
cecho "blue" "high-quality AirPlay 2 receiver. Just follow the prompts!"
echo
read -p "Press Enter to begin..."
echo

# Run all setup steps
pre_flight_checks
select_audio_device
get_airplay_name
configure_wifi

# --- Confirmation ---
cecho "magenta" "╔═════════════════════════════════════════════════════╗"
cecho "magenta" "║           INSTALLATION CONFIGURATION                ║"
cecho "magenta" "╚═════════════════════════════════════════════════════╝"
echo
cecho "yellow" "  📱 AirPlay Name:        $airplay_name"
cecho "yellow" "  🔊 Audio Output:        $audio_device_plug"
cecho "yellow" "  🎚️  Volume Control:      ${mixer_control:-None (fixed volume)}"
cecho "yellow" "  📡 Disable Wi-Fi PM:    $disable_wifi_pm"
echo
cecho "blue" "Installation will take 10-20 minutes depending on your Pi model."
cecho "blue" "(Pi Zero will be slower, Pi 4/5 will be faster)"
echo
read -p "Press Enter to start installation, or Ctrl+C to cancel..."
echo

# --- System Update ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Updating System Packages..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
sudo apt-get update -qq
sudo apt-get upgrade -y -qq
cecho "green" "✓ System updated"
echo

# --- Install Dependencies ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Installing Dependencies..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
sudo apt-get install -y -qq \
    build-essential git autoconf automake libtool \
    libpopt-dev libconfig-dev libasound2-dev \
    avahi-daemon libavahi-client-dev libssl-dev \
    libsoxr-dev libplist-dev libsodium-dev \
    libavutil-dev libavcodec-dev libavformat-dev \
    uuid-dev libgcrypt-dev xxd alsa-utils
cecho "green" "✓ Dependencies installed"
echo

# --- Install NQPTP ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Installing NQPTP (Timing System)..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cd /tmp
rm -rf nqptp 2>/dev/null || true
git clone -q https://github.com/mikebrady/nqptp.git
cd nqptp
autoreconf -fi > /dev/null 2>&1
./configure --with-systemd-startup > /dev/null 2>&1
make -j$(nproc) > /dev/null 2>&1
sudo make install > /dev/null 2>&1
sudo systemctl enable nqptp > /dev/null 2>&1
sudo systemctl start nqptp
sleep 2
check_service "nqptp" || exit 1
echo

# --- Install Shairport-Sync ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Installing Shairport-Sync..."
cecho "blue" "   (This takes 10-15 mins on Pi Zero)"
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cd /tmp
rm -rf shairport-sync 2>/dev/null || true
git clone -q https://github.com/mikebrady/shairport-sync.git
cd shairport-sync
autoreconf -fi > /dev/null 2>&1
./configure --sysconfdir=/etc --with-alsa --with-avahi \
    --with-ssl=openssl --with-soxr --with-systemd \
    --with-airplay-2 > /dev/null 2>&1

cecho "yellow" "Compiling (be patient)..."
make -j$(nproc) > /dev/null 2>&1 &
make_pid=$!
show_spinner $make_pid
wait $make_pid

sudo make install > /dev/null 2>&1
cecho "green" "✓ Shairport-Sync compiled and installed"
echo

# --- Configure Shairport-Sync ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Configuring Shairport-Sync..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

# Backup existing config
[ -f /etc/shairport-sync.conf ] && sudo cp /etc/shairport-sync.conf /etc/shairport-sync.conf.backup

# Create optimized config
sudo tee /etc/shairport-sync.conf > /dev/null <<EOF
// Generated by AirPlay 2 Installer v2.1
general = {
  name = "$airplay_name";
  interpolation = "soxr";
  output_backend = "alsa";
  volume_max_db = 3.0;
  default_airplay_volume = -6.0;
  high_volume_idle_timeout_in_minutes = 1;
};

alsa = {
  output_device = "$audio_device_plug";
EOF

# Only add mixer control if one was detected
if [ -n "$mixer_control" ]; then
    sudo tee -a /etc/shairport-sync.conf > /dev/null <<EOF
  mixer_control_name = "$mixer_control";
EOF
fi

sudo tee -a /etc/shairport-sync.conf > /dev/null <<EOF
  output_rate = 44100;
  output_format = "S16";
};
EOF

cecho "green" "✓ Configuration file created"

# Set mixer volume to maximum if available
if [ -n "$mixer_control" ]; then
    amixer -c "$card_number" set "$mixer_control" 100% > /dev/null 2>&1 || true
    sudo alsactl store > /dev/null 2>&1 || true
    cecho "green" "✓ Mixer volume set to maximum"
fi
echo

# --- Create Systemd Service ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Setting Up Auto-Start Service..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

sudo tee /lib/systemd/system/shairport-sync.service > /dev/null <<EOF
[Unit]
Description=Shairport Sync - AirPlay Audio Receiver
After=sound.target network-online.target avahi-daemon.service
Wants=network-online.target
Requires=nqptp.service
After=nqptp.service

[Service]
Type=notify
ExecStart=/usr/local/bin/shairport-sync
Restart=on-failure
RestartSec=5
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable shairport-sync > /dev/null 2>&1
sudo systemctl restart shairport-sync
sleep 3
check_service "shairport-sync" || exit 1
echo

# --- Configure Wi-Fi Power Management ---
if [ "$disable_wifi_pm" = true ]; then
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Disabling Wi-Fi Power Saving..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    
    sudo tee /etc/rc.local > /dev/null <<EOF
#!/bin/bash
# Disable Wi-Fi power management for stable audio streaming
/sbin/iw dev wlan0 set power_save off 2>/dev/null || true
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
RemainAfterExit=yes
SysVStartPriority=99

[Install]
WantedBy=multi-user.target
EOF
    
    sudo systemctl daemon-reload
    sudo systemctl enable rc-local.service > /dev/null 2>&1
    
    # Apply immediately
    sudo /sbin/iw dev wlan0 set power_save off 2>/dev/null || true
    cecho "green" "✓ Wi-Fi power management disabled"
    echo
fi

# --- Audio Test ---
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
cecho "blue" "   Testing Audio Output..."
cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo
cecho "yellow" "Playing test sound in 2 seconds..."
sleep 2

if speaker-test -D "$audio_device_plug" -c 2 -t wav -l 1 > /dev/null 2>&1; then
    cecho "green" "✓ Audio test successful!"
else
    cecho "yellow" "⚠ Audio test couldn't run, but setup is complete."
fi
echo

# --- Cleanup ---
cecho "blue" "Cleaning up temporary files..."
rm -rf /tmp/nqptp /tmp/shairport-sync
cecho "green" "✓ Cleanup complete"
echo

# --- Success Message ---
cecho "green" "╔═════════════════════════════════════════════════════╗"
cecho "green" "║                                                     ║"
cecho "green" "║            ✅ INSTALLATION COMPLETE! ✅            ║"
cecho "green" "║                                                     ║"
cecho "green" "╚═════════════════════════════════════════════════════╝"
echo
cecho "magenta" "🎵 Your AirPlay 2 device is ready!"
echo
cecho "yellow" "  📱 Device Name:  $airplay_name"
cecho "yellow" "  🔊 Audio Output: $audio_device_plug"
echo
cecho "blue" "How to use:"
cecho "blue" "  1. Open Music, Spotify, or any audio app on your iPhone/iPad"
cecho "blue" "  2. Tap the AirPlay icon (📡)"
cecho "blue" "  3. Select '$airplay_name'"
cecho "blue" "  4. Enjoy high-quality wireless audio!"
echo
cecho "yellow" "💡 Tip: Your device should appear within 30 seconds."
cecho "yellow" "    If not, make sure your phone and Pi are on the same Wi-Fi network."
echo
cecho "blue" "To view logs: sudo journalctl -u shairport-sync -f"
cecho "blue" "To restart:   sudo systemctl restart shairport-sync"
echo
read -p "Press Enter to reboot now (recommended), or Ctrl+C to reboot later..."
sudo reboot
