#ifndef NOSTRWORKER_H
#define NOSTRWORKER_H

#include <QObject>
#include <QSettings>
#include <QUrl>

#include "api/common/attributes/result.h"
#include "api/wallet/attributes/slate.h"
#include "api/wallet/owner/walletownerapi.h"
#include "worker/nostrbridge/nostrbridge.h"
#include "nostrdatabase/nostrdatabase.h"

class NostrWorker : public QObject
{
    Q_OBJECT

public:
    explicit NostrWorker(QSettings *settings, WalletOwnerApi *walletOwnerApi, QObject *parent = nullptr);
    bool init();

private slots:
    void onNostrEvent(const QNostrRelay::Event &event, const QUrl &relay);

private:
    void handleTextEvent(const QString &recipient);
    void handleSlatepackEvent(const QString &recipient, const Slate &slate);
    void sendInstruction(const QString &recipient);
    void sendTextReply(const QString &recipient, const QString &text);
    Result<QString> respondWithS2(const Slate &slate);
    Result<QString> respondWithI2(const Slate &slate);
    qlonglong slateAmount(const Slate &slate) const;

    QSettings *m_settings;
    WalletOwnerApi *m_walletOwnerApi;
    NostrBridge *m_bridge = nullptr;
    bool m_initialized = false;
    NostrDatabase *m_database = nullptr;
};

#endif // NOSTRWORKER_H
