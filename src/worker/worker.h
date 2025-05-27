#ifndef WORKER_H
#define WORKER_H

#include <QSettings>
#include <QDir>

#include "telegrambot.h"

class Worker : public QObject
{

    Q_OBJECT

public:
    Worker();
    void initBot();


private slots:
    void onMessage(TelegramBotUpdate update);

private:
    TelegramBot *m_bot;
};

#endif // WORKER_H
