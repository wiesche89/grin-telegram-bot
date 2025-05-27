# GRIN Tipping Bot Bounty Proposal
by wiesche (copy from renzokuken)

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

The response message template should be customizable.

### Deposit protocol

1. User runs `/donate` command.
2. Bot send a DM with a slatepack address message and requests to send a slatepack.
3. User that has to initiate the SRS flow in the own wallet and send the slatepack to the group. The bot will automatically consider slatepacks that do not reply to any messages as donations.
4. If slatepack was correct, the bot will respond with another slatepack and request the user to run the `send` command in their wallet. Otherwise, it will inform the slatepack was not correct.
5. User will run the send command in their wallet.
6. Bot will automatically inform the user when payment is processed by tagging the user in the group. It will also send a message to the group informing of successful donation.

### Withdrawal

1. User interested in getting a withdrawal would need to send a message to the group that states why it would be cool for this user to receive GRIN.
2. At any moment, the user from (1) can respond to their message with `/faucet` command, then the bot will inform the user if qualifies for the faucet withdrawal or not. If qualifies, the bot will send a DM requesting the user's address to encrypt slatepack. After receiving the address it will respond with an SRS flow slatepack. If user did not qualify for withdrawal, it will inform what is necessary (for instance more responses to the original message containing specific keywords or emojis from other users, etc).
3. User runs `receive` command in own wallet with slatepack stated in (2). Responds to message from (2) with own slatepack resulting from `receive` command.
4. If correct, the bot will broadcast the transaction and inform the user of it. It will also send a message to the group informing of the successful withdrawal. If not, it will inform you that the slatepack was not correct.

### Customized approval mechanisms

Users requesting the faucet would need to provide a message and the bot will review if that message qualifies. Possible criteria are
1. Number of responses quoting the message from other users in the group.
2. Presence/absence of particular keywords/emojis in the response messages.
3. Author of the message satisfying a particular amount of time in the group, having several messages sent, profile photo, etc.

### Admin settings

The bot would need to be configurable without restarting. The owner of the group should be able to:

1. Enable/disable deposits
2. Enable/disable withdrawals
3. Update bot response messages templates
4. Set the required number of responses to approve the withdrawal
5. Set the profile requirements to approve the withdrawal
6. Set the approved withdrawal amount

For security reasons, Owner API settings would need to be provided via a config file and not using the admin UI.
