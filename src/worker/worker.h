#ifndef WORKER_H
#define WORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>

#include "telegrambot.h"
#include "databasemanager.h"
#include "nodeownerapi.h"
#include "walletownerapi.h"
#include "walletforeignapi.h"
#include "nodeforeignapi.h"

#include "slate.h"
#include "walletinfo.h"
#include "txlogentry.h"
#include "debugutils.h"

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker();
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);
    void cleanupRetrieveTxs();

private:
    QString readFileToString(const QString &filePath);
    bool isAdmin(qlonglong id);

    Result<QString> handleSlateS1State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI1State(Slate slate, TelegramBotMessage message);
    QString downloadFileToQString(const QUrl &url);
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain);
    bool scanWallet();

    DatabaseManager *m_dbManager;
    TelegramBot *m_bot;
    NodeOwnerApi *m_nodeOwnerApi;
    NodeForeignApi *m_nodeForeignApi;
    WalletOwnerApi *m_walletOwnerApi;
    WalletForeignApi *m_walletForeignApi;
    QSettings *m_settings;
};

#endif // WORKER_H
