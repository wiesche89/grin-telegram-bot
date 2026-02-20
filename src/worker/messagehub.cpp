#include "messagehub.h"

#include "ggcworker.h"
#include "tippingworker.h"
#include "telegrambot.h"

MessageHub::MessageHub(TelegramBot *bot, TippingWorker *tippingWorker, GgcWorker *ggcWorker, QObject *parent) :
    QObject(parent),
    m_bot(bot),
    m_tippingWorker(tippingWorker),
    m_ggcWorker(ggcWorker)
{
    if (m_bot) {
        connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onBotMessage(TelegramBotUpdate)));
    }
}

void MessageHub::onBotMessage(TelegramBotUpdate update)
{

        qDebug()<<"from: "<<update->message->from.firstName<<"  "<<update->message->text;

    if (!update || update.isNull()) {
        return;
    }

    if (m_tippingWorker && m_tippingWorker->handleUpdate(update)) {
        qDebug()<<"m_tippingWorker handle message";
        return;
    }

    if (m_ggcWorker) {
        qDebug()<<"message go to ggcworker";
        m_ggcWorker->handleUpdate(update);
    }
}
