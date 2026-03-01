#include "nostrworker.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>

namespace {
constexpr qlonglong NanogrinPerGrin = 1000000000LL;

QString instructionText()
{
    return QStringLiteral("Bitte sende einen Slatepack als Nostr-Nachricht:\n"
                          "- I1 (Invoice) mit maximal 1 GRIN, wenn du eine Auszahlung anforderst\n"
                          "- oder S1, wenn du mir GRIN senden möchtest.\n"
                          "Ich antworte mit dem passenden I2 bzw. S2.");
}

QString normalizeRecipient(const QNostrRelay::Event &event)
{
    if (!event.pubkey || event.pubkey->isEmpty()) {
        return {};
    }
    return event.pubkey.value();
}
}

NostrWorker::NostrWorker(QSettings *settings, WalletOwnerApi *walletOwnerApi, QObject *parent) :
    QObject(parent),
    m_settings(settings),
    m_walletOwnerApi(walletOwnerApi)
{
}

bool NostrWorker::init()
{
    if (!m_walletOwnerApi) {
        qWarning() << "[NostrWorker] wallet owner API missing";
        return false;
    }

    m_bridge = new NostrBridge(this);
    connect(m_bridge, &NostrBridge::eventReceived, this, &NostrWorker::onNostrEvent);

    QStringList relays = m_settings ? m_settings->value("nostr/relays").toStringList() : QStringList();
    if (relays.isEmpty()) {
        relays << QStringLiteral("wss://relay.damus.io");
    }

    for (const QString &relayValue : relays) {
        QUrl relayUrl(relayValue);
        if (!relayUrl.isValid()) {
            qWarning() << "[NostrWorker] invalid relay URL" << relayValue;
            continue;
        }
        m_bridge->addRelay(relayUrl);
        qDebug() << "[NostrWorker] connected relay" << relayUrl;
    }

    m_initialized = true;
    return true;
}

void NostrWorker::onNostrEvent(const QNostrRelay::Event &event, const QUrl &relay)
{
    if (!m_initialized) {
        return;
    }

    QString author = event.pubkey ? event.pubkey.value() : QStringLiteral("unknown");
    QString eventId = event.id ? event.id.value() : QStringLiteral("unknown");
    QString content = event.content.trimmed();

    qDebug() << "[NostrWorker] incoming event" << eventId << "from" << author << "kind" << event.kind << "relay" << relay << "content:" << content;

    QString recipient = normalizeRecipient(event);
    if (recipient.isEmpty()) {
        qWarning() << "[NostrWorker] incoming event without pubkey";
        return;
    }

    // content already defined above
    if (content.isEmpty()) {
        qDebug() << "[NostrWorker] ignoring empty content from" << recipient;
        return;
    }

    if (!content.contains(QStringLiteral("BEGINSLATEPACK"), Qt::CaseInsensitive) ||
        !content.contains(QStringLiteral("ENDSLATEPACK"), Qt::CaseInsensitive)) {
        handleTextEvent(recipient);
        return;
    }

    Result<Slate> slateRes = m_walletOwnerApi->slateFromSlatepackMessage(content);
    if (slateRes.hasError()) {
        sendTextReply(recipient,
                      QStringLiteral("Slatepack konnte nicht entschlüsselt werden: %1").arg(slateRes.errorMessage()));
        return;
    }

    Slate slate = slateRes.value();
    handleSlatepackEvent(recipient, slate);
}

void NostrWorker::handleTextEvent(const QString &recipient)
{
    sendInstruction(recipient);
}

void NostrWorker::handleSlatepackEvent(const QString &recipient, const Slate &slate)
{
    SlateState state = Slate::slateStateFromString(slate.sta());
    Result<QString> response = Error(ErrorType::Unknown, QStringLiteral("No handler"));
    bool hasResponse = false;

    switch (state) {
    case SlateState::S1:
        response = respondWithS2(slate);
        hasResponse = true;
        break;
    case SlateState::I1: {
        qlonglong amount = slateAmount(slate);
        if (amount > NanogrinPerGrin) {
            sendTextReply(recipient, QStringLiteral("I1-Slatepacks dürfen maximal 1 GRIN anfordern."));
            return;
        }
        response = respondWithI2(slate);
        hasResponse = true;
        break;
    }
    default:
        sendInstruction(recipient);
        return;
    }

    if (!hasResponse || response.hasError()) {
        sendTextReply(recipient, QStringLiteral("Slatepack konnte nicht beantwortet werden: %1").arg(response.errorMessage()));
        return;
    }

    QString reply = QStringLiteral("Slatepack-Antwort: %1").arg(response.value());
    sendTextReply(recipient, reply);
}

void NostrWorker::sendInstruction(const QString &recipient)
{
    sendTextReply(recipient, instructionText());
}

void NostrWorker::sendTextReply(const QString &recipient, const QString &text)
{
    if (!m_bridge || recipient.isEmpty() || text.isEmpty()) {
        return;
    }

    qDebug() << "[NostrWorker] sending reply to" << recipient;
    m_bridge->sendMessage(text, recipient);
}

Result<QString> NostrWorker::respondWithS2(const Slate &slate)
{
    Result<Slate> finalizeRes = m_walletOwnerApi->finalizeTx(slate);
    if (finalizeRes.hasError()) {
        return finalizeRes.error();
    }

    Slate finalized = finalizeRes.value();
    Result<QString> slatepackRes = m_walletOwnerApi->createSlatepackMessage(finalized, QJsonArray(), 0);
    if (slatepackRes.hasError()) {
        return slatepackRes.error();
    }

    return slatepackRes.value();
}

qlonglong NostrWorker::slateAmount(const Slate &slate) const
{
    bool ok = false;
    qlonglong value = slate.amt().toLongLong(&ok);
    if (!ok) {
        return 0;
    }
    return value;
}

Result<QString> NostrWorker::respondWithI2(const Slate &slate)
{
    QJsonObject txData;
    txData["src_acct_name"] = QJsonValue::Null;
    txData["amount"] = slate.amt();
    txData["minimum_confirmations"] = 10;
    txData["selection_strategy_is_use_all"] = false;
    txData["amount_includes_fee"] = QJsonValue::Null;
    txData["max_outputs"] = 500;
    txData["num_change_outputs"] = 1;
    txData["target_slate_version"] = QJsonValue::Null;
    txData["ttl_blocks"] = QJsonValue::Null;
    txData["estimate_only"] = false;
    txData["payment_proof_recipient_address"] = QJsonValue::Null;
    txData["late_lock"] = false;
    txData["send_args"] = QJsonValue::Null;

    Result<Slate> processRes = m_walletOwnerApi->processInvoiceTx(slate, txData);
    if (processRes.hasError()) {
        return processRes.error();
    }

    Slate processed = processRes.value();
    Result<bool> lockRes = m_walletOwnerApi->txLockOutputs(slate);
    if (lockRes.hasError()) {
        qWarning() << "[NostrWorker] txLockOutputs failed:" << lockRes.errorMessage();
    }

    Result<QString> slatepackRes = m_walletOwnerApi->createSlatepackMessage(processed, QJsonArray(), 0);
    if (slatepackRes.hasError()) {
        return slatepackRes.error();
    }

    return slatepackRes.value();
}
