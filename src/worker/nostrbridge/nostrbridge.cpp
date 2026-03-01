#include "nostrbridge.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QDebug>
#include <QSettings>
#include <QtGlobal>
#include <utility>

namespace {
const char BECH32_CHARSET[] = "qpzry9x8gf2tvdw0s3jn54khce6mua7l";
const quint32 BECH32_GENERATORS[] = {
    0x3b6a57b2,
    0x26508e6d,
    0x1ea119fa,
    0x3d4233dd,
    0x2a1462b3,
};

int bech32CharValue(char ch)
{
    static int lookup[128];
    static bool initialized = false;
    if (!initialized) {
        for (int i = 0; i < 128; ++i) {
            lookup[i] = -1;
        }
        for (int i = 0; i < 32; ++i) {
            lookup[static_cast<int>(BECH32_CHARSET[i])] = i;
        }
        initialized = true;
    }

    unsigned char uch = static_cast<unsigned char>(ch);
    if (uch >= 128) {
        return -1;
    }
    return lookup[static_cast<int>(uch)];
}

QVector<int> hrpExpand(const QString &hrp)
{
    QVector<int> expanded;
    for (QChar ch : hrp) {
        expanded.append((ch.unicode() >> 5) & 0x07);
    }
    expanded.append(0);
    for (QChar ch : hrp) {
        expanded.append(ch.unicode() & 0x1f);
    }
    return expanded;
}

quint32 bech32Polymod(const QVector<int> &values)
{
    quint32 chk = 1;
    for (int value : values) {
        quint32 top = chk >> 25;
        chk = ((chk & 0x1ffffff) << 5) ^ static_cast<quint32>(value);
        for (int i = 0; i < 5; ++i) {
            if (top & (1u << i)) {
                chk ^= BECH32_GENERATORS[i];
            }
        }
    }
    return chk;
}

bool bech32VerifyChecksum(const QString &hrp, const QVector<int> &values)
{
    QVector<int> expanded = hrpExpand(hrp);
    expanded += values;
    return bech32Polymod(expanded) == 1;
}

bool bech32Decode(const QString &input, QString &hrp, QVector<int> &data)
{
    if (input.isEmpty() || input.size() < 8 || input.size() > 90) {
        return false;
    }

    QString lower = input.toLower();
    int pos = lower.lastIndexOf('1');
    if (pos < 1 || pos + 7 > lower.size()) {
        return false;
    }

    hrp = lower.left(pos);
    data.clear();
    for (int i = pos + 1; i < lower.size(); ++i) {
        int value = bech32CharValue(lower[i].toLatin1());
        if (value == -1) {
            return false;
        }
        data.append(value);
    }

    if (!bech32VerifyChecksum(hrp, data)) {
        return false;
    }

    data = data.mid(0, data.size() - 6);
    return true;
}

bool convertBits(const QVector<int> &data, int fromBits, int toBits, bool pad, QByteArray &out)
{
    int acc = 0;
    int bits = 0;
    const int maxv = (1 << toBits) - 1;

    for (int value : data) {
        if (value < 0 || (value >> fromBits) != 0) {
            return false;
        }

        acc = (acc << fromBits) | value;
        bits += fromBits;

        while (bits >= toBits) {
            bits -= toBits;
            out.append(static_cast<char>((acc >> bits) & maxv));
        }
    }

    if (pad && bits) {
        out.append(static_cast<char>((acc << (toBits - bits)) & maxv));
    } else if (!pad && bits >= fromBits) {
        return false;
    } else if (!pad && ((acc << (toBits - bits)) & maxv) != 0) {
        return false;
    }

    return true;
}

QString bech32ToHex(const QString &value)
{
    QString hrp;
    QVector<int> data;
    if (!bech32Decode(value, hrp, data)) {
        return {};
    }

    if (hrp != QStringLiteral("npub")) {
        return {};
    }

    QByteArray bytes;
    if (!convertBits(data, 5, 8, false, bytes)) {
        return {};
    }

    return bytes.toHex();
}

QString normalizeRecipient(const QString &candidate)
{
    QString trimmed = candidate.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    if (trimmed.startsWith(QStringLiteral("npub"), Qt::CaseInsensitive)) {
        return bech32ToHex(trimmed);
    }

    return trimmed.toLower();
}

QString publicKeyHexFromBase64(const QString &value)
{
    auto decoded = QByteArray::fromBase64(value.toLatin1());
    if (decoded.size() <= 1) {
        return {};
    }

    return QString::fromLatin1(decoded.mid(1).toHex()).toLower();
}
} // namespace

