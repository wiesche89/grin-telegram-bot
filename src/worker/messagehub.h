#ifndef MESSAGEHUB_H
#define MESSAGEHUB_H

#include "telegramdatastructs.h"
#include <QObject>

class TelegramBot;
class TippingWorker;
class GgcWorker;

class MessageHub : public QObject
{
    Q_OBJECT

public:
    explicit MessageHub(TelegramBot *bot, TippingWorker *tippingWorker, GgcWorker *ggcWorker, QObject *parent = nullptr);

private slots:
    void onBotMessage(TelegramBotUpdate update);

private:
    TelegramBot *m_bot;
    TippingWorker *m_tippingWorker;
    GgcWorker *m_ggcWorker;
};

#endif // MESSAGEHUB_H
