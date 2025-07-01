#include "torconfig.h"

TorConfig::TorConfig() :
    m_useTorListener(false)
{
}

bool TorConfig::useTorListener() const
{
    return m_useTorListener;
}

void TorConfig::setUseTorListener(bool value)
{
    m_useTorListener = value;
}

QString TorConfig::socksProxyAddr() const
{
    return m_socksProxyAddr;
}

void TorConfig::setSocksProxyAddr(const QString &value)
{
    m_socksProxyAddr = value;
}

QString TorConfig::sendConfigDir() const
{
    return m_sendConfigDir;
}

void TorConfig::setSendConfigDir(const QString &value)
{
    m_sendConfigDir = value;
}

void TorConfig::fromJson(const QJsonObject &obj)
{
    m_useTorListener = obj.value("use_tor_listener").toBool();
    m_socksProxyAddr = obj.value("socks_proxy_addr").toString();
    m_sendConfigDir = obj.value("send_config_dir").toString();
}

QJsonObject TorConfig::toJson() const
{
    QJsonObject obj;
    obj["use_tor_listener"] = m_useTorListener;
    obj["socks_proxy_addr"] = m_socksProxyAddr;
    obj["send_config_dir"] = m_sendConfigDir;
    return obj;
}
