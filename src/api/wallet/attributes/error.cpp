#include "error.h"

Error::Error(const QString &message)
    : m_message(message)
{
}

bool Error::isValid()
{
    if(message().isEmpty())
    {
        return false;
    }

    return true;
}

QString Error::message() const
{
    return m_message;
}

void Error::setMessage(const QString &message)
{
    m_message = message;
}


void Error::parseFromJson(const QJsonObject &json)
{
    if (json.contains("Err")) {
        QJsonObject errObj = json["Err"].toObject();

        QStringList keys = errObj.keys();
        if (!keys.isEmpty()) {
            QString errType = keys.first();
            QString errValue = errObj.value(errType).toString();
            m_message = QString("%1: %2").arg(errType, errValue);
        }
    } else {
        m_message = "Unknown error structure";
    }
}
