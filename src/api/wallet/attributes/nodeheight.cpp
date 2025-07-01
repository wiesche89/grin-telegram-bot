#include "nodeheight.h"

NodeHeight::NodeHeight() :
    m_headerHash(""),
    m_height(0),
    m_updatedFromNode(false)
{
}

QString NodeHeight::headerHash() const
{
    return m_headerHash;
}

void NodeHeight::setHeaderHash(const QString &value)
{
    m_headerHash = value;
}

quint64 NodeHeight::height() const
{
    return m_height;
}

void NodeHeight::setHeight(quint64 value)
{
    m_height = value;
}

bool NodeHeight::updatedFromNode() const
{
    return m_updatedFromNode;
}

void NodeHeight::setUpdatedFromNode(bool value)
{
    m_updatedFromNode = value;
}

QJsonObject NodeHeight::toJson() const
{
    QJsonObject obj;
    obj["header_hash"] = m_headerHash;
    obj["height"] = QString::number(m_height);
    obj["updated_from_node"] = m_updatedFromNode;
    return obj;
}

bool NodeHeight::fromJson(const QJsonObject &json)
{
    if (json.contains("header_hash") && json["header_hash"].isString()) {
        m_headerHash = json["header_hash"].toString();
    } else {
        m_headerHash = "";
    }

    if (json.contains("height")) {
        if (json["height"].isString()) {
            m_height = json["height"].toString().toULongLong();
        } else if (json["height"].isDouble()) {
            m_height = static_cast<quint64>(json["height"].toDouble());
        } else {
            return false;
        }
    } else {
        m_height = 0;
    }

    if (json.contains("updated_from_node") && json["updated_from_node"].isBool()) {
        m_updatedFromNode = json["updated_from_node"].toBool();
    } else {
        m_updatedFromNode = false;
    }

    return true;
}
