#include "nostrworker.h"

#include <QVariant>
#include <QJsonArray>
#include <cmath>

namespace {
constexpr qlonglong NanogrinPerGrin = 1000000000LL;

QStringList configuredRelays(QSettings *settings)
{
    QStringList relays;
    if (!settings) {
        return relays;
    }

    QVariant value = settings->value("nostr/relays");
    if (value.canConvert<QStringList>()) {
        relays = value.toStringList();
    } else if (value.type() == QVariant::String) {
        relays = value.toString().split(',', Qt::SkipEmptyParts);
    }

    for (QString &relay : relays) {
        relay = relay.trimmed();
    }

    return relays;
}

QString userHandle(const TelegramBotMessage &message)
{
    if (!message.from.username.isEmpty()) {
        return QStringLiteral("@%1").arg(message.from.username);
    }
    if (!message.from.firstName.isEmpty()) {
        return message.from.firstName;
    }
    return QString::number(message.from.id);
}
}

NostrWorker::NostrWorker(TelegramBot *bot, QSettings *settings, WalletOwnerApi *walletOwnerApi) :
    m_bot(bot),
    m_settings(settings),
    m_walletOwnerApi(walletOwnerApi)
{
}

bool NostrWorker::init()
{
    if (!m_bot || !m_walletOwnerApi) {
        qWarning() << "[NostrWorker] Missing bot or wallet owner api";
        return false;
    }

    m_bridge = new NostrBridge(this);
    connect(m_bridge, &NostrBridge::eventReceived, this, &NostrWorker::onNostrEvent);
    qInfo() << "[NostrWorker] listening for events, public key =" << m_bridge->publicKey();

    QStringList relays = configuredRelays(m_settings);
    for (const QString &relayValue : relays) {
        QUrl relayUrl(relayValue);
        if (relayUrl.isValid()) {
            m_bridge->addRelay(relayUrl);
            qInfo() << "[NostrWorker] added relay from settings:" << relayUrl;
        } else {
            qWarning() << "[NostrWorker] ignoring invalid relay URL:" << relayValue;
        }
    }

    m_initialized = true;
    return true;
}

bool NostrWorker::handleUpdate(TelegramBotUpdate update)
{
    if (!m_initialized || !update || update.isNull()) {
        return false;
    }

    if (update->type != TelegramBotMessageType::Message) {
        return false;
    }

    TelegramBotMessage message = *update->message;
    QString text = message.text.trimmed();
    if (text.isEmpty()) {
        return false;
    }

    QStringList tokens = text.split(' ', Qt::SkipEmptyParts);
    if (tokens.isEmpty()) {
        return false;
    }

    QString cmd = tokens.first();
    int atIndex = cmd.indexOf('@');
    if (atIndex != -1) {
        cmd = cmd.left(atIndex);
    }

    if (!cmd.compare(QStringLiteral("/nostrpay"), Qt::CaseInsensitive)) {
        if (tokens.size() < 3) {
            sendUsageMessage(message);
            return true;
        }

        qlonglong nanogrin = 0;
        QString errorMessage;
        if (!parseAmount(tokens.at(1), nanogrin, errorMessage)) {
            sendTextToChat(message.chat.id, errorMessage);
            return true;
        }

        QString recipient = tokens.at(2).trimmed();
        if (recipient.isEmpty()) {
            sendTextToChat(message.chat.id, QStringLiteral("Bitte gib eine gültige Nostr-Adresse an."));
            return true;
        }

        Result<InvoiceResult> invoiceRes = createInvoiceSlatepack(nanogrin);
        if (invoiceRes.hasError()) {
            sendTextToChat(message.chat.id, QStringLiteral("Konnte kein Invoice erstellen: %1").arg(invoiceRes.errorMessage()));
            return true;
        }

        InvoiceResult invoice = invoiceRes.value();
        QString slateId = invoice.slate.id();
        if (!slateId.isEmpty()) {
            PendingInvoice pending{ message.chat.id, nanogrin, recipient, userHandle(message) };
            m_pendingInvoices.insert(slateId, pending);
            qDebug() << "[NostrWorker] stored pending invoice" << slateId << "for" << pending.userLabel;
        } else {
            qWarning() << "[NostrWorker] invoice returned without slate id";
        }

        QString eventId;
        if (m_bridge) {
            eventId = m_bridge->sendMessage(invoice.slatepack, recipient);
            qDebug() << "[NostrWorker] sent invoice slatepack to" << recipient << "event id =" << eventId;
        }

        QString response = QStringLiteral("Invoice über %1 GRIN an %2 gesendet.").arg(formatGrin(nanogrin), recipient);
        if (!eventId.isEmpty()) {
            response += QStringLiteral(" Event: %1").arg(eventId);
        }

        if (m_bridge && !m_bridge->connected()) {
            response += QStringLiteral("\nHinweis: Keine Relay-Verbindung verfügbar, Event wird gepuffert.");
        }

        sendTextToChat(message.chat.id, response);
        return true;
    }

    return false;
}

