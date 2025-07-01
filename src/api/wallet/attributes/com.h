#ifndef COM_H
#define COM_H

#include <QString>
#include <QJsonObject>

class Com
{
public:
    static Com fromJson(const QJsonObject &obj);
    QJsonObject toJson() const;

    QString c() const;
    void setC(const QString &newC);

    int f() const;
    void setF(int newF);

    QString p() const;
    void setP(const QString &newP);

private:
    QString m_c;
    int m_f = 0;
    QString m_p;
};

#endif // COM_H
