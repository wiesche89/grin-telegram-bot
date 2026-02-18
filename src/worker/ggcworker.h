#ifndef GGCWORKER_H
#define GGCWORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QProcessEnvironment>

#include "telegrambot.h"
#include "ggcdatabasemanager.h"
#include "nodeownerapi.h"
#include "walletownerapi.h"
#include "walletforeignapi.h"
#include "nodeforeignapi.h"

#include "slate.h"
#include "walletinfo.h"
#include "txlogentry.h"
#include "debugutils.h"

class GgcWorker : public QObject
{
    Q_OBJECT

public:
    GgcWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);
    void cleanupRetrieveTxs(bool cleanAll);

private:
    QString readFileToString(const QString &filePath);
    bool isAdmin(qlonglong id);

    Result<QString> handleSlateS1State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI1State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateS2State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateS3State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI2State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI3State(Slate slate, TelegramBotMessage message);

    QString downloadFileToQString(const QUrl &url);
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain);
    void sendUserMarkdownMessage(TelegramBotMessage message, QString content, bool plain);
    QString sanitizeTelegramMarkdown(const QString &input);
    bool scanWallet();

    GgcDatabaseManager *m_dbManager;
    TelegramBot *m_bot;
    NodeOwnerApi *m_nodeOwnerApi;
    NodeForeignApi *m_nodeForeignApi;
    WalletOwnerApi *m_walletOwnerApi;
    WalletForeignApi *m_walletForeignApi;
    QSettings *m_settings;
    qlonglong m_faucetAmount;

};

#endif // GGCWORKER_H
