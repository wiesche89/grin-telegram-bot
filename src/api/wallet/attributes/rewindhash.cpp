#include "rewindhash.h"

RewindHash::RewindHash(const QString &rewindHash) :
    m_rewindHash(rewindHash)
{
}

QString RewindHash::rewindHash() const
{
    return m_rewindHash;
}

void RewindHash::setRewindHash(const QString &rewindHash)
{
    m_rewindHash = rewindHash;
}
