#!/bin/bash

# Quick pre-installation check script
# Run this before the main installer to verify your system is ready

echo "╔═════════════════════════════════════════════════════╗"
echo "║   AirPlay 2 Pre-Installation Check                 ║"
echo "╚═════════════════════════════════════════════════════╝"
echo

CHECKS_PASSED=0
CHECKS_FAILED=0
WARNINGS=0

check_pass() {
    echo "✓ $1"
    ((CHECKS_PASSED++))
}

check_fail() {
    echo "✗ $1"
    ((CHECKS_FAILED++))
}

check_warn() {
    echo "⚠ $1"
    ((WARNINGS++))
}

echo "Running system checks..."
echo

# Check 1: Not running as root
if [ "$EUID" -eq 0 ]; then
    check_fail "Running as root - please run as normal user"
else
    check_pass "Running as normal user"
fi

# Check 2: Can sudo
if sudo -n true 2>/dev/null || sudo true 2>/dev/null; then
    check_pass "Sudo access available"
else
    check_fail "Cannot use sudo"
fi

# Check 3: Internet
TEST_HOSTS=("8.8.8.8" "1.1.1.1" "github.com")
INTERNET_OK=0
for host in "${TEST_HOSTS[@]}"; do
    # Try twice with longer timeout for slow Pi Zero networks
    ping -c 2 -W 5 -i 0.5 "$host" >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        INTERNET_OK=1
        break
    fi
    sleep 1
done

if [ $INTERNET_OK -eq 1 ]; then
    check_pass "Internet connection working"
else
    check_warn "Internet check inconclusive (may be slow network)"
    echo "  Verify manually: ping -c 3 8.8.8.8"
fi

# Check 4: Disk space
SPACE=$(df / | tail -1 | awk '{print $4}')
if [ "$SPACE" -gt 1000000 ]; then
    check_pass "Disk space: $((SPACE / 1024)) MB available"
else
    check_fail "Insufficient disk space: $((SPACE / 1024)) MB (need 1000+ MB)"
fi

# Check 5: Memory
MEM=$(free -m | awk '/^Mem:/{print $7}')
if [ "$MEM" -gt 100 ]; then
    check_pass "Available memory: $MEM MB"
else
    check_warn "Low memory: $MEM MB (may be slow)"
fi

# Check 6: Audio devices
echo
echo "Audio devices found:"
if aplay -l 2>/dev/null | grep -q "card"; then
    aplay -l 2>/dev/null | grep "^card" | while read line; do
        echo "  → $line"
    done
    check_pass "Audio devices detected"
else
    check_fail "No audio devices found"
fi

# Check 7: USB DAC
echo
USB_DAC=$(aplay -l 2>/dev/null | grep "^card" | grep -iv "bcm2835\|Headphones\|vc4-hdmi" || true)
if [ -n "$USB_DAC" ]; then
    echo "External audio devices:"
    echo "$USB_DAC" | while read line; do
        echo "  → $line"
    done
    check_pass "USB DAC or external audio found"
else
    check_warn "No USB DAC detected (only built-in audio)"
    echo "  Connect a USB DAC for better quality"
fi

# Check 8: Pi model
echo
if [ -f /proc/device-tree/model ]; then
    PI_MODEL=$(tr -d '\0' < /proc/device-tree/model)
    echo "Device: $PI_MODEL"
    if echo "$PI_MODEL" | grep -qE "Pi Zero W|Pi 1"; then
        check_warn "Old Pi model - may not work well with AirPlay 2"
    else
        check_pass "Compatible Pi model"
    fi
else
    check_warn "Not a Raspberry Pi"
fi

# Check 9: Wi-Fi
echo
if ip link show wlan0 &>/dev/null; then
    WIFI_STATE=$(ip link show wlan0 | grep -o "state [A-Z]*" | awk '{print $2}')
    if [ "$WIFI_STATE" = "UP" ]; then
        check_pass "Wi-Fi interface active"
    else
        check_warn "Wi-Fi interface down"
    fi
else
    check_warn "No Wi-Fi interface found (wlan0)"
fi

# Summary
echo
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo "Summary:"
echo "  ✓ Passed:   $CHECKS_PASSED"
echo "  ✗ Failed:   $CHECKS_FAILED"
echo "  ⚠ Warnings: $WARNINGS"
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo

if [ $CHECKS_FAILED -eq 0 ]; then
    echo "✅ System ready for installation!"
    echo
    echo "To install, run:"
    echo "  bash install_airplay_v3.sh"
    exit 0
else
    echo "❌ Please fix the failed checks before installing"
    exit 1
fi
