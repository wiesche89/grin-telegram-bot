#ifndef INITTXSENDARGS_H
#define INITTXSENDARGS_H

#include <QString>
#include <QJsonObject>

class InitTxSendArgs
{
public:
    InitTxSendArgs();

    QString dest() const;
    void setDest(const QString &value);

    bool postTx() const;
    void setPostTx(bool value);

    bool fluff() const;
    void setFluff(bool value);

    bool skipTor() const;
    void setSkipTor(bool value);

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:
    QString m_dest;
    bool m_postTx;
    bool m_fluff;
    bool m_skipTor;
};

#endif // INITTXSENDARGS_H
