#ifndef SLATEPACK_H
#define SLATEPACK_H

#include <QString>
#include <QJsonObject>

class Slatepack
{
public:
    Slatepack();

    int mode() const;
    void setMode(int value);

    QString payload() const;
    void setPayload(const QString &value);

    QString sender() const;
    void setSender(const QString &value);

    QString slatepack() const;
    void setSlatepack(const QString &value);

    void fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

private:
    int m_mode;
    QString m_payload;
    QString m_sender;
    QString m_slatepack;
};

#endif // SLATEPACK_H