NostrBridge::NostrBridge(QObject *parent)
    : QObject(parent)
    , m_secretKey()
    , m_recipient(QStringLiteral("npub14eftzc986979z74vfpsaj55pjttsynhj082ujctth8ujxacvz0lq65pcmk"))
    , m_defaultRelay(QStringLiteral("wss://relay.damus.io"))
{
    QSettings settings;
    QString storedSecret = settings.value("nostr/secretKey").toString();
    if (storedSecret.isEmpty()) {
        storedSecret = QNostr::generateNewSecret();
        settings.setValue("nostr/secretKey", storedSecret);
    }
    m_secretKey = storedSecret;
    m_nostr = new QNostr(m_secretKey, this);
    m_publicKeyHex = publicKeyHexFromBase64(m_nostr->publicKey());
    emit keysChanged();
    updateStatus(QStringLiteral("Ready"));
    qInfo() << "NostrBridge created, public key =" << m_publicKeyHex << "secret key =" << m_secretKey;

    connect(m_nostr, &QNostr::connected, this, [this](const QUrl &relay) {
        m_activeRelays.insert(relay);
        updateStatus(QStringLiteral("Connected to %1").arg(relay.toString()));
        emit connectionChanged();
        qInfo() << "Connected to relay" << relay;
    });

    connect(m_nostr, &QNostr::disconnected, this, [this](const QUrl &relay) {
        m_activeRelays.remove(relay);
        updateStatus(m_activeRelays.isEmpty()
                         ? QStringLiteral("Disconnected")
                         : QStringLiteral("Still connected to %1").arg(m_activeRelays.values().first().toString()));
        emit connectionChanged();
        qInfo() << "Disconnected from relay" << relay;
    });

    connect(m_nostr, &QNostr::newEvent, this, [this](const QString &, const QNostrRelay::Event &event, bool stored, const QUrl &relay) {
        Q_UNUSED(stored);
        m_lastEventText = QStringLiteral("[%1] %2").arg(relay.host(), event.content);
        emit lastEventTextChanged();
        emit eventReceived(event, relay);
        qInfo() << "Received event from" << relay << "content:" << event.content;
    });

    connect(m_nostr, &QNostr::notice, this, [this](const QString &msg, const QUrl &relay) {
        qInfo() << "Relay notice" << relay << msg;
    });

    connectDefaultRelay();
}

QString NostrBridge::secretKey() const
{
    return m_secretKey;
}

QString NostrBridge::publicKey() const
{
    return m_publicKeyHex;
}

QString NostrBridge::statusText() const
{
    return m_statusText;
}

QString NostrBridge::lastError() const
{
    return m_lastError;
}

QString NostrBridge::lastEventText() const
{
    return m_lastEventText;
}

bool NostrBridge::connected() const
{
    return !m_activeRelays.isEmpty();
}

QString NostrBridge::recipient() const
{
    return m_recipient;
}

void NostrBridge::setRecipient(const QString &recipient)
{
    QString normalized = recipient.trimmed();
    if (m_recipient == normalized) {
        return;
    }

    m_recipient = normalized;
    emit recipientChanged();
}

void NostrBridge::addRelay(const QUrl &relayUrl)
{
    if (!relayUrl.isValid()) {
        setError(QStringLiteral("Invalid relay URL"));
        return;
    }

    updateStatus(QStringLiteral("Connecting to %1").arg(relayUrl.toString()));
    m_nostr->addRelay(relayUrl);
    qInfo() << "Adding relay" << relayUrl;
    subscribeToMentions();
}

void NostrBridge::removeRelay(const QUrl &relayUrl)
{
    if (!relayUrl.isValid()) {
        return;
    }

    m_nostr->removeRelay(relayUrl);
    m_activeRelays.remove(relayUrl);
    emit connectionChanged();
    qInfo() << "Removed relay" << relayUrl;
}

void NostrBridge::disconnectFromRelays()
{
    for (const QUrl &relay : std::as_const(m_activeRelays)) {
        m_nostr->removeRelay(relay);
    }

    m_activeRelays.clear();
    updateStatus(QStringLiteral("Disconnected"));
    emit connectionChanged();
    qInfo() << "Disconnected from all relays";
}

QString NostrBridge::requestEvents(const QStringList &authors, const QList<int> &kinds, const QStringList &pTags, int limit)
{
    if (!m_nostr) {
        return {};
    }

    QNostrRelay::Request req;
    req.authors = authors;
    req.kinds = kinds;
    req.p = pTags;
    req.limit = limit;
    qInfo() << "Requesting events authors" << authors << "kinds" << kinds << "p" << pTags << "limit" << limit;
    return m_nostr->sendRequest(req);
}

QString NostrBridge::sendMessage(const QString &content, const QString &recipientBech, int kind)
{
    if (!m_nostr) {
        return {};
    }

    QNostrRelay::Event event;
    event.kind = kind;
    event.content = content;
    const QString target = normalizeRecipient(recipientBech.isEmpty() ? m_recipient : recipientBech);
    if (!target.isEmpty()) {
        event.tags.append({QStringLiteral("p"), target});
    }
    qInfo() << "Sending message kind" << kind << "content" << content << "recipient" << target;
    return m_nostr->sendEvent(event);
}

void NostrBridge::connectDefaultRelay()
{
    if (!m_defaultRelay.isValid()) {
        return;
    }

    qInfo() << "Auto-connecting to default Damus relay" << m_defaultRelay;
    addRelay(m_defaultRelay);
}

void NostrBridge::subscribeToMentions()
{
    if (!m_nostr || m_publicKeyHex.isEmpty()) {
        return;
    }

    if (!m_replySubscriptionId.isEmpty()) {
        QNostrRelay::Close closeReq;
        closeReq.subscriptionId = m_replySubscriptionId;
        m_nostr->sendClose(closeReq);
    }

    QStringList pTags;
    pTags << m_publicKeyHex;
    QList<int> kinds;
    kinds << 1;
    m_replySubscriptionId = requestEvents(QStringList(), kinds, pTags, 50);
    qInfo() << "Subscribed to replies for" << m_publicKeyHex << "subscription" << m_replySubscriptionId;
}

void NostrBridge::updateStatus(const QString &text)
{
    if (m_statusText == text) {
        return;
    }

    m_statusText = text;
    emit statusTextChanged();
}

void NostrBridge::setError(const QString &message)
{
    if (m_lastError != message) {
        m_lastError = message;
        emit lastErrorChanged();
    }

    updateStatus(QStringLiteral("Error: %1").arg(message));
}