void NostrWorker::onNostrEvent(const QNostrRelay::Event &event, const QUrl &relay)
{
    Q_UNUSED(relay);

    if (!m_walletOwnerApi) {
        qWarning() << "[NostrWorker] WalletOwnerApi missing while handling event";
        return;
    }

    if (event.kind != 1) {
        return;
    }

    QString content = event.content.trimmed();
    if (content.isEmpty()) {
        return;
    }

    if (!content.contains(QStringLiteral("BEGINSLATEPACK")) || !content.contains(QStringLiteral("ENDSLATEPACK"))) {
        return;
    }

    Result<Slate> slateRes = m_walletOwnerApi->slateFromSlatepackMessage(content);
    if (slateRes.hasError()) {
        qWarning() << "[NostrWorker] Slatepack parsing failed:" << slateRes.errorMessage();
        return;
    }

    Slate slate = slateRes.value();
    SlateState state = Slate::slateStateFromString(slate.sta());
    if (state != SlateState::I2) {
        qDebug() << "[NostrWorker] Received slate state" << static_cast<int>(state) << "ignoring";
        return;
    }

    QString slateId = slate.id();
    QString finalizedMessage;
    Result<QString> finalizeRes = finalizeSlate(slate);
    if (!finalizeRes.unwrapOrLog(finalizedMessage, Q_FUNC_INFO, "finalize nostalgic slate")) {
        return;
    }

    qInfo() << "[NostrWorker]" << finalizedMessage;

    if (slateId.isEmpty()) {
        return;
    }

    if (!m_pendingInvoices.contains(slateId)) {
        qDebug() << "[NostrWorker] No pending invoice for received slate" << slateId;
        return;
    }

    PendingInvoice pending = m_pendingInvoices.take(slateId);
    QString reply = QStringLiteral("Hi %1,\n%2").arg(pending.userLabel, finalizedMessage);
    sendTextToChat(pending.chatId, reply, true);
}

bool NostrWorker::parseAmount(const QString &input, qlonglong &nanogrin, QString &errorMessage) const
{
    QString trimmed = input.trimmed();
    if (trimmed.isEmpty()) {
        errorMessage = QStringLiteral("Bitte gib einen Betrag an.");
        return false;
    }

    if (trimmed.contains(',')) {
        errorMessage = QStringLiteral("Bitte nutze einen Punkt als Dezimaltrenner.");
        return false;
    }

    bool ok = false;
    double value = trimmed.toDouble(&ok);
    if (!ok || value <= 0.0 || !std::isfinite(value)) {
        errorMessage = QStringLiteral("Ungültiger Betrag: %1").arg(trimmed);
        return false;
    }

    nanogrin = static_cast<qlonglong>(value * NanogrinPerGrin + 0.5);
    if (nanogrin <= 0) {
        errorMessage = QStringLiteral("Der Betrag ist zu klein.");
        return false;
    }

    return true;
}

void NostrWorker::sendUsageMessage(const TelegramBotMessage &message) const
{
    Q_UNUSED(message);
    sendTextToChat(message.chat.id, QStringLiteral("Nutze /nostrpay <Betrag> <Nostr-Adresse>"));
}

void NostrWorker::sendTextToChat(qlonglong chatId, const QString &text, bool markdown) const
{
    if (!m_bot || chatId == 0 || text.isEmpty()) {
        return;
    }

    QVariant chatVar = QVariant::fromValue(chatId);
    m_bot->sendMessage(chatVar,
                       text,
                       0,
                       markdown ? TelegramBot::Markdown : TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

Result<NostrWorker::InvoiceResult> NostrWorker::createInvoiceSlatepack(qlonglong nanogrin)
{
    Result<Slate> invoiceRes = m_walletOwnerApi->issueInvoiceTx(QString::number(nanogrin), QString(), QString());
    if (invoiceRes.hasError()) {
        return invoiceRes.error();
    }

    Slate invoiceSlate = invoiceRes.value();
    Result<QString> slatepackRes = m_walletOwnerApi->createSlatepackMessage(invoiceSlate, QJsonArray(), 0);
    if (slatepackRes.hasError()) {
        return slatepackRes.error();
    }

    InvoiceResult result;
    result.slatepack = slatepackRes.value();
    result.slate = invoiceSlate;
    return result;
}

Result<QString> NostrWorker::finalizeSlate(const Slate &slate)
{
    Result<Slate> finalizeRes = m_walletOwnerApi->finalizeTx(slate);
    if (finalizeRes.hasError()) {
        return finalizeRes.error();
    }

    Slate finalized = finalizeRes.value();
    Result<bool> postRes = m_walletOwnerApi->postTx(finalized, false);
    if (postRes.hasError()) {
        QString errMsg = postRes.errorMessage();
        if (!errMsg.contains(QStringLiteral("no result element"), Qt::CaseInsensitive)) {
            return postRes.error();
        }
        qWarning() << "[NostrWorker] postTx reported missing result element; assuming success.";
    }

    qlonglong amount = slateAmount(finalized);
    if (amount <= 0) {
        return Error(ErrorType::Unknown, QStringLiteral("Amount missing after finalizing slate"));
    }

    return QStringLiteral("%1 depositiert, der Slatepack wurde verbucht.").arg(formatGrin(amount));
}

QString NostrWorker::formatGrin(qlonglong nanogrin) const
{
    bool negative = nanogrin < 0;
    qlonglong absolute = negative ? -nanogrin : nanogrin;
    qlonglong whole = absolute / NanogrinPerGrin;
    qlonglong fraction = absolute % NanogrinPerGrin;

    QString result = QString::number(whole);
    if (fraction != 0) {
        QString fractional = QString::number(fraction).rightJustified(9, '0');
        while (fractional.endsWith('0')) {
            fractional.chop(1);
        }
        result += QStringLiteral(".%1").arg(fractional);
    }

    if (negative) {
        result.prepend(QStringLiteral("-"));
    }

    return QStringLiteral("%1 GRIN").arg(result);
}

qlonglong NostrWorker::slateAmount(const Slate &slate) const
{
    bool ok = false;
    qlonglong value = slate.amt().toLongLong(&ok);
    return ok ? value : 0;
}
