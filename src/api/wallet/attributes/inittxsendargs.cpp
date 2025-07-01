#include "inittxsendargs.h"

InitTxSendArgs::InitTxSendArgs() :
    m_postTx(false),
    m_fluff(false),
    m_skipTor(false)
{
}

QString InitTxSendArgs::dest() const
{
    return m_dest;
}

void InitTxSendArgs::setDest(const QString &value)
{
    m_dest = value;
}

bool InitTxSendArgs::postTx() const
{
    return m_postTx;
}

void InitTxSendArgs::setPostTx(bool value)
{
    m_postTx = value;
}

bool InitTxSendArgs::fluff() const
{
    return m_fluff;
}

void InitTxSendArgs::setFluff(bool value)
{
    m_fluff = value;
}

bool InitTxSendArgs::skipTor() const
{
    return m_skipTor;
}

void InitTxSendArgs::setSkipTor(bool value)
{
    m_skipTor = value;
}

QJsonObject InitTxSendArgs::toJson() const
{
    QJsonObject obj;
    obj["dest"] = m_dest;
    obj["post_tx"] = m_postTx;
    obj["fluff"] = m_fluff;
    obj["skip_tor"] = m_skipTor;
    return obj;
}

bool InitTxSendArgs::fromJson(const QJsonObject &json)
{
    if (json.contains("dest") && json["dest"].isString()) {
        m_dest = json["dest"].toString();
    } else {
        return false;
    }

    if (json.contains("post_tx") && json["post_tx"].isBool()) {
        m_postTx = json["post_tx"].toBool();
    } else {
        return false;
    }

    if (json.contains("fluff") && json["fluff"].isBool()) {
        m_fluff = json["fluff"].toBool();
    } else {
        return false;
    }

    if (json.contains("skip_tor") && json["skip_tor"].isBool()) {
        m_skipTor = json["skip_tor"].toBool();
    } else {
        return false;
    }

    return true;
}
