#include "dexworker.h"
#include <QSqlQuery>
#include <QDateTime>
#include <QSqlError>

DexWorker::DexWorker(TelegramBot *bot, QSettings *settings, QObject *parent)
    : QObject(parent), m_bot(bot), m_settings(settings)
{
    m_db = new DexDatabase("dex.db", this);
}

bool DexWorker::init()
{
    if (!m_db->initialize()) {
        qWarning() << "DEX DB init failed";
        return false;
    }

    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));
    return true;
}

void DexWorker::onMessage(TelegramBotUpdate update)
{
    if (update->type != TelegramBotMessageType::Message) return;

    const TelegramBotMessage &msg = *update->message;

    if (msg.text.startsWith("/balance")) {
        handleBalanceCommand(msg);
    } else if (msg.text.startsWith("/order")) {
        handleOrderCommand(msg);
    } else if (msg.text.startsWith("/orders")) {
        handleOrdersCommand(msg);
    } else if (msg.text.startsWith("/cancelorder")) {
        handleCancelCommand(msg);
    }
}

void DexWorker::handleBalanceCommand(const TelegramBotMessage &message)
{
    QSqlQuery q(m_db->database());
    q.prepare("SELECT grin_balance, btc_balance FROM balances WHERE user_id = ?");
    q.addBindValue(message.from.id);

    if (q.exec() && q.next()) {
        int grin = q.value(0).toInt();
        int btc = q.value(1).toInt();
        sendUserMessage(message, QString("💰 Your Balances:\nGRIN: %1\nBTC: %2 sats").arg(grin).arg(btc));
    } else {
        sendUserMessage(message, "❌ You have no balance entry.");
    }
}

void DexWorker::handleOrderCommand(const TelegramBotMessage &message)
{
    // /order buy grin 1 btc 1000
    const QStringList parts = message.text.split(" ");
    if (parts.size() != 6) {
        sendUserMessage(message, "❌ Usage: /order buy/sell grin <amount> btc <price>");
        return;
    }

    QString type = parts[1].toLower();
    QString base = parts[2].toLower();
    int amount = parts[3].toInt();
    QString quote = parts[4].toLower();
    int price = parts[5].toInt();

    if ((type != "buy" && type != "sell") || base != "grin" || quote != "btc" || amount <= 0 || price <= 0) {
        sendUserMessage(message, "❌ Invalid parameters.");
        return;
    }

    QSqlQuery q(m_db->database());

    // Eintrag erzeugen (kein Matching an dieser Stelle)
    q.prepare("INSERT INTO orders (user_id, type, base, quote, amount, price, status, created_at) "
              "VALUES (?, ?, ?, ?, ?, ?, 'open', ?)");
    q.addBindValue(message.from.id);
    q.addBindValue(type);
    q.addBindValue(base);
    q.addBindValue(quote);
    q.addBindValue(amount);
    q.addBindValue(price);
    q.addBindValue(QDateTime::currentSecsSinceEpoch());

    if (!q.exec()) {
        qWarning() << "Order insert error:" << q.lastError().text();
        sendUserMessage(message, "❌ Failed to place order.");
        return;
    }

    sendUserMessage(message, QString("✅ Order placed:\n%1 %2 GRIN @ %3 sats")
                                 .arg(type.toUpper()).arg(amount).arg(price));
}

void DexWorker::handleOrdersCommand(const TelegramBotMessage &message)
{
    QSqlQuery q(m_db->database());
    q.prepare("SELECT id, type, amount, price, status FROM orders WHERE user_id = ? ORDER BY created_at DESC LIMIT 10");
    q.addBindValue(message.from.id);

    if (!q.exec()) {
        sendUserMessage(message, "❌ Failed to load orders.");
        return;
    }

    QStringList lines;
    lines << "📄 Your recent orders:";

    while (q.next()) {
        int id = q.value(0).toInt();
        QString type = q.value(1).toString().toUpper();
        int amount = q.value(2).toInt();
        int price = q.value(3).toInt();
        QString status = q.value(4).toString();

        lines << QString("ID %1: %2 %3 GRIN @ %4 sats [%5]").arg(id).arg(type).arg(amount).arg(price).arg(status);
    }

    if (lines.size() == 1)
        lines << "No orders found.";

    sendUserMessage(message, lines.join("\n"));
}

void DexWorker::handleCancelCommand(const TelegramBotMessage &message)
{
    // /cancelorder <id>
    QStringList parts = message.text.split(" ");
    if (parts.size() != 2) {
        sendUserMessage(message, "❌ Usage: /cancelorder <id>");
        return;
    }

    int orderId = parts[1].toInt();

    QSqlQuery q(m_db->database());
    q.prepare("UPDATE orders SET status = 'canceled' WHERE id = ? AND user_id = ?");
    q.addBindValue(orderId);
    q.addBindValue(message.from.id);

    if (q.exec() && q.numRowsAffected() > 0) {
        sendUserMessage(message, QString("✅ Order %1 canceled.").arg(orderId));
    } else {
        sendUserMessage(message, QString("❌ Could not cancel order %1.").arg(orderId));
    }
}

void DexWorker::sendUserMessage(const TelegramBotMessage &message, const QString &text, bool plain)
{
    QString msg = plain ? text : QString("Hi %1,\n%2").arg(message.from.firstName, text);
    m_bot->sendMessage(message.chat.id, msg, 0, TelegramBot::NoFlag, TelegramKeyboardRequest(), nullptr);
}
