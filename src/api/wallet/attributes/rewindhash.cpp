#include "rewindhash.h"

/**
 * @brief RewindHash::RewindHash
 * @param rewindHash
 */
RewindHash::RewindHash(const QString &rewindHash) :
    m_rewindHash(rewindHash)
{
}

/**
 * @brief RewindHash::rewindHash
 * @return
 */
QString RewindHash::rewindHash() const
{
    return m_rewindHash;
}

/**
 * @brief RewindHash::setRewindHash
 * @param rewindHash
 */
void RewindHash::setRewindHash(const QString &rewindHash)
{
    m_rewindHash = rewindHash;
}
