#!/bin/bash
set -e  # Skript beenden, wenn ein Befehl fehlschlägt
set -x  # Alle Befehle anzeigen (Debug-Modus)

export QT_QPA_PLATFORM=offscreen
export QT_PLUGIN_PATH=/usr/lib/x86_64-linux-gnu/qt6/plugins

echo "[INFO] Startskript ausgeführt"
echo "[INFO] DATA_DIR = $DATA_DIR"

# Prüfe ob DATA_DIR existiert
if [ ! -d "$DATA_DIR" ]; then
    echo "[ERROR] DATA_DIR '$DATA_DIR' existiert nicht"
    exit 1
fi

# .grin-Verzeichnis verlinken
if [ -d "$DATA_DIR/.grin" ]; then
    echo "[INFO] .grin-Verzeichnis gefunden, verlinke nach /root/.grin"
    ln -sf "$DATA_DIR/.grin" /root/.grin
else
    echo "[WARN] Kein .grin-Verzeichnis in $DATA_DIR gefunden"
fi

# Fixe Tor-Onion-Service-Rechte
if [ -d "/root/.grin/main/tor/listener/onion_service_addresses" ]; then
    echo "[INFO] Setze Rechte für Tor onion_service_addresses"
    find /root/.grin/main/tor/listener/onion_service_addresses -type d -exec chmod 700 {} \;
else
    echo "[WARN] Onion-Service-Verzeichnis existiert nicht (noch nicht erstellt?)"
fi

# Starte grin-wallet listen im Hintergrund
if [ -f "$DATA_DIR/.wallet/password.txt" ]; then
    echo "[INFO] Starte grin-wallet mit Passwortdatei..."
    cat "$DATA_DIR/.wallet/password.txt" | ./grin-wallet listen &
else
    echo "[ERROR] Passwortdatei nicht gefunden: $DATA_DIR/.wallet/password.txt"
    exit 1
fi

# Warte auf Tor-Startup
echo "[INFO] Warte 30 Sekunden auf Tor/wallet..."
sleep 30

# Starte den Telegram Bot über strace
if [ -x ./grin-telegram-bot ]; then
    echo "[INFO] Starte grin-telegram-bot mit strace..."
    strace -f -o /tmp/grinbot_strace.log ./grin-telegram-bot
else
    echo "[ERROR] grin-telegram-bot nicht gefunden oder nicht ausführbar!"
    ls -la .
    exit 1
fi