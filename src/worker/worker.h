#ifndef WORKER_H
#define WORKER_H

#include <QSettings>
#include <QDir>
#include <QCoreApplication>

#include "telegrambot.h"
#include "databasemanager.h"
#include "nodeownerapi.h"
#include "walletownerapi.h"
#include "walletforeignapi.h"
#include "nodeforeignapi.h"

#include "slate.h"
#include "summaryinfo.h"
#include "transaction.h"

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker();
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);

private:
    QString readFileToString(const QString &filePath);
    bool isAdmin(qlonglong id);

    QString handleSlateS1State(QJsonObject slate, TelegramBotMessage message);
    QString handleSlateI1State(QJsonObject slate, TelegramBotMessage message);

    void outputRetrieveTxs();

    DatabaseManager *m_dbManager;
    TelegramBot *m_bot;
    NodeOwnerApi *m_nodeOwnerApi;
    NodeForeignApi *m_nodeForeignApi;
    WalletOwnerApi *m_walletOwnerApi;
    WalletForeignApi *m_walletForeignApi;
    QSettings *m_settings;
};

#endif // WORKER_H
