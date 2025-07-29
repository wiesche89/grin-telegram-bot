#ifndef TRADEOGREWORKER_H
#define TRADEOGREWORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QMap>

#include "telegrambot.h"
#include "tradeogrepublicapi.h"

class TradeOgreWorker : public QObject
{
    Q_OBJECT

public:
    TradeOgreWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain);
    QString renderChartToFile(const QJsonArray &chart, const QString &market);

    TelegramBot *m_bot;
    QSettings *m_settings;

    TradeOgrePublicApi *m_publicApi;
};

#endif // TRADEOGREWORKER_H
