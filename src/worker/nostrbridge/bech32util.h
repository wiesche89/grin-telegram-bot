#ifndef NOSTRBRIDGE_BECH32UTIL_H
#define NOSTRBRIDGE_BECH32UTIL_H

#include <QString>

namespace NostBech32 {
QString bech32ToHex(const QString &value);
QString hexToBech32(const QString &hex, const QString &hrp);
}

#endif // NOSTRBRIDGE_BECH32UTIL_H
