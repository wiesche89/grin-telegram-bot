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

    // Show user balance
    if (cmd == "/balance") {
        int bal = m_db->getBalance(sender);
        sendUserMessage(message, QString("Your current balance: %1 GRIN").arg(bal));
        return;
    }

}

/**
 * @brief Handles deposit logic
 */
QString TippingWorker::handleDeposit(const QString &user, int amount)
{
    //TODO Create Slatepack



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
