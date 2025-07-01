#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QString>
#include <QJsonObject>

class Account
{
private:
    QString m_label;
    QString m_path;

public:
    Account();
    Account(const QString &label, const QString &path);

    // Getter
    QString label() const;
    QString path() const;

    // Setter
    void setLabel(const QString &label);
    void setPath(const QString &path);

    // JSON
    static Account fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;
};

#endif // ACCOUNT_H
