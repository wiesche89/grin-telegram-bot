#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QJsonObject>

#include "walletconfig.h"
#include "loggingconfig.h"
#include "torconfig.h"

class Config
{
public:
    Config();

    QString chainType() const;
    void setChainType(const QString &value);

    WalletConfig walletConfig() const;
    void setWalletConfig(const WalletConfig &value);

    LoggingConfig loggingConfig() const;
    void setLoggingConfig(const LoggingConfig &value);

    TorConfig torConfig() const;
    void setTorConfig(const TorConfig &value);

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    QString m_chainType;
    WalletConfig m_walletConfig;
    LoggingConfig m_loggingConfig;
    TorConfig m_torConfig;
};

#endif // CONFIG_H
