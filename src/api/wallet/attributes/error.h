#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <QStringList>
#include <QJsonObject>

class Error
{
public:
    explicit Error(const QString &message = "");

    bool isValid();

    QString message() const;
    void setMessage(const QString &message);

    void parseFromJson(const QJsonObject &json);

private:
    QString m_message;
};

#endif // ERROR_H
