#ifndef VIEWWALLETENTRY_H
#define VIEWWALLETENTRY_H

#include <QString>

class ViewWalletEntry
{
public:
    ViewWalletEntry();

    QString commit() const;
    void setCommit(const QString &commit);

    int height() const;
    void setHeight(int height);

    bool isCoinbase() const;
    void setIsCoinbase(bool isCoinbase);

    int lockHeight() const;
    void setLockHeight(int lockHeight);

    int mmrIndex() const;
    void setMmrIndex(int mmrIndex);

    qint64 value() const;
    void setValue(qint64 value);

private:
    QString m_commit;
    int m_height;
    bool m_isCoinbase;
    int m_lockHeight;
    int m_mmrIndex;
    qint64 m_value;
};

#endif // VIEWWALLETENTRY_H
