#include "txsource.h"

/**
 * @brief txSourceToString
 * @param src
 * @return
 */
QString txSourceToString(TxSource src)
{
    switch (src) {
    case TxSource::PushApi:
        return "PushApi";
    case TxSource::Broadcast:
        return "Broadcast";
    case TxSource::Fluff:
        return "Fluff";
    case TxSource::EmbargoExpired:
        return "EmbargoExpired";
    case TxSource::Deaggregate:
        return "Deaggregate";
    }
    return {};
}

/**
 * @brief txSourceFromString
 * @param str
 * @return
 */
TxSource txSourceFromString(const QString &str)
{
    if (str == "PushApi") {
        return TxSource::PushApi;
    }
    if (str == "Broadcast") {
        return TxSource::Broadcast;
    }
    if (str == "Fluff") {
        return TxSource::Fluff;
    }
    if (str == "EmbargoExpired") {
        return TxSource::EmbargoExpired;
    }
    if (str == "Deaggregate") {
        return TxSource::Deaggregate;
    }
    // Default or error handling
    return TxSource::PushApi; // Default fallback
}
