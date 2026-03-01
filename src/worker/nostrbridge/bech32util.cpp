#include "worker/nostrbridge/bech32util.h"

#include <QByteArray>
#include <QVector>
#include <QtGlobal>

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

bool bytesToFiveBits(const QByteArray &input, QVector<int> &out)
{
    QVector<int> octets;
    octets.reserve(input.size());
    for (unsigned char ch : input) {
        octets.append(ch);
    }

    QByteArray converted;
    if (!convertBits(octets, 8, 5, true, converted)) {
        return false;
    }

    out.clear();
    out.reserve(converted.size());
    for (char ch : converted) {
        out.append(static_cast<unsigned char>(ch));
    }

    return true;
}

QVector<int> createChecksum(const QString &hrp, const QVector<int> &values)
{
    QString lowerHrp = hrp.toLower();
    QVector<int> expanded = hrpExpand(lowerHrp);
    expanded += values;
    for (int i = 0; i < 6; ++i) {
        expanded.append(0);
    }

    quint32 polymod = bech32Polymod(expanded) ^ 1;
    QVector<int> checksum(6);
    for (int i = 0; i < 6; ++i) {
        checksum[i] = (polymod >> (5 * (5 - i))) & 0x1f;
    }
    return checksum;
}

QString bech32Encode(const QString &hrp, const QVector<int> &values)
{
    if (hrp.isEmpty()) {
        return {};
    }

    QString lowerHrp = hrp.toLower();
    QString encoded = lowerHrp + QStringLiteral("1");

    for (int value : values) {
        if (value < 0 || value >= 32) {
            return {};
        }
        encoded.append(BECH32_CHARSET[value]);
    }

    QVector<int> checksum = createChecksum(lowerHrp, values);
    for (int value : checksum) {
        encoded.append(BECH32_CHARSET[value]);
    }

    return encoded;
}
} // namespace

namespace NostBech32 {
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

QString hexToBech32(const QString &hex, const QString &hrp)
{
    QString trimmed = hex.trimmed();
    if (trimmed.isEmpty() || hrp.isEmpty()) {
        return {};
    }

    QByteArray bytes = QByteArray::fromHex(trimmed.toLatin1());
    if (bytes.isEmpty()) {
        return {};
    }

    QVector<int> data;
    if (!bytesToFiveBits(bytes, data)) {
        return {};
    }

    return bech32Encode(hrp, data);
}
} // namespace NostBech32
