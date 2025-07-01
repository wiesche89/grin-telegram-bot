#ifndef NODEVERSION_H
#define NODEVERSION_H

#include <QString>
#include <QJsonObject>

class NodeVersion
{
private:
    QString m_nodeVersion;
    uint16_t m_blockHeaderVersion;

public:
    NodeVersion() = default;
    NodeVersion(const QString &nodeVersion, uint16_t blockHeaderVersion);

    // Getter
    QString nodeVersion() const;
    uint16_t blockHeaderVersion() const;

    // Setter
    void setNodeVersion(const QString &nodeVersion);
    void setBlockHeaderVersion(uint16_t blockHeaderVersion);

    // JSON serialization
    static NodeVersion fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

#endif // NODEVERSION_H
