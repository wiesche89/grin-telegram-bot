#include "torconfig.h"

/**
 * @brief TorConfig::TorConfig
 */
TorConfig::TorConfig() :
    m_useTorListener(false)
{
}

/**
 * @brief TorConfig::useTorListener
 * @return
 */
bool TorConfig::useTorListener() const
{
    return m_useTorListener;
}

/**
 * @brief TorConfig::setUseTorListener
 * @param value
 */
void TorConfig::setUseTorListener(bool value)
{
    m_useTorListener = value;
}

/**
 * @brief TorConfig::socksProxyAddr
 * @return
 */
QString TorConfig::socksProxyAddr() const
{
    return m_socksProxyAddr;
}

/**
 * @brief TorConfig::setSocksProxyAddr
 * @param value
 */
void TorConfig::setSocksProxyAddr(const QString &value)
{
    m_socksProxyAddr = value;
}

/**
 * @brief TorConfig::sendConfigDir
 * @return
 */
QString TorConfig::sendConfigDir() const
{
    return m_sendConfigDir;
}

/**
 * @brief TorConfig::setSendConfigDir
 * @param value
 */
void TorConfig::setSendConfigDir(const QString &value)
{
    m_sendConfigDir = value;
}

/**
 * @brief TorConfig::fromJson
 * @param obj
 */
void TorConfig::fromJson(const QJsonObject &obj)
{
    m_useTorListener = obj.value("use_tor_listener").toBool();
    m_socksProxyAddr = obj.value("socks_proxy_addr").toString();
    m_sendConfigDir = obj.value("send_config_dir").toString();
}

/**
 * @brief TorConfig::toJson
 * @return
 */
QJsonObject TorConfig::toJson() const
{
    QJsonObject obj;
    obj["use_tor_listener"] = m_useTorListener;
    obj["socks_proxy_addr"] = m_socksProxyAddr;
    obj["send_config_dir"] = m_sendConfigDir;
    return obj;
}
