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
#include <QHash>
#include <QUrl>
#include <QFile>
#include <QTextStream>

#include "telegrambot.h"
#include "result.h"
#include "slate.h"
#include "tippingdatabase.h"
#include "walletownerapi.h"
#include "walletinfo.h"
#include "account.h"
#include "inittxargs.h"

class TippingWorker : public QObject
{
    Q_OBJECT

public:
    TippingWorker(TelegramBot *bot, QSettings *settings, WalletOwnerApi *walletOwnerApi);
    bool init();
    bool handleUpdate(TelegramBotUpdate update);

private slots:
    void checkPendingDeposits();

private:
    void sendUserMessage(TelegramBotMessage message, QString content, bool plain = false, bool sendToUserChat = true);
    bool handleSlatepackDocument(TelegramBotMessage &message);
    bool handleSlatepackText(TelegramBotMessage &message, const QString &text);

    QString handleDepositCommand(const QString &senderId, int amount, TelegramBotMessage message);
    QString handleWithdrawCommand(const QString &senderId, int amount, TelegramBotMessage message);
    QString handleOpenTransactionsCommand(const QString &sender);
    QString handleLedgerCommand(const QString &senderId, const QString &senderLabel);
    QString handleAdminAmountsCommand();
    bool isAdmin(qlonglong id);
    Result<WalletInfo> fetchAccountSummary(const QString &accountLabel);
    QString formatWalletSummary(const WalletInfo &walletInfo) const;
    void sendSlatepackMessage(TelegramBotMessage message, const QString &slatepack, const QString &stateLabel, bool sendToUserChat = true);
    bool ensureTippingAccount();
    Result<QString> createInvoiceSlatepack(qlonglong nanogrin, QString &slateId);
    Result<QString> createSendSlatepack(qlonglong nanogrin, const QString &senderId);
    Result<QString> handleSlateS2State(Slate slate, TelegramBotMessage message, const PendingWithdrawRecord &pendingWithdraw);
    Result<QString> handleSlateI2State(Slate slate, TelegramBotMessage message);
    qlonglong slateToGrin(const Slate &slate) const;
    QString userLabel(const TelegramBotMessage &message) const;

    QString downloadFileToQString(const QUrl &url);

    QString readFileToString(const QString &filePath);
    void sendUserMarkdownMessage(TelegramBotMessage message, QString content, bool plain, bool sendToUserChat = true);
    QString resolveRecipientId(const QString &target, const TelegramBotMessage &message) const;

    TelegramBot *m_bot;
    QSettings *m_settings;

    TippingDatabase *m_db;
    WalletOwnerApi *m_walletOwnerApi;
    QString tippingAccountLabel;
    QString walletPassword;
    QTimer *m_pendingDepositTimer;
    QHash<QString, PendingWithdrawRecord> m_pendingWithdraws;

    void sendUserMessage(QString user, QString content);
    bool activateWalletAccount(const QString &accountLabel);
    bool activateTippingWalletAccount();
    void checkPendingWithdrawConfirmations();
    void sendUserDirectMessage(const QString &userId, QString content, bool plain = false);
};
#endif // TIPPINGWORKER_H

