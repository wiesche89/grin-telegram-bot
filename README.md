# GRIN Tipping Bot Bounty Proposal
by wiesche

## Introduction

This document aims to technically define the requirements for a Telegram bot that can deposit and withdraw GRIN digital cash cryptocurrency.

## Use cases

Such a bot could serve more than one purpose.

### GRIN Faucet

To popularize GRIN digital cash currency, some small amount of coins could be distributed free of charge to users who satisfy certain predefined criteria. 
Such as sending a message, getting a certain number of reactions, etc. In this case, the owner of the group would need to enable both deposit and withdrawal features.

### Collecting donations

If someone runs a Telegram group and would like to accept donations in GRIN. In this case, only the deposit feature would be enabled.

## Technical discussion

The bot would use the Owner API. It would load the credentials and API endpoint from a config file provided in the process launch command arguments.

## User experience

### Getting the address

1. User runs `/address` command.
2. Bot replies with slatepack address.

### Deposit protocol

1. User runs '/donate' to get manual
2. Send a Slatepack to donate GRIN
3. Bot send repsonse Slatepack
4. Finalize
5. Bot will automatically inform the user when payment is processed by tagging the user in the group. It will also send a message to the group informing of successful donation.

### Withdrawal

1. User runs '/faucet' to get manual
2. Send a Slatepack to receive GRIN
3. Bot send repsonse Slatepack
4. Finalize
5. If correct, the bot will broadcast the transaction and inform the user of it. It will also send a message to the group informing of the successful withdrawal. If not, it will inform you that the slatepack was not correct.

### Admin settings

The bot would need to be configurable without restarting. The owner of the group should be able to:

1. Enable/disable deposits
2. Enable/disable withdrawals
3. Update bot response messages templates
4. Set the required number of responses to approve the withdrawal
5. Set the profile requirements to approve the withdrawal
6. Set the approved withdrawal amount

For security reasons, Owner API settings would need to be provided via a config file and not using the admin UI.

Workflow
![grin-telegram-bot](https://github.com/user-attachments/assets/19f69736-02e3-4aec-8cc0-8e3f1f2c3222)




# Grin Telegram Bot - Build Instructions

## Windows

###Develop

### 1. Instal Qt 
install Qt 6.9

MSYS2 installieren

## instal secp256k1
pacman -Syu    # einmal komplett aktualisieren, danach Shell neu starten
pacman -S --needed base-devel mingw-w64-x86_64-toolchain autoconf automake libtool
pacman -S git


git clone https://github.com/bitcoin-core/secp256k1.git

cd secp236k1

make distclean   # wenn du schonmal gebaut hast

./autogen.sh
./configure --enable-module-ecdh --enable-module-recovery \
            --enable-shared --disable-static \
            --host=x86_64-w64-mingw32
make -j$(nproc)


### 2. Check Enviroments
Currently the bot doesnt create a wallet.
Copy the following directory structure and files **into the project root**:

```
.grin/
└── main/
    ├── grin-wallet.toml
    ├── .foreing_api_secret
    ├── .owner_api_secret
    └── wallet_data/

.wallet/
└── password.txt

etc/
├── database/
│   └── database.db
├── messages/
│   └── start.txt
└── settings.ini
```

Settings.ini are not to setup

### 2. Build
Use Qt Creator to build bot from sources

### Build and Deploy (Linux/Windows)

### 1. Clone the Bot Repository

```bash
git clone https://github.com/wiesche89/grin-telegram-bot.git
```

---

### 2. Prepare Required Files

Currently the bot doesnt create a wallet.
Copy the following directory structure and files **into the project root**:

```
.grin/
└── main/
    ├── grin-wallet.toml
    ├── .foreing_api_secret
    ├── .owner_api_secret
    └── wallet_data/

.wallet/
└── password.txt

etc/
├── database/
│   └── database.db
├── messages/
│   └── start.txt
└── settings.ini
```

---

### 3. Install Docker

If you don't have Docker installed:

#### Linux
```bash
sudo snap install docker
```

#### Windows
Install Docker Desktop
---

### 4. Navigate to the Bot Repository

```bash
cd grin-telegram-bot/
```

---

### 5. Build & Run with Docker Compose (Mainnet / Testnet)

This repository contains **two** compose files:

- `docker-compose.yml` → **Mainnet**
- `docker-compose-testnet.yml` → **Testnet**

> Note: Both compose files use the same `container_name` (`grin-telegram-bot-container`).  
> Run **either mainnet or testnet**, not both at the same time (unless you change the container name and ports).

#### Mainnet: build + start

```bash
sudo docker compose -f docker-compose.yml up -d --build
```

View logs:

```bash
sudo docker logs -f grin-telegram-bot-container
```

Stop:

```bash
sudo docker compose -f docker-compose.yml down
```

#### Testnet: build + start

```bash
sudo docker compose -f docker-compose-testnet.yml up -d --build
```

View logs:

```bash
sudo docker logs -f grin-telegram-bot-container
```

Stop:

```bash
sudo docker compose -f docker-compose-testnet.yml down
```

#### Persistent data directory

Both compose files bind-mount the same host directory:

- Host: `~/grin-telegram-bot-data`
- Container: `/opt/grin-telegram-bot/data`

Expected structure inside `~/grin-telegram-bot-data`:

```
.grin/
  main/   (mainnet wallet + config)
  test/   (testnet wallet + config)
.wallet/
  password.txt
etc/
  database/database.db
  messages/start.txt
  messages/donate.txt
  messages/faucet.txt
  settings.ini
```

Tip: message templates can be copied from `deploy/etc/messages/` into your data directory:

```bash
mkdir -p ~/grin-telegram-bot-data/etc/messages
cp -av ./deploy/etc/messages/. ~/grin-telegram-bot-data/etc/messages/
```
