#ifndef DEBUGUTILS_H
#define DEBUGUTILS_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

// Gibt eine schön formatierte JSON-Darstellung zurück
template <typename T>
inline QString debugJsonString(const T &obj)
{
    QJsonDocument doc(obj.toJson());
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

#endif // DEBUGUTILS_H
