#include "version.h"

/**
 * @brief Version::Version
 * @param foreignApiVersion
 * @param supportedSlateVersions
 */
Version::Version(int foreignApiVersion, const QStringList &supportedSlateVersions) :
    m_foreignApiVersion(foreignApiVersion),
    m_supportedSlateVersions(supportedSlateVersions)
{
}

/**
 * @brief Version::fromJson
 * @param obj
 * @return
 */
Version Version::fromJson(const QJsonObject &obj)
{
    Version version;

    if (obj.contains("foreign_api_version") && obj["foreign_api_version"].isDouble()) {
        version.m_foreignApiVersion = obj["foreign_api_version"].toInt();
    }

    if (obj.contains("supported_slate_versions") && obj["supported_slate_versions"].isArray()) {
        QJsonArray arr = obj["supported_slate_versions"].toArray();
        QStringList list;
        for (const QJsonValue &val : arr) {
            if (val.isString()) {
                list.append(val.toString());
            }
        }
        version.m_supportedSlateVersions = list;
    }

    return version;
}

/**
 * @brief Version::toJson
 * @return
 */
QJsonObject Version::toJson() const
{
    QJsonObject obj;
    obj["foreign_api_version"] = m_foreignApiVersion;

    QJsonArray arr;
    for (const QString &v : m_supportedSlateVersions) {
        arr.append(v);
    }
    obj["supported_slate_versions"] = arr;

    return obj;
}

/**
 * @brief Version::foreignApiVersion
 * @return
 */
int Version::foreignApiVersion() const
{
    return m_foreignApiVersion;
}

/**
 * @brief Version::setForeignApiVersion
 * @param version
 */
void Version::setForeignApiVersion(int version)
{
    m_foreignApiVersion = version;
}

/**
 * @brief Version::supportedSlateVersions
 * @return
 */
QStringList Version::supportedSlateVersions() const
{
    return m_supportedSlateVersions;
}

/**
 * @brief Version::setSupportedSlateVersions
 * @param versions
 */
void Version::setSupportedSlateVersions(const QStringList &versions)
{
    m_supportedSlateVersions = versions;
}
