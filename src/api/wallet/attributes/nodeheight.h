#ifndef NODEHEIGHT_H
#define NODEHEIGHT_H

#include <QString>
#include <QJsonObject>

class NodeHeight
{
public:
    NodeHeight();

    QString headerHash() const;
    void setHeaderHash(const QString &value);

    quint64 height() const;
    void setHeight(quint64 value);

    bool updatedFromNode() const;
    void setUpdatedFromNode(bool value);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:
    QString m_headerHash;
    quint64 m_height;
    bool m_updatedFromNode;
};

#endif // NODEHEIGHT_H
