#include "tippingworker.h"

/**
 * @brief Constructor for TippingWorker
 * Initializes database and starts cleanup timer
 */
TippingWorker::TippingWorker(TelegramBot *bot, QSettings *settings) :
    m_bot(bot),
    m_settings(settings),
    m_db(nullptr)
{
    // Timer to clean up expired or finished games every 60 seconds
    QTimer *cleanupTimer = new QTimer(this);
    connect(cleanupTimer, &QTimer::timeout, this, [this]() {
        QList<QString> toRemove;
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            if (it.value()->isFinished() || it.value()->startedAt.secsTo(QDateTime::currentDateTime()) > 300) {
                delete it.value();
                toRemove.append(it.key());
            }
        }
        for (const QString &key : toRemove) {
            m_activeGames.remove(key);
        }
    });
    cleanupTimer->start(60000); // every 60 seconds
}

/**
 * @brief Initializes the tipping worker and connects message handler
 */
bool TippingWorker::init()
{
    QString dbPath;
    QString dataDir = qEnvironmentVariable("DATA_DIR");

    if (dataDir.isEmpty()) {
        dbPath = QCoreApplication::applicationDirPath() + "/etc/database/tipping.db";
    }
    else
    {
        dbPath = QDir(dataDir).filePath("etc/database/tipping.db");
    }

    qDebug() << "DB Pfad Tipping:" << dbPath;

    m_db = new TippingDatabase(dbPath, this);
    m_db->initialize();

    // Connect incoming Telegram messages to the handler
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));
    return true;
}

/**
 * @brief Checks if a user is currently in a game
 */
bool TippingWorker::isPlayerInGame(const QString &user) const
{
    for (auto *game : m_activeGames) {
        if (!game->isFinished() && (game->player1 == user || game->player2 == user)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Handles all incoming bot messages
 */
void TippingWorker::onMessage(TelegramBotUpdate update)
{
    if (update->type != TelegramBotMessageType::Message) return;

    TelegramBotMessage &message = *update->message;
    const QString text = message.text.trimmed();
    qDebug() << "text: " << text;

    const QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) return;

    const QString cmd = parts[0];
    const QString sender = message.from.username;

    // Handle /deposit <amount>
    if (cmd == "/deposit" && parts.size() == 2) {
        int amount = parts[1].toInt();
        sendUserMessage(message, handleDeposit(sender, amount));
        return;
    }

    // Handle /withdraw <amount>
    if (cmd == "/withdraw" && parts.size() == 2) {
        int amount = parts[1].toInt();
        sendUserMessage(message, handleWithdraw(sender, amount));
        return;
    }

    // Handle /tip <user> <amount>
    if (cmd == "/tip" && parts.size() == 3) {
        QString toUser = parts[1];
        if (toUser.startsWith("@")) toUser = toUser.mid(1);
        int amount = parts[2].toInt();
        sendUserMessage(message, handleTip(sender, toUser, amount));
        return;
    }

    // Handle /blackjack <user|bot> <amount>
    if (cmd == "/blackjack" && parts.size() == 3) {
        QString toUser = parts[1];
        if (toUser.startsWith("@")) toUser = toUser.mid(1);
        int amount = parts[2].toInt();

        // Bot game logic
        if (toUser == "bot") {
            if (m_db->getBalance(sender) < amount) {
                sendUserMessage(message, "Not enough balance to play against the bot.");
                return;
            }

            m_db->updateBalance(sender, -amount);
            m_db->recordTransaction(sender, "bot", amount, "stake", "blackjack");

            BlackjackGame *game = new BlackjackGame(sender, "bot", amount);
            game->start();

            QString key = makeGameKey(sender, "bot");
            m_activeGames.insert(key, game);

            sendUserMessage(message, QString("You are playing against the bot for %1 GRIN. Use /hit or /stand.").arg(amount));
            return;
        }

        // PvP game logic
        sendUserMessage(message, handleBlackjackRequest(sender, toUser, amount));
        return;
    }

    // Handle /hit command
    if (cmd == "/hit") {
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (game->getCurrentPlayer() == sender && !game->isFinished()) {
                bool busted = game->hit(sender);
                sendUserMessage(message, game->getHandText(sender));

                if (busted) {
                    sendUserMessage(message, "Bust! You lost.");
                }

                if (game->isFinished()) {
                    QString result = QString("Game over. Winner: %1").arg(game->determineWinner());
                    sendUserMessage(message, result);

                    QString winner = game->determineWinner();
                    if (winner != "Draw") {
                        m_db->updateBalance(winner, game->stake);
                        QString loser = (winner == game->player1) ? game->player2 : game->player1;
                        if (loser != "bot") {
                            m_db->updateBalance(loser, -game->stake);
                        }
                        m_db->recordTransaction(loser, winner, game->stake, "game", "blackjack");
                    }
                } else if (game->getCurrentPlayer() == "bot") {
                    // Bot auto-play
                    int botPoints = game->calculatePoints(game->getCards("bot"));
                    while (botPoints < 17) {
                        game->hit("bot");
                        botPoints = game->calculatePoints(game->getCards("bot"));
                    }
                    game->stand("bot");
                }
                return;
            }
        }
    }

    // Handle /stand command
    if (cmd == "/stand") {
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (game->getCurrentPlayer() == sender && !game->isFinished()) {
                game->stand(sender);
                sendUserMessage(message, "You stand.");

                if (game->getCurrentPlayer() == "bot") {
                    // Bot auto-play
                    int botPoints = game->calculatePoints(game->getCards("bot"));
                    while (botPoints < 17) {
                        game->hit("bot");
                        botPoints = game->calculatePoints(game->getCards("bot"));
                    }
                    game->stand("bot");
                }

                if (game->isFinished()) {
                    QString result = QString("Game over. Winner: %1").arg(game->determineWinner());
                    sendUserMessage(message, result);

                    QString winner = game->determineWinner();
                    if (winner != "Draw") {
                        m_db->updateBalance(winner, game->stake);
                        QString loser = (winner == game->player1) ? game->player2 : game->player1;
                        if (loser != "bot") {
                            m_db->updateBalance(loser, -game->stake);
                        }
                        m_db->recordTransaction(loser, winner, game->stake, "game", "blackjack");
                    }
                }
                return;
            }
        }
    }

    // Show user balance
    if (cmd == "/balance") {
        int bal = m_db->getBalance(sender);
        sendUserMessage(message, QString("Your current balance: %1 GRIN").arg(bal));
        return;
    }

    // Cancel ongoing game for user
    if (cmd == "/cancelgame") {
        QStringList toRemove;
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (!game->isFinished() && (game->player1 == sender || game->player2 == sender)) {
                sendUserMessage(message, "Game has been cancelled.");
                toRemove << it.key();
                delete game;
            }
        }
        for (const QString &key : toRemove) {
            m_activeGames.remove(key);
        }
        return;
    }

    // List all active games
    if (cmd == "/games") {
        QStringList list;
        for (auto *game : m_activeGames) {
            list << QString("@%1 vs @%2 (%3 GRIN)").arg(game->player1, game->player2).arg(game->stake);
        }
        if (list.isEmpty()) {
            sendUserMessage(message, "No active games.");
        } else {
            sendUserMessage(message, "Active games:\n" + list.join("\n"));
        }
        return;
    }
}

