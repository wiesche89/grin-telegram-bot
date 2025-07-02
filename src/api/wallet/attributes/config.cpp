#include "config.h"

/**
 * @brief Config::Config
 */
Config::Config()
{
}

/**
 * @brief Config::chainType
 * @return
 */
QString Config::chainType() const
{
    return m_chainType;
}

/**
 * @brief Config::setChainType
 * @param value
 */
void Config::setChainType(const QString &value)
{
    m_chainType = value;
}

/**
 * @brief Config::walletConfig
 * @return
 */
WalletConfig Config::walletConfig() const
{
    return m_walletConfig;
}

/**
 * @brief Config::setWalletConfig
 * @param value
 */
void Config::setWalletConfig(const WalletConfig &value)
{
    m_walletConfig = value;
}

/**
 * @brief Config::loggingConfig
 * @return
 */
LoggingConfig Config::loggingConfig() const
{
    return m_loggingConfig;
}

/**
 * @brief Config::setLoggingConfig
 * @param value
 */
void Config::setLoggingConfig(const LoggingConfig &value)
{
    m_loggingConfig = value;
}

/**
 * @brief Config::torConfig
 * @return
 */
TorConfig Config::torConfig() const
{
    return m_torConfig;
}

/**
 * @brief Config::setTorConfig
 * @param value
 */
void Config::setTorConfig(const TorConfig &value)
{
    m_torConfig = value;
}

/**
 * @brief Config::fromJson
 * @param obj
 */
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

/**
 * @brief Config::toJson
 * @return
 */
QJsonObject Config::toJson() const
{
    QJsonObject obj;
    obj["chain_type"] = m_chainType;
    obj["wallet_config"] = m_walletConfig.toJson();
    obj["logging_config"] = m_loggingConfig.toJson();
    obj["tor_config"] = m_torConfig.toJson();
    return obj;
}
