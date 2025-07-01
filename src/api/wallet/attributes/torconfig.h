#ifndef TORCONFIG_H
#define TORCONFIG_H

#include <QString>
#include <QJsonObject>

class TorConfig
{
public:
    TorConfig();

    bool useTorListener() const;
    void setUseTorListener(bool value);

    QString socksProxyAddr() const;
    void setSocksProxyAddr(const QString &value);

    QString sendConfigDir() const;
    void setSendConfigDir(const QString &value);

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    bool m_useTorListener;
    QString m_socksProxyAddr;
    QString m_sendConfigDir;
};

#endif // TORCONFIG_H