/**
 * @brief Handles deposit logic
 */
QString TippingWorker::handleDeposit(const QString &user, int amount)
{
    m_db->updateBalance(user, amount);
    m_db->recordTransaction("", user, amount, "deposit");
    return QString("Slatepack created for deposit of %1 GRIN.").arg(amount);
}

/**
 * @brief Handles withdrawal logic
 */
QString TippingWorker::handleWithdraw(const QString &user, int amount)
{
    if (m_db->getBalance(user) < amount) {
        return "Insufficient funds.";
    }
    m_db->updateBalance(user, -amount);
    m_db->recordTransaction(user, "", amount, "withdraw");
    return QString("Slatepack created for withdrawal of %1 GRIN.").arg(amount);
}

/**
 * @brief Handles tipping another user
 */
QString TippingWorker::handleTip(const QString &fromUser, const QString &toUser, int amount)
{
    if (m_db->getBalance(fromUser) < amount) {
        return "Insufficient funds.";
    }
    m_db->updateBalance(fromUser, -amount);
    m_db->updateBalance(toUser, amount);
    m_db->recordTransaction(fromUser, toUser, amount, "tip");
    return QString("%1 GRIN sent to @%2.").arg(amount).arg(toUser);
}

/**
 * @brief Handles the start of a new blackjack game between two users
 */
QString TippingWorker::handleBlackjackRequest(const QString &fromUser, const QString &toUser, int amount)
{
    if (fromUser == toUser) return "You can't play against yourself.";
    if (m_db->getBalance(fromUser) < amount) return "Not enough balance.";
    if (m_db->getBalance(toUser) < amount) return QString("@%1 may not have enough balance.").arg(toUser);
    if (isPlayerInGame(fromUser) || isPlayerInGame(toUser)) return "One of the users is already in a game.";

    QString key = makeGameKey(fromUser, toUser);
    if (m_activeGames.contains(key)) return "A game between you already exists.";

    BlackjackGame *game = new BlackjackGame(fromUser, toUser, amount);
    game->start();
    m_activeGames.insert(key, game);

    sendUserMessage(fromUser, QString("Game started against @%1 for %2 GRIN. It's your turn.").arg(toUser).arg(amount));
    sendUserMessage(toUser, QString("@%1 started a game for %2 GRIN. Use /hit or /stand.").arg(fromUser).arg(amount));

    return "Game initialized.";
}

/**
 * @brief Creates a unique key for a user pair to store in the game map
 */
QString TippingWorker::makeGameKey(const QString &user1, const QString &user2)
{
    QStringList users = { user1.toLower(), user2.toLower() };
    users.sort();
    return users.join(":");
}

/**
 * @brief Sends a message to a Telegram username (not used currently)
 */
void TippingWorker::sendUserMessage(QString user, QString content)
{
    qDebug() << "Message to " << user << ": " << content;
    // You may implement a username → chat ID map here
}

/**
 * @brief Sends a message to a user via Telegram
 */
void TippingWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi " + message.from.firstName + ",\n" + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}
