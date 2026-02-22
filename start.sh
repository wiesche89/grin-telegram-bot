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
    rm -rf /root/.grin
    ln -s "$DATA_DIR/.grin" /root/.grin
else
    echo "[WARN] No .grin directory found in $DATA_DIR"
fi

# Fix Tor onion service permissions
CHAIN_DIR="main"
[ "$GRIN_CHAIN_TYPE" = "testnet" ] && CHAIN_DIR="test"

if [ -d "/root/.grin/${CHAIN_DIR}/tor/listener/onion_service_addresses" ]; then
  find "/root/.grin/${CHAIN_DIR}/tor/listener/onion_service_addresses" -type d -exec chmod 700 {} \;
else
  echo "[WARN] Onion service directory does not exist (not yet created?)"
fi

# Start grin-wallet listen in the background
if [ -f "$DATA_DIR/.wallet/password.txt" ]; then
    echo "[INFO] Starting grin-wallet using password file..."

    WALLET_ARGS=("listen")
    if [ "$GRIN_CHAIN_TYPE" = "testnet" ]; then
        WALLET_ARGS=("--testnet" "listen")
    fi

    ./grin-wallet "${WALLET_ARGS[@]}" < "$DATA_DIR/.wallet/password.txt" &
else
    echo "[ERROR] Password file not found: $DATA_DIR/.wallet/password.txt"
    exit 1
fi

# Wait for Tor/wallet to start
echo "[INFO] Waiting 10 seconds for Tor/wallet to initialize..."
sleep 10

# Start the Telegram bot (no strace)
if [ -x ./grin-telegram-bot ]; then
    echo "[INFO] Starting grin-telegram-bot..."
    ./grin-telegram-bot
else
    echo "[ERROR] grin-telegram-bot not found or not executable!"
    ls -la .
    exit 1
fi
