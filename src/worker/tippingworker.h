#ifndef TIPPINGWORKER_H
#define TIPPINGWORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "telegrambot.h"
#include "result.h"
#include "slate.h"

class TippingWorker : public QObject
{
    Q_OBJECT

public:
    TippingWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    Result<QString> handleSlateS1State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI1State(Slate slate, TelegramBotMessage message);

    void sendUserMessage(TelegramBotMessage message, QString content, bool plain);

    TelegramBot *m_bot;
    QSettings *m_settings;
};

#endif // TIPPINGWORKER_H
