#include "viewwalletentry.h"

/**
 * @brief ViewWalletEntry::ViewWalletEntry
 */
ViewWalletEntry::ViewWalletEntry() : m_height(0),
    m_isCoinbase(false),
    m_lockHeight(0),
    m_mmrIndex(0),
    m_value(0)
{
}

/**
 * @brief ViewWalletEntry::commit
 * @return
 */
QString ViewWalletEntry::commit() const
{
    return m_commit;
}

/**
 * @brief ViewWalletEntry::setCommit
 * @param commit
 */
void ViewWalletEntry::setCommit(const QString &commit)
{
    m_commit = commit;
}

/**
 * @brief ViewWalletEntry::height
 * @return
 */
int ViewWalletEntry::height() const
{
    return m_height;
}

/**
 * @brief ViewWalletEntry::setHeight
 * @param height
 */
void ViewWalletEntry::setHeight(int height)
{
    m_height = height;
}

/**
 * @brief ViewWalletEntry::isCoinbase
 * @return
 */
bool ViewWalletEntry::isCoinbase() const
{
    return m_isCoinbase;
}

/**
 * @brief ViewWalletEntry::setIsCoinbase
 * @param isCoinbase
 */
void ViewWalletEntry::setIsCoinbase(bool isCoinbase)
{
    m_isCoinbase = isCoinbase;
}

/**
 * @brief ViewWalletEntry::lockHeight
 * @return
 */
int ViewWalletEntry::lockHeight() const
{
    return m_lockHeight;
}

/**
 * @brief ViewWalletEntry::setLockHeight
 * @param lockHeight
 */
void ViewWalletEntry::setLockHeight(int lockHeight)
{
    m_lockHeight = lockHeight;
}

/**
 * @brief ViewWalletEntry::mmrIndex
 * @return
 */
int ViewWalletEntry::mmrIndex() const
{
    return m_mmrIndex;
}

/**
 * @brief ViewWalletEntry::setMmrIndex
 * @param mmrIndex
 */
void ViewWalletEntry::setMmrIndex(int mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

/**
 * @brief ViewWalletEntry::value
 * @return
 */
qint64 ViewWalletEntry::value() const
{
    return m_value;
}

/**
 * @brief ViewWalletEntry::setValue
 * @param value
 */
void ViewWalletEntry::setValue(qint64 value)
{
    m_value = value;
}
