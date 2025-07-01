#ifndef REWINDHASH_H
#define REWINDHASH_H

#include <QString>
#include <QJsonObject>

class RewindHash
{
public:
    RewindHash() = default;
    explicit RewindHash(const QString &rewindHash);

    QString rewindHash() const;
    void setRewindHash(const QString &rewindHash);

private:
    QString m_rewindHash;
};

#endif // REWINDHASH_H
