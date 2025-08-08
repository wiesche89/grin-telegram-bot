#include "inittxsendargs.h"

/**
 * @brief InitTxSendArgs::InitTxSendArgs
 */
InitTxSendArgs::InitTxSendArgs() :
    m_valid(false),
    m_dest(QString()),
    m_postTx(false),
    m_fluff(false),
    m_skipTor(false)
{
}
InitTxSendArgs::InitTxSendArgs(QString dest, bool postTx, bool fluff, bool skipTor) :
    m_valid(true),
    m_dest(dest),
    m_postTx(postTx),
    m_fluff(fluff),
    m_skipTor(skipTor)
{
}

bool InitTxSendArgs::isValid() const
{
    return m_valid;
}

/**
 * @brief InitTxSendArgs::dest
 * @return
 */
QString InitTxSendArgs::dest() const
{
    return m_dest;
}

/**
 * @brief InitTxSendArgs::postTx
 * @return
 */
bool InitTxSendArgs::postTx() const
{
    return m_postTx;
}

/**
 * @brief InitTxSendArgs::fluff
 * @return
 */
bool InitTxSendArgs::fluff() const
{
    return m_fluff;
}

/**
 * @brief InitTxSendArgs::skipTor
 * @return
 */
bool InitTxSendArgs::skipTor() const
{
    return m_skipTor;
}

/**
 * @brief InitTxSendArgs::toJson
 * @return
 */
QJsonObject InitTxSendArgs::toJson() const
{

    if(!m_valid)
    {
        return QJsonObject();
    }

    QJsonObject obj;
    obj["dest"] = m_dest;
    obj["post_tx"] = m_postTx;
    obj["fluff"] = m_fluff;
    obj["skip_tor"] = m_skipTor;
    return obj;
}

/**
 * @brief InitTxSendArgs::fromJson
 * @param json
 * @return
 */
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
