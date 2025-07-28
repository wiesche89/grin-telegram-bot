#include "tippingworker.h"
#include <QTimer>
#include <QDateTime>
#include <QDebug>

TippingWorker::TippingWorker(TelegramBot *bot, QSettings *settings) :
    m_bot(bot),
    m_settings(settings),
    m_db(new TippingDatabase("tipping.db", this))
{
    m_db->initialize();

    // Cleanup Timer für veraltete Spiele
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
    cleanupTimer->start(60000); // alle 60 Sekunden prüfen
}

bool TippingWorker::init()
{

    // Set Slot to bot message
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));

    return true;
}

bool TippingWorker::isPlayerInGame(const QString &user) const
{
    for (auto *game : m_activeGames) {
        if (!game->isFinished() && (game->player1 == user || game->player2 == user)) {
            return true;
        }
    }
    return false;
}

void TippingWorker::onMessage(TelegramBotUpdate update)
{
    if (update->type != TelegramBotMessageType::Message) {
        return;
    }
    TelegramBotMessage &message = *update->message;

    const QString text = message.text.trimmed();

    qDebug()<<"text: "<<text;

    const QStringList parts = text.split(' ', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return;
    }

    const QString cmd = parts[0];
    const QString sender = message.from.username;

    if (cmd == "/deposit" && parts.size() == 2) {
        int amount = parts[1].toInt();
        sendUserMessage(message, handleDeposit(sender, amount));
        return;
    }

    if (cmd == "/withdraw" && parts.size() == 2) {
        int amount = parts[1].toInt();
        sendUserMessage(message, handleWithdraw(sender, amount));
        return;
    }

    if (cmd == "/tip" && parts.size() == 3) {
        QString toUser = parts[1];
        if (toUser.startsWith("@")) {
            toUser = toUser.mid(1);
        }
        int amount = parts[2].toInt();
        sendUserMessage(message, handleTip(sender, toUser, amount));
        return;
    }

    if (cmd == "/blackjack" && parts.size() == 3) {
        QString toUser = parts[1];
        if (toUser.startsWith("@")) {
            toUser = toUser.mid(1);
        }
        int amount = parts[2].toInt();

        if (toUser == "bot") {
            if (m_db->getBalance(sender) < amount) {
                sendUserMessage(message, "Nicht genug Guthaben gegen den Bot.");
                return;
            }

            m_db->updateBalance(sender, -amount);
            m_db->recordTransaction(sender, "bot", amount, "stake", "blackjack");

            BlackjackGame *game = new BlackjackGame(sender, "bot", amount);
            game->start();

            QString key = makeGameKey(sender, "bot");
            m_activeGames.insert(key, game);

            sendUserMessage(message, QString("Du spielst gegen den Bot um %1 GRIN. Nutze /hit oder /stand.").arg(amount));
            return;
        }

        sendUserMessage(message, handleBlackjackRequest(sender, toUser, amount));
        return;
    }

    if (cmd == "/hit") {
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (game->getCurrentPlayer() == sender && !game->isFinished()) {
                bool busted = game->hit(sender);
                sendUserMessage(message, game->getHandText(sender));

                if (busted) {
                    sendUserMessage(message, "Bust! Du hast verloren.");
                }

                if (game->isFinished()) {
                    QString result = QString("Spiel beendet. Gewinner: %1").arg(game->determineWinner());
                    sendUserMessage(message, result);

                    QString winner = game->determineWinner();
                    if (winner != "Unentschieden") {
                        m_db->updateBalance(winner, game->stake);
                        QString loser = (winner == game->player1) ? game->player2 : game->player1;
                        if (loser != "bot") {
                            m_db->updateBalance(loser, -game->stake);
                        }
                        m_db->recordTransaction(loser, winner, game->stake, "game", "blackjack");
                    }
                } else if (game->getCurrentPlayer() == "bot") {
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

    if (cmd == "/stand") {
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (game->getCurrentPlayer() == sender && !game->isFinished()) {
                game->stand(sender);
                sendUserMessage(message, "Du stehst.");

                if (game->getCurrentPlayer() == "bot") {
                    int botPoints = game->calculatePoints(game->getCards("bot"));
                    while (botPoints < 17) {
                        game->hit("bot");
                        botPoints = game->calculatePoints(game->getCards("bot"));
                    }
                    game->stand("bot");
                }

                if (game->isFinished()) {
                    QString result = QString("Spiel beendet. Gewinner: %1").arg(game->determineWinner());
                    sendUserMessage(message, result);

                    QString winner = game->determineWinner();
                    if (winner != "Unentschieden") {
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

    if (cmd == "/mybalance") {
        int bal = m_db->getBalance(sender);
        sendUserMessage(message, QString("Dein aktuelles Guthaben: %1 GRIN").arg(bal));
        return;
    }

    if (cmd == "/cancelgame") {
        QStringList toRemove;
        for (auto it = m_activeGames.begin(); it != m_activeGames.end(); ++it) {
            BlackjackGame *game = it.value();
            if (!game->isFinished() && (game->player1 == sender || game->player2 == sender)) {
                sendUserMessage(message, "Spiel wurde abgebrochen.");
                toRemove << it.key();
                delete game;
            }
        }
        for (const QString &key : toRemove) {
            m_activeGames.remove(key);
        }
        return;
    }

    if (cmd == "/games") {
        QStringList list;
        for (auto *game : m_activeGames) {
            list << QString("@%1 vs @%2 (%3 GRIN)").arg(game->player1, game->player2).arg(game->stake);
        }
        if (list.isEmpty()) {
            sendUserMessage(message, "Keine laufenden Spiele.");
        } else {
            sendUserMessage(message, "Laufende Spiele:\n" + list.join("\n"));
        }
        return;
    }
}

QString TippingWorker::handleDeposit(const QString &user, int amount)
{
    m_db->updateBalance(user, amount);
    m_db->recordTransaction("", user, amount, "deposit");
    return QString("Slatepack erstellt für Einzahlung von %1 GRIN.").arg(amount);
}

QString TippingWorker::handleWithdraw(const QString &user, int amount)
{
    if (m_db->getBalance(user) < amount) {
        return "Nicht genügend Guthaben.";
    }
    m_db->updateBalance(user, -amount);
    m_db->recordTransaction(user, "", amount, "withdraw");
    return QString("Slatepack erstellt für Auszahlung von %1 GRIN.").arg(amount);
}

QString TippingWorker::handleTip(const QString &fromUser, const QString &toUser, int amount)
{
    if (m_db->getBalance(fromUser) < amount) {
        return "Nicht genügend Guthaben.";
    }
    m_db->updateBalance(fromUser, -amount);
    m_db->updateBalance(toUser, amount);
    m_db->recordTransaction(fromUser, toUser, amount, "tip");
    return QString("%1 GRIN an @%2 gesendet.").arg(amount).arg(toUser);
}

QString TippingWorker::handleBlackjackRequest(const QString &fromUser, const QString &toUser, int amount)
{
    if (fromUser == toUser) {
        return "Du kannst nicht gegen dich selbst spielen.";
    }
    if (m_db->getBalance(fromUser) < amount) {
        return "Nicht genug Guthaben für Einsatz.";
    }
    if (m_db->getBalance(toUser) < amount) {
        return QString("@%1 hat möglicherweise nicht genug Guthaben.").arg(toUser);
    }
    if (isPlayerInGame(fromUser) || isPlayerInGame(toUser)) {
        return "Einer der Spieler ist bereits in einem Spiel aktiv.";
    }

    QString key = makeGameKey(fromUser, toUser);
    if (m_activeGames.contains(key)) {
        return "Es läuft bereits ein Spiel zwischen euch.";
    }

    BlackjackGame *game = new BlackjackGame(fromUser, toUser, amount);
    game->start();
    m_activeGames.insert(key, game);

    sendUserMessage(fromUser, QString("Spiel gestartet gegen @%1 um %2 GRIN. Du bist dran.").arg(toUser).arg(amount));
    sendUserMessage(toUser, QString("@%1 hat ein Spiel um %2 GRIN gestartet. Nutze /hit oder /stand.").arg(fromUser).arg(amount));

    return "Spiel wurde initialisiert.";
}

QString TippingWorker::makeGameKey(const QString &user1, const QString &user2)
{
    QStringList users = { user1.toLower(), user2.toLower() };
    users.sort();
    return users.join(":");
}

void TippingWorker::sendUserMessage(QString user, QString content)
{
    qDebug() << "Nachricht an " << user << ": " << content;
    // Hier ggf. Nutzer-zu-ChatID Mapping verwenden
}

void TippingWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi "
                      + message.from.firstName
                      + ",\n"
                      + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}
