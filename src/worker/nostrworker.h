#ifndef NOSTRWORKER_H
#define NOSTRWORKER_H

#include <QtGlobal>
#include <QSettings>
#include <QHash>
#include <QUrl>
#include <QJsonArray>
#include <QDebug>

#include "telegrambot.h"
#include "result.h"
#include "slate.h"
#include "walletownerapi.h"
#include "worker/nostrbridge/nostrbridge.h"

class NostrWorker : public QObject
{
    Q_OBJECT

public:
    NostrWorker(TelegramBot *bot, QSettings *settings, WalletOwnerApi *walletOwnerApi);
    bool init();
    bool handleUpdate(TelegramBotUpdate update);

private slots:
    void onNostrEvent(const QNostrRelay::Event &event, const QUrl &relay);

private:
    struct InvoiceResult
    {
        QString slatepack;
        Slate slate;
    };

    struct PendingInvoice
    {
        qlonglong chatId;
        qlonglong amount;
        QString recipient;
        QString userLabel;
    };

    bool parseAmount(const QString &input, qlonglong &nanogrin, QString &errorMessage) const;
    void sendUsageMessage(const TelegramBotMessage &message) const;
    void sendTextToChat(qlonglong chatId, const QString &text, bool markdown = false) const;

    Result<InvoiceResult> createInvoiceSlatepack(qlonglong nanogrin);
    Result<QString> finalizeSlate(const Slate &slate);

    QString formatGrin(qlonglong nanogrin) const;
    qlonglong slateAmount(const Slate &slate) const;

    TelegramBot *m_bot;
    QSettings *m_settings;
    WalletOwnerApi *m_walletOwnerApi;
    NostrBridge *m_bridge = nullptr;
    QHash<QString, PendingInvoice> m_pendingInvoices;
    bool m_initialized = false;
};

#endif // NOSTRWORKER_H
