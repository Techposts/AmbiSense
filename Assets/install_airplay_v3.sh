#!/bin/bash

# ===================================================================================
# Shairport-Sync AirPlay 2 ROBUST Installer - ENHANCED VERSION 3.0
#
# Tailored for: Raspberry Pi (Zero 2/3/4/5) with USB DAC
# Version: 3.0 - Production Ready
# Features:
#   - Comprehensive error handling with rollback capability
#   - Dependency validation before installation
#   - Build failure recovery
#   - Audio device validation
#   - Service health checks
#   - Firewall configuration
#   - Latest package versions
# ===================================================================================

set -eo pipefail   # Exit on error and pipe failures
IFS=$'\n\t'        # Safer word splitting

# --- Global Variables ---
SCRIPT_VERSION="3.0"
LOG_FILE="/tmp/airplay_install_$(date +%Y%m%d_%H%M%S).log"
BACKUP_DIR="/tmp/airplay_backup_$(date +%Y%m%d_%H%M%S)"
INSTALLATION_FAILED=0

# Audio configuration variables
audio_device=""
audio_device_plug=""
card_number=""
device_number=""
mixer_control=""
selected_device=""
airplay_name=""
disable_wifi_pm=false

# --- Cleanup Handler ---
cleanup() {
    local exit_code=$?
    if [ $exit_code -ne 0 ] && [ $INSTALLATION_FAILED -eq 1 ]; then
        cecho "red" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        cecho "red" "   Installation Failed - Cleaning Up"
        cecho "red" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

        # Stop services if they were started
        sudo systemctl stop shairport-sync 2>/dev/null || true
        sudo systemctl stop nqptp 2>/dev/null || true

        # Restore backups if they exist
        if [ -d "$BACKUP_DIR" ]; then
            cecho "yellow" "Restoring original configuration..."
            [ -f "$BACKUP_DIR/shairport-sync.conf" ] && \
                sudo cp "$BACKUP_DIR/shairport-sync.conf" /etc/shairport-sync.conf 2>/dev/null || true
        fi

        cecho "yellow" "Installation log saved to: $LOG_FILE"
        cecho "yellow" "Please check the log for details."
    fi

    # Cleanup temp build directories
    rm -rf /tmp/nqptp /tmp/shairport-sync 2>/dev/null || true
}

trap cleanup EXIT ERR INT TERM

