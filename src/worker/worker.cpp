#include "worker.h"

/**
 * @brief Worker::Worker
 */
Worker::Worker()
{
}

/**
 * @brief Worker::initBot
 */
void Worker::initBot()
{
    QSettings settings(QDir::currentPath() + "/release/etc/settings.ini", QSettings::IniFormat);
    m_bot = new TelegramBot(settings.value("bot/token").toString());
    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));
    m_bot->startMessagePulling();
}

/**
 * @brief Worker::onMessage
 * @param update
 */
void Worker::onMessage(TelegramBotUpdate update)
{
    // only handle Messages
    if (update->type != TelegramBotMessageType::Message)
    {
        return;
    }

    // simplify message access
    TelegramBotMessage &message = *update->message;

    /// following commands exists
    /**
    time - get current time
    address - get slatepack address
    donate - dm slatepack address
    faucet - use faucet
    */

    qlonglong id = message.chat.id;

    qDebug() << "id: " << id;
    qDebug() << "message: " << message.text;
    qDebug() << "from: " << message.from.firstName;
    qDebug() << "from Id: " << message.from.id;

    // --------------------------------------------------------------------------------------------------------------------------------------
    // command time
    // --------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/time"))
    {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\nthis is the current time: " + QDateTime::currentDateTime().toString(),
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }

    // --------------------------------------------------------------------------------------------------------------------------------------
    // command address
    // --------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/address"))
    {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\nslatepack address",
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }

    // --------------------------------------------------------------------------------------------------------------------------------------
    // command donate
    // --------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/donate"))
    {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\nthanks for donation!",
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }

    // --------------------------------------------------------------------------------------------------------------------------------------
    // command faucet
    // --------------------------------------------------------------------------------------------------------------------------------------
    if (message.text.contains("/faucet"))
    {
        m_bot->sendMessage(id,
                           "Hi " + message.from.firstName + ",\nhere one GRIN",
                           0,
                           TelegramBot::NoFlag,
                           TelegramKeyboardRequest(),
                           nullptr);

        return;
    }
}
