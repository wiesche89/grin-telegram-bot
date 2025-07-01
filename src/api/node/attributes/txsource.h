#ifndef TXSOURCE_H
#define TXSOURCE_H

#include <QString>

enum class TxSource {
    PushApi,
    Broadcast,
    Fluff,
    EmbargoExpired,
    Deaggregate
};

QString txSourceToString(TxSource src);
TxSource txSourceFromString(const QString &str);

#endif // TXSOURCE_H
