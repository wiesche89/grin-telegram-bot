#ifndef WALLETCONFIG_H
#define WALLETCONFIG_H

#include <QString>
#include <QJsonObject>

class WalletConfig
{
public:
    WalletConfig();

    QString chainType() const;
    void setChainType(const QString &value);

    QString apiListenInterface() const;
    void setApiListenInterface(const QString &value);

    int apiListenPort() const;
    void setApiListenPort(int value);

    int ownerApiListenPort() const;
    void setOwnerApiListenPort(int value);

    QString apiSecretPath() const;
    void setApiSecretPath(const QString &value);

    QString nodeApiSecretPath() const;
    void setNodeApiSecretPath(const QString &value);

    QString checkNodeApiHttpAddr() const;
    void setCheckNodeApiHttpAddr(const QString &value);

    bool ownerApiIncludeForeign() const;
    void setOwnerApiIncludeForeign(bool value);

    QString dataFileDir() const;
    void setDataFileDir(const QString &value);

    bool noCommitCache() const;
    void setNoCommitCache(bool value);

    QString tlsCertificateFile() const;
    void setTlsCertificateFile(const QString &value);

    QString tlsCertificateKey() const;
    void setTlsCertificateKey(const QString &value);

    QString darkBackgroundColorScheme() const;
    void setDarkBackgroundColorScheme(const QString &value);

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    QString m_chainType;
    QString m_apiListenInterface;
    int m_apiListenPort;
    int m_ownerApiListenPort;
    QString m_apiSecretPath;
    QString m_nodeApiSecretPath;
    QString m_checkNodeApiHttpAddr;
    bool m_ownerApiIncludeForeign;
    QString m_dataFileDir;
    bool m_noCommitCache;
    QString m_tlsCertificateFile;
    QString m_tlsCertificateKey;
    QString m_darkBackgroundColorScheme;
};

#endif // WALLETCONFIG_H
