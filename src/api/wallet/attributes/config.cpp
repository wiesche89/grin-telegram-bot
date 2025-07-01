#include "config.h"

Config::Config()
{
}

QString Config::chainType() const
{
    return m_chainType;
}

void Config::setChainType(const QString &value)
{
    m_chainType = value;
}

WalletConfig Config::walletConfig() const
{
    return m_walletConfig;
}

void Config::setWalletConfig(const WalletConfig &value)
{
    m_walletConfig = value;
}

LoggingConfig Config::loggingConfig() const
{
    return m_loggingConfig;
}

void Config::setLoggingConfig(const LoggingConfig &value)
{
    m_loggingConfig = value;
}

TorConfig Config::torConfig() const
{
    return m_torConfig;
}

void Config::setTorConfig(const TorConfig &value)
{
    m_torConfig = value;
}

void Config::fromJson(const QJsonObject &obj)
{
    m_chainType = obj.value("chain_type").toString();

    if (obj.contains("wallet_config") && obj["wallet_config"].isObject()) {
        m_walletConfig.fromJson(obj["wallet_config"].toObject());
    }

    if (obj.contains("logging_config") && obj["logging_config"].isObject()) {
        m_loggingConfig.fromJson(obj["logging_config"].toObject());
    }

    if (obj.contains("tor_config") && obj["tor_config"].isObject()) {
        m_torConfig.fromJson(obj["tor_config"].toObject());
    }
}

QJsonObject Config::toJson() const
{
    QJsonObject obj;
    obj["chain_type"] = m_chainType;
    obj["wallet_config"] = m_walletConfig.toJson();
    obj["logging_config"] = m_loggingConfig.toJson();
    obj["tor_config"] = m_torConfig.toJson();
    return obj;
}
