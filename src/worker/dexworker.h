#ifndef DEXWORKER_H
#define DEXWORKER_H

#include <QObject>
#include <QSettings>
#include "telegrambot.h"
#include "dexdatabase/dexdatabase.h"

class DexWorker : public QObject
{
    Q_OBJECT
public:
    explicit DexWorker(TelegramBot *bot, QSettings *settings, QObject *parent = nullptr);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    void sendUserMessage(const TelegramBotMessage &message, const QString &text, bool plain = false);
    void handleBalanceCommand(const TelegramBotMessage &message);
    void handleOrderCommand(const TelegramBotMessage &message);
    void handleOrdersCommand(const TelegramBotMessage &message);
    void handleCancelCommand(const TelegramBotMessage &message);

    TelegramBot *m_bot;
    QSettings *m_settings;
    DexDatabase *m_db;
};

#endif // DEXWORKER_H
