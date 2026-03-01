#ifndef MESSAGEHUB_H
#define MESSAGEHUB_H

#include "telegramdatastructs.h"
#include <QObject>

class TelegramBot;
class TippingWorker;
class GgcWorker;
class NostrWorker;

class MessageHub : public QObject
{
    Q_OBJECT

public:
    explicit MessageHub(TelegramBot *bot,
                        TippingWorker *tippingWorker,
                        GgcWorker *ggcWorker,
                        NostrWorker *nostrWorker,
                        QObject *parent = nullptr);

private slots:
    void onBotMessage(TelegramBotUpdate update);

private:
    TelegramBot *m_bot;
    TippingWorker *m_tippingWorker;
    GgcWorker *m_ggcWorker;
    NostrWorker *m_nostrWorker;
};

#endif // MESSAGEHUB_H
