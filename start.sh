#!/bin/bash

# Basisdatenverzeichnis (kann per ENV DATA_DIR gesetzt werden, sonst Default)
DATA_DIR="${DATA_DIR:-/opt/grin-telegram-bot/data}"

# Fix permissions for Tor onion service directories (falls vorhanden)
find "$DATA_DIR/.grin/main/tor/listener/onion_service_addresses" -type d -exec chmod 700 {} \; 2>/dev/null

# Wallet-Verzeichnis
WALLET_DIR="$DATA_DIR/.wallet"

# Starte grin-wallet listen mit Passwort aus Volume (Background)
if [ -f "$WALLET_DIR/password.txt" ]; then
    cat "$WALLET_DIR/password.txt" | ./grin-wallet listen -d "$WALLET_DIR" &
else
    echo "Kein Passwort gefunden unter $WALLET_DIR/password.txt"
    exit 1
fi

# Warte 30 Sekunden auf Tor/Wallet-Verbindung
sleep 30

# Starte den Telegram-Bot
exec ./grin-telegram-bot "$DATA_DIR"
