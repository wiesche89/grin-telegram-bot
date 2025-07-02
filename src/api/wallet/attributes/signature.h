#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <QString>
#include <QJsonObject>

class Signature
{
public:
    Signature();
    Signature(const QString &nonce, const QString &xs, const QString &part);

    QJsonObject toJson() const;
    static Signature fromJson(const QJsonObject &obj);

    QString nonce() const;
    void setNonce(const QString &nonce);

    QString xs() const;
    void setXs(const QString &xs);

    QString part() const;
    void setPart(const QString &part);

private:
    QString m_nonce;
    QString m_xs;
    QString m_part;
};

#endif // SIGNATURE_H
