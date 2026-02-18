#ifndef TIPPINGWORKER_H
#define TIPPINGWORKER_H

#include <QtGlobal>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QTemporaryFile>
#include <QTimer>
#include <QDateTime>
#include <QDebug>
#include <QUrl>

#include "telegrambot.h"
#include "result.h"
#include "slate.h"
#include "tippingdatabase.h"
#include "walletownerapi.h"
#include "walletinfo.h"
#include "account.h"
#include "inittxargs.h"

struct PendingWithdraw {
    QString slateId;
    int amount;
};

class TippingWorker : public QObject
{
    Q_OBJECT

public:
    TippingWorker(TelegramBot *bot, QSettings *settings);
    bool init();

private slots:
    void onMessage(TelegramBotUpdate update);
    void checkPendingDeposits();

private:
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain = false);
    void handleSlatepackDocument(TelegramBotMessage &message);
    void handleSlatepackText(TelegramBotMessage &message, const QString &text);

    QString handleDepositCommand(const QString &senderId, int amount, TelegramBotMessage message);
    QString handleWithdrawCommand(const QString &senderId, int amount, TelegramBotMessage message);
    QString handleTip(const QString &fromId, const QString &fromLabel, const QString &toUser, int amount);
    QString handleOpenTransactionsCommand(const QString &sender);
    QString handleAdminAmountsCommand();
    bool isAdmin(qlonglong id);
    Result<WalletInfo> fetchAccountSummary(const QString &accountLabel);
    QString formatWalletSummary(const WalletInfo &walletInfo) const;
    void sendSlatepackMessage(TelegramBotMessage message, const QString &slatepack, const QString &stateLabel);
    bool ensureTippingAccount();
    Result<QString> createInvoiceSlatepack(qlonglong nanogrin, QString &slateId);
    Result<QString> createSendSlatepack(qlonglong nanogrin);
    Result<QString> handleSlateS2State(Slate slate, TelegramBotMessage message);
    Result<QString> handleSlateI2State(Slate slate, TelegramBotMessage message);
    qlonglong slateToGrin(const Slate &slate) const;
    QString userLabel(const TelegramBotMessage &message) const;

    QString downloadFileToQString(const QUrl &url);

    TelegramBot *m_bot;
    QSettings *m_settings;

    TippingDatabase *m_db;
    WalletOwnerApi *m_walletOwnerApi;
    QString tippingAccountLabel;
    QString walletPassword;
    QTimer *m_pendingDepositTimer;
    QHash<QString, PendingWithdraw> m_pendingWithdraws;

    void sendUserMessage(QString user, QString content);
};
#endif // TIPPINGWORKER_H

