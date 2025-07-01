#include "walletconfig.h"

WalletConfig::WalletConfig() :
    m_apiListenPort(0),
    m_ownerApiListenPort(0),
    m_ownerApiIncludeForeign(false),
    m_noCommitCache(false)
{
}

QString WalletConfig::chainType() const
{
    return m_chainType;
}

void WalletConfig::setChainType(const QString &value)
{
    m_chainType = value;
}

QString WalletConfig::apiListenInterface() const
{
    return m_apiListenInterface;
}

void WalletConfig::setApiListenInterface(const QString &value)
{
    m_apiListenInterface = value;
}

int WalletConfig::apiListenPort() const
{
    return m_apiListenPort;
}

void WalletConfig::setApiListenPort(int value)
{
    m_apiListenPort = value;
}

int WalletConfig::ownerApiListenPort() const
{
    return m_ownerApiListenPort;
}

void WalletConfig::setOwnerApiListenPort(int value)
{
    m_ownerApiListenPort = value;
}

QString WalletConfig::apiSecretPath() const
{
    return m_apiSecretPath;
}

void WalletConfig::setApiSecretPath(const QString &value)
{
    m_apiSecretPath = value;
}

QString WalletConfig::nodeApiSecretPath() const
{
    return m_nodeApiSecretPath;
}

void WalletConfig::setNodeApiSecretPath(const QString &value)
{
    m_nodeApiSecretPath = value;
}

QString WalletConfig::checkNodeApiHttpAddr() const
{
    return m_checkNodeApiHttpAddr;
}

void WalletConfig::setCheckNodeApiHttpAddr(const QString &value)
{
    m_checkNodeApiHttpAddr = value;
}

bool WalletConfig::ownerApiIncludeForeign() const
{
    return m_ownerApiIncludeForeign;
}

void WalletConfig::setOwnerApiIncludeForeign(bool value)
{
    m_ownerApiIncludeForeign = value;
}

QString WalletConfig::dataFileDir() const
{
    return m_dataFileDir;
}

void WalletConfig::setDataFileDir(const QString &value)
{
    m_dataFileDir = value;
}

bool WalletConfig::noCommitCache() const
{
    return m_noCommitCache;
}

void WalletConfig::setNoCommitCache(bool value)
{
    m_noCommitCache = value;
}

QString WalletConfig::tlsCertificateFile() const
{
    return m_tlsCertificateFile;
}

void WalletConfig::setTlsCertificateFile(const QString &value)
{
    m_tlsCertificateFile = value;
}

QString WalletConfig::tlsCertificateKey() const
{
    return m_tlsCertificateKey;
}

void WalletConfig::setTlsCertificateKey(const QString &value)
{
    m_tlsCertificateKey = value;
}

QString WalletConfig::darkBackgroundColorScheme() const
{
    return m_darkBackgroundColorScheme;
}

void WalletConfig::setDarkBackgroundColorScheme(const QString &value)
{
    m_darkBackgroundColorScheme = value;
}

void WalletConfig::fromJson(const QJsonObject &obj)
{
    m_chainType = obj.value("chain_type").toString();
    m_apiListenInterface = obj.value("api_listen_interface").toString();
    m_apiListenPort = obj.value("api_listen_port").toInt();
    m_ownerApiListenPort = obj.value("owner_api_listen_port").toInt();
    m_apiSecretPath = obj.value("api_secret_path").toString();
    m_nodeApiSecretPath = obj.value("node_api_secret_path").toString();
    m_checkNodeApiHttpAddr = obj.value("check_node_api_http_addr").toString();
    m_ownerApiIncludeForeign = obj.value("owner_api_include_foreign").toBool();
    m_dataFileDir = obj.value("data_file_dir").toString();
    m_noCommitCache = obj.value("no_commit_cache").toBool();
    m_tlsCertificateFile = obj.value("tls_certificate_file").toString();
    m_tlsCertificateKey = obj.value("tls_certificate_key").toString();
    m_darkBackgroundColorScheme = obj.value("dark_background_color_scheme").toString();
}

QJsonObject WalletConfig::toJson() const
{
    QJsonObject obj;
    obj["chain_type"] = m_chainType.isEmpty() ? QJsonValue(QJsonValue::Null) : m_chainType;
    obj["api_listen_interface"] = m_apiListenInterface;
    obj["api_listen_port"] = m_apiListenPort;
    obj["owner_api_listen_port"] = m_ownerApiListenPort;
    obj["api_secret_path"] = m_apiSecretPath.isEmpty() ? QJsonValue(QJsonValue::Null) : m_apiSecretPath;
    obj["node_api_secret_path"] = m_nodeApiSecretPath.isEmpty() ? QJsonValue(QJsonValue::Null) : m_nodeApiSecretPath;
    obj["check_node_api_http_addr"] = m_checkNodeApiHttpAddr;
    obj["owner_api_include_foreign"] = m_ownerApiIncludeForeign;
    obj["data_file_dir"] = m_dataFileDir;
    obj["no_commit_cache"] = m_noCommitCache;
    obj["tls_certificate_file"] = m_tlsCertificateFile.isEmpty() ? QJsonValue(QJsonValue::Null) : m_tlsCertificateFile;
    obj["tls_certificate_key"] = m_tlsCertificateKey.isEmpty() ? QJsonValue(QJsonValue::Null) : m_tlsCertificateKey;
    obj["dark_background_color_scheme"]
        = m_darkBackgroundColorScheme.isEmpty() ? QJsonValue(QJsonValue::Null) : m_darkBackgroundColorScheme;
    return obj;
}
