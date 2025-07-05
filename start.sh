#!/bin/bash

# Fix permissions for Tor onion service directories
find /root/.grin/main/tor/listener/onion_service_addresses -type d -exec chmod 700 {} \;

# Read password from file and pipe it to grin-wallet listen, running in background
cat /opt/grin-telegram-bot/.wallet/password.txt | ./grin-wallet listen &

# Wait 30 seconds before starting the telegram bot
sleep 30

# Start the grin-telegram-bot
./grin-telegram-bot