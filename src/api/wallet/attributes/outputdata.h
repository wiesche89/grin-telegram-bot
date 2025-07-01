#ifndef OUTPUTDATA_H
#define OUTPUTDATA_H

#include <QString>
#include <QJsonObject>

class OutputData
{
public:

    enum OutputStatus {
        OutputStatusUnconfirmed,
        OutputStatusUnspent,
        OutputStatusLocked,
        OutputStatusSpent,
        OutputStatusReverted,
        OutputStatusUnknown = 256
    };

    OutputData();

    // Getter/Setter
    QString rootKeyId() const;
    void setRootKeyId(const QString &rootKeyId);

    QString keyId() const;
    void setKeyId(const QString &keyId);

    uint nChild() const;
    void setNChild(uint nChild);

    QString commit() const;
    void setCommit(const QString &commit);

    quint64 mmrIndex() const;
    void setMmrIndex(quint64 mmrIndex);

    quint64 value() const;
    void setValue(quint64 value);

    OutputStatus status() const;
    void setStatus(OutputStatus status);

    quint64 height() const;
    void setHeight(quint64 height);

    quint64 lockHeight() const;
    void setLockHeight(quint64 lockHeight);

    bool isCoinbase() const;
    void setIsCoinbase(bool isCoinbase);

    uint txLogEntry() const;
    void setTxLogEntry(uint txLogEntry);

    // JSON
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_rootKeyId;
    QString m_keyId;
    uint m_nChild;
    QString m_commit;
    quint64 m_mmrIndex;
    quint64 m_value;
    OutputStatus m_status;
    quint64 m_height;
    quint64 m_lockHeight;
    bool m_isCoinbase;
    uint m_txLogEntry;
};

#endif // OUTPUTDATA_H
