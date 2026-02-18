#ifndef TIPPINGWORKER_H
#define TIPPINGWORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

#include "telegrambot.h"
#include "result.h"
#include "slate.h"
#include "tippingdatabase.h"
class TippingWorker : public QObject
{
    Q_OBJECT

public:
    TippingWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain = false);

    QString handleDeposit(const QString &user, int amount);
    QString handleWithdraw(const QString &user, int amount);
    QString handleTip(const QString &fromUser, const QString &toUser, int amount);

    TelegramBot *m_bot;
    QSettings *m_settings;
    TippingDatabase *m_db;
    void sendUserMessage(QString user, QString content);
};

#endif // TIPPINGWORKER_H
