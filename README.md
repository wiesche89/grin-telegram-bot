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

## Linux

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

```bash
sudo snap install docker
```

---

### 4. Navigate to the Bot Repository

```bash
cd grin-telegram-bot/
```

---

### 5. Build Using Docker Compose

```bash
sudo docker-compose build
```

### 6. Run Using Docker Compose
```bash
sudo docker-compose up -d
```

### 7. Get Container id
```bash
sudo docker container ls
```

### 8. Go into Docker logs (first 3 letter)
```bash
sudo docker logs -f <container id>
```