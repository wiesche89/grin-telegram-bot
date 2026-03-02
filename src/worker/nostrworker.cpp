#include "nostrworker.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QString>

namespace {
constexpr qlonglong NanogrinPerGrin = 1000000000LL;

QString instructionText()
{
    return QStringLiteral("Please send a Slatepack as a Nostr message:\n"
                          "- I1 (Invoice) asking for up to 1 GRIN if you're requesting a payout\n"
                          "- or S1 if you want to send GRIN to me.\n"
                          "I will reply with the corresponding I2 or S2.");
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
    m_walletOwnerApi(walletOwnerApi),
    m_walletForeignApi(nullptr)
{
}

bool NostrWorker::init()
{
    if (!m_walletOwnerApi) {
        qWarning() << "[NostrWorker] wallet owner API missing";
        return false;
    }

    // Wallet Foreign Api Instance
    m_walletForeignApi = new WalletForeignApi(m_settings ? m_settings->value("wallet/foreignUrl").toString()
                                                        : QString());

    QString dataDir = qEnvironmentVariable("DATA_DIR");
    QString dbPath;
    if (dataDir.isEmpty()) {
        dbPath = QCoreApplication::applicationDirPath() + "/etc/database/nostr.db";
    } else {
        dbPath = QDir(dataDir).filePath("etc/database/nostr.db");
    }

    m_database = new NostrDatabase(dbPath, this);
    if (!m_database->initialize()) {
        qWarning() << "[NostrWorker] failed to initialize Nostr database:" << dbPath;
        return false;
    }

    m_bridge = new NostrBridge(m_settings,this);
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

    if (!event.id.has_value()) {
        qWarning() << "[NostrWorker] incoming event without id";
        return;
    }

    QString eventId = event.id.value();
    if (eventId.trimmed().isEmpty()) {
        qWarning() << "[NostrWorker] incoming event with empty id";
        return;
    }

    QString author = event.pubkey ? event.pubkey.value() : QStringLiteral("unknown");
    QString content = event.content.trimmed();

    if (m_database && m_database->eventExists(eventId)) {
        qDebug() << "[NostrWorker] ignoring already stored event" << eventId;
        return;
    }

    if (m_database) {
        qint64 createdAt = event.created_at.has_value() ? event.created_at->toSecsSinceEpoch() : -1;
        if (m_database->recordEvent(eventId, author, event.kind, content, relay.toString(), createdAt)) {
            qDebug() << "[NostrWorker] recorded event" << eventId;
        }
        if (!m_database->acknowledgeEvent(eventId)) {
            qWarning() << "[NostrWorker] could not acknowledge event" << eventId;
        }
    }

    qDebug() << "[NostrWorker] incoming event" << eventId << "from" << author << "kind" << event.kind << "relay" << relay << "content:" << content;

    QString recipient = normalizeRecipient(event);
    if (recipient.isEmpty()) {
        qWarning() << "[NostrWorker] incoming event without pubkey";
        return;
    }

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
                      QStringLiteral("Failed to decode Slatepack: %1").arg(slateRes.errorMessage()));
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
    qInfo() << "[NostrWorker] slatepack event from" << recipient << "- state" << slate.sta() << "amount" << slate.amt();
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
            sendTextReply(recipient, QStringLiteral("I1 Slatepacks may request at most 1 GRIN."));
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
        sendTextReply(recipient, QStringLiteral("Slatepack could not be answered: %1").arg(response.errorMessage()));
        return;
    }

    QString reply = QStringLiteral("%1").arg(response.value());
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
    qInfo() << "[NostrWorker] reply text length" << text.size();
    m_bridge->sendMessage(text, recipient);
}

Result<QString> NostrWorker::respondWithS2(const Slate &slate)
{

    if (!m_walletForeignApi) {
        return Error(ErrorType::Unknown, QStringLiteral("Wallet foreign API is not initialized"));
    }

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Debugging
    ///---------------------------------------------------------------------------------------------------------------------------
    qDebug() << "donate: " << slate.amt();

    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling receiveTx
    ///---------------------------------------------------------------------------------------------------------------------------
    Slate slate2;
    {
        Result<Slate> res = m_walletForeignApi->receiveTx(slate, "", "");
        if (!res.unwrapOrLog(slate2, Q_FUNC_INFO)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }
    ///---------------------------------------------------------------------------------------------------------------------------
    /// Handling createSlatepackMessage
    ///---------------------------------------------------------------------------------------------------------------------------
    QString slatepack;
    {
        Result<QString> res = m_walletOwnerApi->createSlatepackMessage(slate2, QJsonArray(), 0);
        if (!res.unwrapOrLog(slatepack, Q_FUNC_INFO)) {
            return Error(ErrorType::Unknown, res.errorMessage());
        }
    }

    return slatepack;
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
    qInfo() << "[NostrWorker] processing invoice (I1) slate for amount" << slate.amt();
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

    qInfo() << "[NostrWorker] I2 response ready, processed slate id" << processed.id();

    return slatepackRes.value();
}
