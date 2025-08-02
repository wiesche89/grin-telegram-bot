#ifndef GATEIOWORKER_H
#define GATEIOWORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QImage>
#include <QPainter>
#include <QStandardPaths>
#include <QMap>

#include "telegrambot.h"
#include "gateio/gateioclient.h"

class GateIoWorker : public QObject
{
    Q_OBJECT

public:
    GateIoWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain = false);
    QString renderChartToFile(const QJsonArray &chart, const QString &market);
    QString escapeMarkdownV2(const QString &text);

    TelegramBot *m_bot;
    QSettings *m_settings;
    GateIoClient *m_client;

};

#endif // GATEIOWORKER_H
