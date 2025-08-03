#!/bin/bash
set -e  # Exit script if any command fails
set -x  # Print each command before executing (debug mode)


export QT_QPA_PLATFORM=offscreen
export QT_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt6/plugins

echo "[INFO] Startup script executed"
echo "[INFO] DATA_DIR = $DATA_DIR"

# Check if DATA_DIR exists
if [ ! -d "$DATA_DIR" ]; then
    echo "[ERROR] DATA_DIR '$DATA_DIR' does not exist"
    exit 1
fi

# Symlink .grin directory
if [ -d "$DATA_DIR/.grin" ]; then
    echo "[INFO] .grin directory found, linking to /root/.grin"
    ln -sf "$DATA_DIR/.grin" /root/.grin
else
    echo "[WARN] No .grin directory found in $DATA_DIR"
fi

# Fix Tor onion service permissions
if [ -d "/root/.grin/main/tor/listener/onion_service_addresses" ]; then
    echo "[INFO] Setting permissions for Tor onion_service_addresses"
    find /root/.grin/main/tor/listener/onion_service_addresses -type d -exec chmod 700 {} \;
else
    echo "[WARN] Onion service directory does not exist (not yet created?)"
fi

# Start grin-wallet listen in the background
if [ -f "$DATA_DIR/.wallet/password.txt" ]; then
    echo "[INFO] Starting grin-wallet using password file..."
    cat "$DATA_DIR/.wallet/password.txt" | ./grin-wallet listen &
else
    echo "[ERROR] Password file not found: $DATA_DIR/.wallet/password.txt"
    exit 1
fi

# Wait for Tor/wallet to start
echo "[INFO] Waiting 30 seconds for Tor/wallet to initialize..."
sleep 30

# Start the Telegram bot with strace
if [ -x ./grin-telegram-bot ]; then
    echo "[INFO] Starting grin-telegram-bot with strace..."
    strace -f -o /tmp/grinbot_strace.log ./grin-telegram-bot
else
    echo "[ERROR] grin-telegram-bot not found or not executable!"
    ls -la .
    exit 1
fi