# --- Logging Function ---
log() {
    # Create log file if it doesn't exist
    touch "$LOG_FILE" 2>/dev/null || true
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $*" | tee -a "$LOG_FILE"
}

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
    while ps -p $pid > /dev/null 2>&1; do
        local temp=${spinstr#?}
        printf " [%c]  " "$spinstr"
        local spinstr=$temp${spinstr%"$temp"}
        sleep $delay
        printf "\b\b\b\b\b\b"
    done
    printf "    \b\b\b\b"
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if a service is running properly
check_service() {
    local service_name=$1
    local max_retries=${2:-3}
    local retry=0

    while [ $retry -lt $max_retries ]; do
        if systemctl is-active --quiet "$service_name"; then
            cecho "green" "✓ $service_name is running"
            return 0
        fi
        retry=$((retry + 1))
        [ $retry -lt $max_retries ] && sleep 2
    done

    cecho "red" "✗ $service_name failed to start after $max_retries attempts"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Diagnostic Information:"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    echo "Service Status:" | tee -a "$LOG_FILE"
    sudo systemctl status "$service_name" --no-pager -l 2>&1 | tee -a "$LOG_FILE"
    echo
    echo "Recent Logs:" | tee -a "$LOG_FILE"
    sudo journalctl -u "$service_name" -n 30 --no-pager 2>&1 | tee -a "$LOG_FILE"
    echo
    cecho "yellow" "Check the log file for more details: $LOG_FILE"
    return 1
}

# Validate package installation
validate_package() {
    local package=$1
    if dpkg -l "$package" 2>/dev/null | grep -q "^ii"; then
        return 0
    else
        cecho "red" "✗ Package $package failed to install"
        return 1
    fi
}

# Safe directory change
safe_cd() {
    cd "$1" || {
        cecho "red" "Failed to change directory to: $1"
        exit 1
    }
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
        cecho "yellow" "   Just run: bash install_airplay_v3.sh"
        exit 1
    fi

    # Check if user can sudo
    if ! sudo -n true 2>/dev/null; then
        cecho "yellow" "Checking sudo access..."
        if ! sudo true; then
            cecho "red" "❌ This script requires sudo access."
            exit 1
        fi
    fi
    cecho "green" "✓ Sudo access confirmed"

    # Check for internet connection
    cecho "yellow" "Checking internet connection..."
    local test_hosts=("8.8.8.8" "1.1.1.1" "github.com")
    local connection_ok=0

    for host in "${test_hosts[@]}"; do
        # Use timeout command to prevent hanging
        cecho "blue" "  Testing $host..."
        timeout 8 ping -c 1 -W 5 "$host" >/dev/null 2>&1 || true
        local result=$?
        if [ $result -eq 0 ]; then
            connection_ok=1
            log "Internet check: Successfully pinged $host"
            cecho "green" "  ✓ Connected"
            break
        elif [ $result -eq 124 ]; then
            log "Internet check: Timeout pinging $host"
            cecho "yellow" "  ✗ Timeout"
        else
            log "Internet check: Failed to ping $host (exit $result)"
            cecho "yellow" "  ✗ Failed"
        fi
    done

    if [ $connection_ok -eq 0 ]; then
        cecho "red" "❌ No internet connection detected."
        cecho "yellow" "   Troubleshooting:"
        cecho "yellow" "   1. Check Wi-Fi is connected: iwconfig"
        cecho "yellow" "   2. Test manually: ping -c 3 8.8.8.8"
        cecho "yellow" "   3. Check DNS: ping -c 3 github.com"
        echo
        cecho "blue" "   Your network status:"
        ip addr show wlan0 2>/dev/null | grep "inet " || echo "   No wlan0 IP address"
        echo
        read -p "Skip internet check and continue anyway? (y/N): " skip_check || true
        if [[ "$skip_check" =~ ^[Yy]$ ]]; then
            cecho "yellow" "⚠ Continuing without internet check (may fail later)"
            log "User skipped internet check"
        else
            exit 1
        fi
    else
        cecho "green" "✓ Internet connection OK"
    fi

    # Check if running on Raspberry Pi
    if [ ! -f /proc/device-tree/model ]; then
        cecho "yellow" "⚠ Warning: This doesn't appear to be a Raspberry Pi."
        read -p "Continue anyway? (y/N): " continue_choice || true
        [[ ! "$continue_choice" =~ ^[Yy]$ ]] && exit 1
    else
        local pi_model
        pi_model=$(tr -d '\0' < /proc/device-tree/model)
        cecho "green" "✓ Detected: $pi_model"

        # Warn on older Pi models
        if echo "$pi_model" | grep -qE "Pi Zero W|Pi 1"; then
            cecho "yellow" "⚠ Warning: $pi_model may not have enough power for AirPlay 2"
            cecho "yellow" "   Recommended: Pi Zero 2 or newer"
            read -p "Continue anyway? (y/N): " continue_choice || true
            [[ ! "$continue_choice" =~ ^[Yy]$ ]] && exit 1
        fi
    fi

    # Check available disk space (need at least 1GB)
    local available_space
    available_space=$(df / | tail -1 | awk '{print $4}')
    if [ "$available_space" -lt 1000000 ]; then
        cecho "red" "❌ Not enough disk space. Need at least 1GB free."
        cecho "yellow" "   Current available: $((available_space / 1024)) MB"
        exit 1
    fi
    cecho "green" "✓ Sufficient disk space available: $((available_space / 1024)) MB"

    # Check available memory
    local available_mem
    available_mem=$(free -m | awk '/^Mem:/{print $7}')
    if [ "$available_mem" -lt 100 ]; then
        cecho "yellow" "⚠ Warning: Low available memory ($available_mem MB)"
        cecho "yellow" "   Consider closing other applications."
    else
        cecho "green" "✓ Available memory: $available_mem MB"
    fi

    # Check for required base tools
    cecho "yellow" "Checking required tools..."
    local required_tools=("git" "gcc" "make" "aplay" "amixer")
    local missing_tools=()

    for tool in "${required_tools[@]}"; do
        if ! command_exists "$tool"; then
            missing_tools+=("$tool")
        fi
    done

    if [ ${#missing_tools[@]} -gt 0 ]; then
        cecho "yellow" "⚠ Missing tools: ${missing_tools[*]}"
        cecho "yellow" "   These will be installed with dependencies."
    else
        cecho "green" "✓ All base tools present"
    fi

    echo
}

# --- Detect and Select USB DAC ---
select_audio_device() {
    echo
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 1: Audio Device Selection"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    cecho "cyan" "⏸  PLEASE RESPOND TO THIS PROMPT ⏸"
    echo

    # Get list of all audio cards
    cecho "blue" "Scanning for audio devices..."
    local all_cards
    all_cards=$(aplay -l 2>/dev/null | grep '^card' || true)

    if [ -z "$all_cards" ]; then
        cecho "red" "❌ No audio devices detected at all!"
        cecho "yellow" "   Make sure your USB DAC is properly connected."
        cecho "yellow" "   Try: lsusb (to check if USB device is recognized)"
        exit 1
    fi

    # Show all detected devices
    cecho "green" "Found these audio devices:"
    echo "$all_cards" | nl -w2 -s'. '
    echo

    # Filter out built-in audio (bcm2835, Headphones, vc4-hdmi)
    mapfile -t external_devices < <(echo "$all_cards" | grep -iv 'bcm2835\|Headphones\|vc4-hdmi' || true)

    if [ ${#external_devices[@]} -eq 0 ]; then
        cecho "yellow" "⚠ No external USB DAC detected!"
        cecho "yellow" "   Only built-in audio found."
        echo
        read -p "Do you want to use built-in audio? (y/N): " use_builtin || true

        if [[ "$use_builtin" =~ ^[Yy]$ ]]; then
            mapfile -t external_devices < <(echo "$all_cards")
        else
            cecho "yellow" "   Please:"
            cecho "yellow" "   1. Connect your USB DAC"
            cecho "yellow" "   2. Wait 5 seconds"
            cecho "yellow" "   3. Run this script again"
            exit 1
        fi
    fi

    # Auto-select if only one device
    if [ ${#external_devices[@]} -eq 1 ]; then
        cecho "green" "✓ Found one audio device, auto-selecting:"
        cecho "magenta" "  → ${external_devices[0]}"
        selected_device="${external_devices[0]}"
    else
        # Multiple devices - let user choose
        cecho "yellow" "Found ${#external_devices[@]} audio devices:"
        for i in "${!external_devices[@]}"; do
            echo "  [$i] ${external_devices[$i]}"
        done
        echo

        local device_choice
        while true; do
            read -p "Enter the number [0-$((${#external_devices[@]}-1))]: " device_choice || true

            if [[ "$device_choice" =~ ^[0-9]+$ ]] && [ "$device_choice" -lt "${#external_devices[@]}" ]; then
                break
            fi
            cecho "red" "Invalid selection. Please try again."
        done

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

    # Validate the audio device actually works
    cecho "blue" "Validating audio device..."
    if aplay -D "$audio_device_plug" -l >/dev/null 2>&1; then
        cecho "green" "✓ Audio device validation passed"
    else
        cecho "red" "❌ Audio device validation failed"
        cecho "yellow" "   The device may not support playback."
        read -p "Continue anyway? (y/N): " continue_choice || true
        [[ ! "$continue_choice" =~ ^[Yy]$ ]] && exit 1
    fi

    # Detect available mixer controls for this card
    cecho "blue" "Detecting volume controls..."
    mapfile -t mixers < <(amixer -c "$card_number" scontrols 2>/dev/null | grep -oP "Simple mixer control '\K[^']+" || true)

    if [ ${#mixers[@]} -eq 0 ]; then
        cecho "yellow" "⚠ No mixer controls found. Volume control will be disabled."
        mixer_control=""
    else
        cecho "green" "Available mixer controls:"
        for mixer in "${mixers[@]}"; do
            echo "  - $mixer"
        done

        # Try to find the best mixer control
        mixer_control=""
        for preferred in "PCM" "Master" "Speaker" "Headphone" "Digital"; do
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
    echo
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 2: Name Your AirPlay Device"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    cecho "cyan" "⏸  PLEASE RESPOND TO THIS PROMPT ⏸"
    echo

    local hostname
    hostname=$(hostname)
    cecho "blue" "This is the name that will appear on your iPhone/iPad."
    cecho "blue" "Examples: Living Room, Bedroom Speaker, Kitchen Audio"
    echo
    cecho "green" ">>> "
    read -p "Enter a name (or press Enter for '$hostname AirPlay'): " airplay_name || true

    if [ -z "$airplay_name" ]; then
        airplay_name="$hostname AirPlay"
    fi

    # Sanitize the name (remove special characters that might cause issues)
    airplay_name=$(echo "$airplay_name" | sed 's/[^a-zA-Z0-9 _-]//g')

    cecho "green" "✓ AirPlay name: '$airplay_name'"
    echo
}

# --- Wi-Fi Power Management ---
configure_wifi() {
    echo
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "yellow" "   Step 3: Wi-Fi Optimization"
    cecho "yellow" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo
    cecho "cyan" "⏸  PLEASE RESPOND TO THIS PROMPT ⏸"
    echo

    # Check if Wi-Fi is actually being used
    if ! ip link show wlan0 &>/dev/null; then
        cecho "yellow" "⚠ No Wi-Fi interface detected (wlan0 not found)"
        cecho "yellow" "   Skipping Wi-Fi optimization."
        disable_wifi_pm=false
        echo
        return
    fi

    cecho "blue" "Wi-Fi power saving can cause audio stuttering and dropouts."
    cecho "blue" "Disabling it ensures smooth, uninterrupted playback."
    echo
    cecho "green" ">>> "
    read -p "Disable Wi-Fi power saving? (Y/n): " wifi_choice || true

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
main() {
    # Initialize log file immediately
    touch "$LOG_FILE" 2>/dev/null || LOG_FILE="/tmp/airplay_install_fallback.log"

    clear
    cecho "green" "╔═════════════════════════════════════════════════════╗"
    cecho "green" "║                                                     ║"
    cecho "green" "║     AirPlay 2 Installer for Raspberry Pi + DAC     ║"
    cecho "green" "║                  Version $SCRIPT_VERSION                        ║"
    cecho "green" "║                                                     ║"
    cecho "green" "╚═════════════════════════════════════════════════════╝"
    echo
    cecho "blue" "This installer will turn your Raspberry Pi into a"
    cecho "blue" "high-quality AirPlay 2 receiver. Just follow the prompts!"
    echo
    cecho "yellow" "Installation log: $LOG_FILE"
    echo

    log "=== AirPlay 2 Installation Started ==="
    log "Script Version: $SCRIPT_VERSION"
    log "Date: $(date)"
    log "User: $(whoami)"
    log "System: $(uname -a)"
    echo

    # Check if running interactively
    if [ -t 0 ]; then
        read -p "Press Enter to begin..." || true
    else
        cecho "yellow" "⚠ Non-interactive mode detected - using defaults"
        sleep 2
    fi
    echo

    # Run all setup steps
    pre_flight_checks
    select_audio_device
    get_airplay_name
    configure_wifi

    # --- Confirmation ---
    echo
    echo
    cecho "magenta" "╔═════════════════════════════════════════════════════╗"
    cecho "magenta" "║           INSTALLATION CONFIGURATION                ║"
    cecho "magenta" "╚═════════════════════════════════════════════════════╝"
    echo
    cecho "yellow" "  📱 AirPlay Name:        $airplay_name"
    cecho "yellow" "  🔊 Audio Output:        $audio_device_plug"
    cecho "yellow" "  🎚️  Volume Control:      ${mixer_control:-None (fixed volume)}"
    cecho "yellow" "  📡 Disable Wi-Fi PM:    $disable_wifi_pm"
    echo
    cecho "blue" "Installation will take 10-30 minutes depending on your Pi model."
    cecho "blue" "(Pi Zero 2 will be slower, Pi 4/5 will be faster)"
    echo
    echo
    cecho "cyan" "⏸  FINAL CONFIRMATION - PRESS ENTER TO CONTINUE ⏸"
    echo
    if [ -t 0 ]; then
        read -p "Press Enter to start installation, or Ctrl+C to cancel..." || true
    else
        cecho "yellow" "Auto-starting in 5 seconds (non-interactive mode)..."
        sleep 5
    fi
    echo

    INSTALLATION_FAILED=1  # Mark that installation has started

    # Create backup directory
    mkdir -p "$BACKUP_DIR"
    [ -f /etc/shairport-sync.conf ] && cp /etc/shairport-sync.conf "$BACKUP_DIR/" 2>/dev/null || true

    # --- System Update ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Updating System Packages..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Updating package lists..."

    if ! sudo apt-get update -qq 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Failed to update package lists"
        exit 1
    fi

    cecho "yellow" "Upgrading existing packages (this may take a few minutes)..."
    if ! sudo apt-get upgrade -y 2>&1 | tee -a "$LOG_FILE"; then
        cecho "yellow" "⚠ Package upgrade had issues, but continuing..."
    fi

    cecho "green" "✓ System updated"
    echo

    # --- Install Dependencies ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Installing Dependencies..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Installing build dependencies..."

    local dependencies=(
        build-essential git autoconf automake libtool pkg-config
        libpopt-dev libconfig-dev libasound2-dev
        avahi-daemon libavahi-client-dev libssl-dev
        libsoxr-dev libplist-dev libsodium-dev
        libavutil-dev libavcodec-dev libavformat-dev
        uuid-dev libgcrypt20-dev xxd alsa-utils
    )

    if ! sudo apt-get install -y "${dependencies[@]}" 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Failed to install dependencies"
        exit 1
    fi

    # Validate critical packages
    local critical_packages=("build-essential" "git" "libasound2-dev" "avahi-daemon")
    for package in "${critical_packages[@]}"; do
        if ! validate_package "$package"; then
            cecho "red" "❌ Critical package $package is not installed"
            exit 1
        fi
    done

    cecho "green" "✓ Dependencies installed"
    echo

    # Make sure avahi-daemon is running
    if ! systemctl is-active --quiet avahi-daemon; then
        cecho "yellow" "Starting avahi-daemon..."
        sudo systemctl enable avahi-daemon
        sudo systemctl start avahi-daemon
    fi

    # --- Install NQPTP ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Installing NQPTP (Timing System)..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Cloning NQPTP repository..."

    # Verify /tmp is writable
    if [ ! -w /tmp ]; then
        cecho "red" "❌ /tmp directory is not writable"
        exit 1
    fi

    safe_cd /tmp
    rm -rf nqptp 2>/dev/null || true

    if ! git clone https://github.com/mikebrady/nqptp.git 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Failed to clone NQPTP repository"
        cecho "yellow" "   Possible causes:"
        cecho "yellow" "   - No internet connection"
        cecho "yellow" "   - GitHub is down"
        cecho "yellow" "   - Firewall blocking access"
        exit 1
    fi

    safe_cd nqptp
    log "Building NQPTP..."

    if ! autoreconf -fi 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ NQPTP autoreconf failed"
        exit 1
    fi

    if ! ./configure --with-systemd-startup 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ NQPTP configure failed"
        exit 1
    fi

    if ! make -j"$(nproc)" 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ NQPTP compilation failed"
        exit 1
    fi

    if ! sudo make install 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ NQPTP installation failed"
        exit 1
    fi

    # Verify binary was installed
    if ! command_exists nqptp; then
        cecho "red" "❌ NQPTP binary not found after installation"
        cecho "yellow" "   Expected location: /usr/local/bin/nqptp"
        exit 1
    fi

    # Enable and start NQPTP
    sudo systemctl enable nqptp 2>&1 | tee -a "$LOG_FILE"
    sudo systemctl restart nqptp 2>&1 | tee -a "$LOG_FILE"
    sleep 3

    if ! check_service "nqptp"; then
        cecho "red" "❌ NQPTP service failed to start"
        exit 1
    fi
    echo

    # --- Install Shairport-Sync ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Installing Shairport-Sync..."
    cecho "blue" "   (This takes 10-20 mins on slower Pis)"
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Cloning Shairport-Sync repository..."

    safe_cd /tmp
    rm -rf shairport-sync 2>/dev/null || true

    if ! git clone https://github.com/mikebrady/shairport-sync.git 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Failed to clone Shairport-Sync repository"
        cecho "yellow" "   Possible causes:"
        cecho "yellow" "   - No internet connection"
        cecho "yellow" "   - GitHub is down"
        cecho "yellow" "   - Firewall blocking access"
        exit 1
    fi

    safe_cd shairport-sync
    log "Building Shairport-Sync..."

    if ! autoreconf -fi 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Shairport-Sync autoreconf failed"
        exit 1
    fi

    cecho "yellow" "Configuring build..."
    if ! ./configure --sysconfdir=/etc --with-alsa --with-avahi \
        --with-ssl=openssl --with-soxr --with-systemd \
        --with-airplay-2 2>&1 | tee -a "$LOG_FILE"; then
        cecho "red" "❌ Shairport-Sync configure failed"
        exit 1
    fi

    cecho "yellow" "Compiling (be patient, this takes time)..."
    log "Starting compilation with $(nproc) cores..."

    # Run make in background so we can show spinner
    local make_log="${LOG_FILE}.make"
    make -j"$(nproc)" > "$make_log" 2>&1 &
    local make_pid=$!
    show_spinner $make_pid

    # Wait for make to complete and check status
    if ! wait $make_pid; then
        cecho "red" "❌ Shairport-Sync compilation failed"
        cecho "yellow" "Last 20 lines of build log:"
        tail -20 "$make_log" 2>/dev/null || echo "  (log file not available)"
        exit 1
    fi
    cat "$make_log" >> "$LOG_FILE" 2>/dev/null || true

    cecho "yellow" "Installing..."
    # Note: make install may fail on systemd service install, but that's OK
    # We'll create the service file manually later
    sudo make install 2>&1 | tee -a "$LOG_FILE" || true

    # What matters is that the binary was installed
    if ! command_exists shairport-sync; then
        cecho "red" "❌ Shairport-Sync binary not found after installation"
        cecho "yellow" "   Expected location: /usr/local/bin/shairport-sync"
        cecho "yellow" "   The 'make install' may have failed - check the log"
        exit 1
    fi

    cecho "green" "✓ Shairport-Sync compiled and installed"
    log "Note: make install may have shown errors about systemd service - this is normal"
    echo

    # --- Configure Shairport-Sync ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Configuring Shairport-Sync..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Creating configuration file from sample..."

    # Copy sample config as base (installed by make install)
    if [ -f /etc/shairport-sync.conf.sample ]; then
        sudo cp /etc/shairport-sync.conf.sample /etc/shairport-sync.conf
        log "Copied sample config to /etc/shairport-sync.conf"
    else
        cecho "yellow" "⚠ Sample config not found, creating minimal config"
        # Fallback to minimal config if sample doesn't exist
        sudo tee /etc/shairport-sync.conf > /dev/null <<'FALLBACK_EOF'
// Minimal configuration - sample file was not available
general = {};
alsa = {};
FALLBACK_EOF
    fi

    # Now edit the config file to set our values using simple, reliable sed commands
    log "Configuring AirPlay name: $airplay_name"

    # Set the AirPlay device name - uncomment and set it
    sudo sed -i "s|^//[[:space:]]*name = .*|        name = \"$airplay_name\";|" /etc/shairport-sync.conf

    # Set output device - uncomment and set it
    log "Configuring audio output: $audio_device_plug"
    sudo sed -i "s|^[[:space:]]*output_device = .*|        output_device = \"$audio_device_plug\";|" /etc/shairport-sync.conf
    sudo sed -i "s|^//[[:space:]]*output_device = .*|        output_device = \"$audio_device_plug\";|" /etc/shairport-sync.conf

    # Set mixer control if available
    if [ -n "$mixer_control" ]; then
        log "Configuring mixer control: $mixer_control on hw:$card_number"
        sudo sed -i "s|^//[[:space:]]*mixer_control_name = .*|        mixer_control_name = \"$mixer_control\";|" /etc/shairport-sync.conf
        sudo sed -i "s|^[[:space:]]*mixer_control_name = .*|        mixer_control_name = \"$mixer_control\";|" /etc/shairport-sync.conf
        # Also set mixer_device if needed (usually commented out by default)
        sudo sed -i "s|^//[[:space:]]*mixer_device = .*|        mixer_device = \"hw:$card_number\";|" /etc/shairport-sync.conf
        sudo sed -i "s|^[[:space:]]*mixer_device = .*|        mixer_device = \"hw:$card_number\";|" /etc/shairport-sync.conf
    fi

    # Set output format
    sudo sed -i "s|^//[[:space:]]*output_rate = .*|        output_rate = \"auto\";|" /etc/shairport-sync.conf
    sudo sed -i "s|^[[:space:]]*output_rate = .*|        output_rate = \"auto\";|" /etc/shairport-sync.conf
    sudo sed -i "s|^//[[:space:]]*output_format = .*|        output_format = \"S16\";|" /etc/shairport-sync.conf
    sudo sed -i "s|^[[:space:]]*output_format = .*|        output_format = \"S16\";|" /etc/shairport-sync.conf

    # Set volume settings
    sudo sed -i "s|^//[[:space:]]*volume_max_db = .*|        volume_max_db = 4.0;|" /etc/shairport-sync.conf
    sudo sed -i "s|^//[[:space:]]*default_airplay_volume = .*|        default_airplay_volume = -6.0;|" /etc/shairport-sync.conf
    sudo sed -i "s|^//[[:space:]]*high_volume_idle_timeout_in_minutes = .*|        high_volume_idle_timeout_in_minutes = 1;|" /etc/shairport-sync.conf

    # Verify config file was created
    if [ ! -f /etc/shairport-sync.conf ]; then
        cecho "red" "❌ Configuration file was not created"
        exit 1
    fi

    cecho "green" "✓ Configuration file created and customized"

    # Set mixer volume to maximum if available
    if [ -n "$mixer_control" ]; then
        cecho "blue" "Setting mixer volume to 100%..."
        if amixer -c "$card_number" set "$mixer_control" 100% unmute > /dev/null 2>&1; then
            sudo alsactl store > /dev/null 2>&1 || true
            cecho "green" "✓ Mixer volume set to maximum"
        else
            cecho "yellow" "⚠ Could not set mixer volume (may not be supported)"
        fi
    fi
    echo

    # --- Create/Update Systemd Service ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Setting Up Auto-Start Service..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    log "Creating shairport-sync user and group..."

    # Create user and group for shairport-sync service
    if ! getent group shairport-sync >/dev/null 2>&1; then
        sudo groupadd -r shairport-sync
        log "Created shairport-sync group"
    fi

    if ! getent passwd shairport-sync >/dev/null 2>&1; then
        sudo useradd -r -M -g shairport-sync -s /usr/sbin/nologin -G audio shairport-sync
        log "Created shairport-sync user"
    fi

    log "Creating systemd service manually (make install sometimes fails at this step)..."

    # Create systemd service file manually - this is more reliable than 'make install'
    # which often fails on the systemd service installation step on Raspberry Pi
    sudo tee /lib/systemd/system/shairport-sync.service > /dev/null <<EOF
[Unit]
Description=Shairport Sync - AirPlay Audio Receiver
After=sound.target network-online.target

[Service]
ExecStart=/usr/local/bin/shairport-sync
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

    sudo systemctl daemon-reload
    sudo systemctl enable shairport-sync 2>&1 | tee -a "$LOG_FILE"
    sudo systemctl restart shairport-sync 2>&1 | tee -a "$LOG_FILE"
    sleep 5

    if ! check_service "shairport-sync" 5; then
        cecho "red" "❌ Shairport-Sync service failed to start"
        cecho "yellow" "Checking service status..."
        sudo systemctl status shairport-sync --no-pager -l | tail -20
        exit 1
    fi

    # Verify avahi-daemon is running (required for AirPlay discovery)
    cecho "blue" "Checking Avahi daemon (required for device discovery)..."
    if ! systemctl is-active --quiet avahi-daemon; then
        cecho "yellow" "⚠ Avahi daemon is not running, attempting to start..."
        sudo systemctl start avahi-daemon
        sleep 2
        if systemctl is-active --quiet avahi-daemon; then
            cecho "green" "✓ Avahi daemon started"
        else
            cecho "red" "❌ Avahi daemon failed to start - AirPlay device may not be discoverable"
        fi
    else
        cecho "green" "✓ Avahi daemon is running"
    fi
    echo

    # --- Wi-Fi Power Management Instructions ---
    if [ "$disable_wifi_pm" = true ]; then
        cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        cecho "blue" "   Wi-Fi Power Management"
        cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        log "User requested Wi-Fi power management disable"

        cecho "yellow" "📝 Manual Wi-Fi Power Management Configuration Needed:"
        echo
        cecho "blue" "After installation completes, disable Wi-Fi power saving to prevent"
        cecho "blue" "audio dropouts. You have two options:"
        echo
        cecho "green" "Option 1: Using raspi-config (Recommended)"
        cecho "blue" "  1. Run: sudo raspi-config"
        cecho "blue" "  2. Go to: Performance Options → Wireless LAN → Power Management"
        cecho "blue" "  3. Select: Disable"
        echo
        cecho "green" "Option 2: Manual command"
        cecho "blue" "  Run: sudo iw dev wlan0 set power_save off"
        cecho "blue" "  (Note: This is temporary, resets on reboot)"
        echo
        cecho "yellow" "⚠ We're not doing this automatically to avoid disconnecting your SSH session"
        log "Wi-Fi power management instructions provided to user"
        echo
    fi

    # --- Configure Firewall (if active) ---
    if command_exists ufw && sudo ufw status | grep -q "Status: active"; then
        cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
        cecho "blue" "   Configuring Firewall..."
        cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

        # Allow mDNS for AirPlay discovery
        sudo ufw allow 5353/udp comment 'mDNS for AirPlay' 2>&1 | tee -a "$LOG_FILE"
        # Allow NQPTP ports
        sudo ufw allow 319/udp comment 'NQPTP PTP' 2>&1 | tee -a "$LOG_FILE"
        sudo ufw allow 320/udp comment 'NQPTP PTP' 2>&1 | tee -a "$LOG_FILE"
        # AirPlay ports
        sudo ufw allow 7000/tcp comment 'AirPlay' 2>&1 | tee -a "$LOG_FILE"

        cecho "green" "✓ Firewall rules added"
        echo
    fi

    # --- Audio Test ---
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    cecho "blue" "   Testing Audio Output..."
    cecho "blue" "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo

    read -p "Do you want to test audio output? (Y/n): " test_audio || true
    if [[ -z "$test_audio" || "$test_audio" =~ ^[Yy]$ ]]; then
        cecho "yellow" "Playing test sound in 2 seconds..."
        cecho "yellow" "(You should hear a voice saying 'Front Left', 'Front Right')"
        sleep 2

        if timeout 10 speaker-test -D "$audio_device_plug" -c 2 -t wav -l 1 > /dev/null 2>&1; then
            echo
            cecho "green" "✓ Audio test completed!"
            read -p "Did you hear the test sound? (y/N): " heard_sound || true
            if [[ ! "$heard_sound" =~ ^[Yy]$ ]]; then
                cecho "yellow" "⚠ If you didn't hear sound, check:"
                cecho "yellow" "  - Speaker/headphone connections"
                cecho "yellow" "  - Volume level on your amplifier/speakers"
                cecho "yellow" "  - USB DAC power"
            fi
        else
            cecho "yellow" "⚠ Audio test couldn't run, but setup is complete."
            cecho "yellow" "  You can test manually with: speaker-test -D $audio_device_plug -c 2 -t wav"
        fi
    fi
    echo

    # --- Cleanup ---
    cecho "blue" "Cleaning up temporary files..."
    rm -rf /tmp/nqptp /tmp/shairport-sync 2>/dev/null || true
    rm -f "${LOG_FILE}.make" 2>/dev/null || true
    cecho "green" "✓ Cleanup complete"
    echo

    INSTALLATION_FAILED=0  # Installation succeeded

    # --- Success Message ---
    cecho "green" "╔═════════════════════════════════════════════════════╗"
    cecho "green" "║                                                     ║"
    cecho "green" "║            ✅ INSTALLATION COMPLETE! ✅            ║"
    cecho "green" "║                                                     ║"
    cecho "green" "╚═════════════════════════════════════════════════════╝"
    echo
    log "=== Installation completed successfully ==="

    cecho "magenta" "🎵 Your AirPlay 2 device is ready!"
    echo
    cecho "yellow" "  📱 Device Name:  $airplay_name"
    cecho "yellow" "  🔊 Audio Output: $audio_device_plug"
    cecho "yellow" "  🎚️  Volume:       ${mixer_control:-Fixed (no hardware control)}"
    echo
    cecho "blue" "┌─────────────────────────────────────────────────────┐"
    cecho "blue" "│ How to use:                                         │"
    cecho "blue" "│ 1. Open Music/Spotify/YouTube on your iPhone/iPad  │"
    cecho "blue" "│ 2. Tap the AirPlay icon (📡)                       │"
    cecho "blue" "│ 3. Select '$airplay_name'                     │"
    cecho "blue" "│ 4. Enjoy high-quality wireless audio!              │"
    cecho "blue" "└─────────────────────────────────────────────────────┘"
    echo
    cecho "yellow" "💡 Tips:"
    cecho "yellow" "   • Device should appear within 30 seconds after reboot"
    cecho "yellow" "   • Make sure iPhone and Pi are on the same Wi-Fi network"
    cecho "yellow" "   • For best quality, use lossless audio sources"
    if [ "$disable_wifi_pm" = true ]; then
        echo
        cecho "yellow" "📝 IMPORTANT - After reboot:"
        cecho "yellow" "   Don't forget to disable Wi-Fi power management using raspi-config!"
        cecho "yellow" "   This prevents audio dropouts and stuttering."
    fi
    echo
    cecho "blue" "📋 Useful commands:"
    cecho "blue" "   View live logs:    sudo journalctl -u shairport-sync -f"
    cecho "blue" "   Restart service:   sudo systemctl restart shairport-sync"
    cecho "blue" "   Check status:      sudo systemctl status shairport-sync"
    cecho "blue" "   Edit config:       sudo nano /etc/shairport-sync.conf"
    cecho "blue" "   Installation log:  $LOG_FILE"
    echo

    read -p "Press Enter to reboot now (recommended), or Ctrl+C to reboot later..." || {
        echo
        cecho "yellow" "Reboot cancelled. Remember to reboot later with: sudo reboot"
        exit 0
    }

    log "User initiated reboot"
    cecho "yellow" "Rebooting in 3 seconds..."
    sleep 3
    sudo reboot
}

# --- Script Entry Point ---
main "$@"
