#include "viewwallet.h"

ViewWallet::ViewWallet() : m_lastPmmrIndex(0),
    m_totalBalance(0)
{
}

int ViewWallet::lastPmmrIndex() const
{
    return m_lastPmmrIndex;
}

void ViewWallet::setLastPmmrIndex(int index)
{
    m_lastPmmrIndex = index;
}

QVector<ViewWalletEntry> ViewWallet::entries() const
{
    return m_entries;
}

void ViewWallet::setEntries(const QVector<ViewWalletEntry> &entries)
{
    m_entries = entries;
}

QString ViewWallet::rewindHash() const
{
    return m_rewindHash;
}

void ViewWallet::setRewindHash(const QString &hash)
{
    m_rewindHash = hash;
}

qint64 ViewWallet::totalBalance() const
{
    return m_totalBalance;
}

void ViewWallet::setTotalBalance(qint64 balance)
{
    m_totalBalance = balance;
}

ViewWallet ViewWallet::fromJson(const QJsonObject &obj)
{
    ViewWallet wallet;

    if (obj.contains("last_pmmr_index") && obj["last_pmmr_index"].isDouble()) {
        wallet.setLastPmmrIndex(obj["last_pmmr_index"].toInt());
    }

    if (obj.contains("output_result") && obj["output_result"].isArray()) {
        QJsonArray arr = obj["output_result"].toArray();
        QVector<ViewWalletEntry> entries;
        for (const QJsonValue &val : arr) {
            if (!val.isObject()) {
                continue;
            }

            QJsonObject entryObj = val.toObject();
            ViewWalletEntry entry;

            if (entryObj.contains("commit") && entryObj["commit"].isString()) {
                entry.setCommit(entryObj["commit"].toString());
            }

            if (entryObj.contains("height") && entryObj["height"].isDouble()) {
                entry.setHeight(entryObj["height"].toInt());
            }

            if (entryObj.contains("is_coinbase") && entryObj["is_coinbase"].isBool()) {
                entry.setIsCoinbase(entryObj["is_coinbase"].toBool());
            }

            if (entryObj.contains("lock_height") && entryObj["lock_height"].isDouble()) {
                entry.setLockHeight(entryObj["lock_height"].toInt());
            }

            if (entryObj.contains("mmr_index") && entryObj["mmr_index"].isDouble()) {
                entry.setMmrIndex(entryObj["mmr_index"].toInt());
            }

            if (entryObj.contains("value") && entryObj["value"].isDouble()) {
                entry.setValue(static_cast<qint64>(entryObj["value"].toDouble()));
            }

            entries.append(entry);
        }
        wallet.setEntries(entries);
    }

    if (obj.contains("rewind_hash") && obj["rewind_hash"].isString()) {
        wallet.setRewindHash(obj["rewind_hash"].toString());
    }

    if (obj.contains("total_balance") && obj["total_balance"].isDouble()) {
        wallet.setTotalBalance(static_cast<qint64>(obj["total_balance"].toDouble()));
    }

    return wallet;
}
