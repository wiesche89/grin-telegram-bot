#ifndef NODEVERSION_H
#define NODEVERSION_H

#include <QString>
#include <QJsonObject>

class NodeVersion
{
public:
    NodeVersion() = default;
    NodeVersion(const QString &nodeVersion, quint64 blockHeaderVersion);

    // Getter
    QString nodeVersion() const;
    quint64 blockHeaderVersion() const;

    // Setter
    void setNodeVersion(const QString &nodeVersion);
    void setBlockHeaderVersion(quint64 blockHeaderVersion);

    // JSON serialization
    static NodeVersion fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    QString m_nodeVersion;
    quint64 m_blockHeaderVersion;
};

#endif // NODEVERSION_H
