#ifndef VIEWWALLET_H
#define VIEWWALLET_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>

#include "viewwalletentry.h"

class ViewWallet
{
public:
    ViewWallet();

    int lastPmmrIndex() const;
    void setLastPmmrIndex(int index);

    QVector<ViewWalletEntry> entries() const;
    void setEntries(const QVector<ViewWalletEntry> &entries);

    QString rewindHash() const;
    void setRewindHash(const QString &hash);

    qint64 totalBalance() const;
    void setTotalBalance(qint64 balance);

    static ViewWallet fromJson(const QJsonObject &obj);

private:
    int m_lastPmmrIndex;
    QVector<ViewWalletEntry> m_entries;
    QString m_rewindHash;
    qint64 m_totalBalance;
};

#endif // VIEWWALLET_H
