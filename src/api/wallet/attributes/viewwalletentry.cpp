#include "viewwalletentry.h"

ViewWalletEntry::ViewWalletEntry() : m_height(0),
    m_isCoinbase(false),
    m_lockHeight(0),
    m_mmrIndex(0),
    m_value(0)
{
}

QString ViewWalletEntry::commit() const
{
    return m_commit;
}

void ViewWalletEntry::setCommit(const QString &commit)
{
    m_commit = commit;
}

int ViewWalletEntry::height() const
{
    return m_height;
}

void ViewWalletEntry::setHeight(int height)
{
    m_height = height;
}

bool ViewWalletEntry::isCoinbase() const
{
    return m_isCoinbase;
}

void ViewWalletEntry::setIsCoinbase(bool isCoinbase)
{
    m_isCoinbase = isCoinbase;
}

int ViewWalletEntry::lockHeight() const
{
    return m_lockHeight;
}

void ViewWalletEntry::setLockHeight(int lockHeight)
{
    m_lockHeight = lockHeight;
}

int ViewWalletEntry::mmrIndex() const
{
    return m_mmrIndex;
}

void ViewWalletEntry::setMmrIndex(int mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

qint64 ViewWalletEntry::value() const
{
    return m_value;
}

void ViewWalletEntry::setValue(qint64 value)
{
    m_value = value;
}
