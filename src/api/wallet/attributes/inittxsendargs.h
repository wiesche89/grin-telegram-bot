#ifndef INITTXSENDARGS_H
#define INITTXSENDARGS_H

#include <QString>
#include <QJsonObject>

class InitTxSendArgs
{
public:
    InitTxSendArgs();
    InitTxSendArgs(QString dest, bool postTx, bool fluff, bool skipTor);

    bool isValid() const;
    QString dest() const;
    bool postTx() const;
    bool fluff() const;
    bool skipTor() const;

    QJsonObject toJson() const;
    bool fromJson(const QJsonObject &json);

private:
    bool m_valid;
    QString m_dest;
    bool m_postTx;
    bool m_fluff;
    bool m_skipTor;
};

#endif // INITTXSENDARGS_H
