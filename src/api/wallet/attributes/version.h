#ifndef VERSION_H
#define VERSION_H

#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>

class Version {
public:
    Version() = default;
    Version(int foreignApiVersion, const QStringList &supportedSlateVersions);

    static Version fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

    int foreignApiVersion() const;
    void setForeignApiVersion(int version);

    QStringList supportedSlateVersions() const;
    void setSupportedSlateVersions(const QStringList &versions);

private:
    int m_foreignApiVersion = 0;
    QStringList m_supportedSlateVersions;
};

#endif // VERSION_H
