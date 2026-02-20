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
    if (!update || update.isNull()) {
        qDebug() << "MessageHub::onBotMessage - empty update";
        return;
    }

    if (!update->message) {
        qDebug() << "MessageHub::onBotMessage - update without message, type" << update->type;
        return;
    }

    qDebug() << "MessageHub::onBotMessage - from:" << update->message->from.firstName << update->message->text;

    if (m_tippingWorker && m_tippingWorker->handleUpdate(update)) {
        qDebug() << "MessageHub::onBotMessage - tipping worker handled update";
        return;
    }

    if (m_ggcWorker) {
        qDebug() << "MessageHub::onBotMessage - handing over to GGC worker";
        m_ggcWorker->handleUpdate(update);
    }
}
