#include "viewwallet.h"

/**
 * @brief ViewWallet::ViewWallet
 */
ViewWallet::ViewWallet() : m_lastPmmrIndex(0),
    m_totalBalance(0)
{
}

/**
 * @brief ViewWallet::lastPmmrIndex
 * @return
 */
int ViewWallet::lastPmmrIndex() const
{
    return m_lastPmmrIndex;
}

/**
 * @brief ViewWallet::setLastPmmrIndex
 * @param index
 */
void ViewWallet::setLastPmmrIndex(int index)
{
    m_lastPmmrIndex = index;
}

/**
 * @brief ViewWallet::entries
 * @return
 */
QVector<ViewWalletEntry> ViewWallet::entries() const
{
    return m_entries;
}

/**
 * @brief ViewWallet::setEntries
 * @param entries
 */
void ViewWallet::setEntries(const QVector<ViewWalletEntry> &entries)
{
    m_entries = entries;
}

/**
 * @brief ViewWallet::rewindHash
 * @return
 */
QString ViewWallet::rewindHash() const
{
    return m_rewindHash;
}

/**
 * @brief ViewWallet::setRewindHash
 * @param hash
 */
void ViewWallet::setRewindHash(const QString &hash)
{
    m_rewindHash = hash;
}

/**
 * @brief ViewWallet::totalBalance
 * @return
 */
qint64 ViewWallet::totalBalance() const
{
    return m_totalBalance;
}

/**
 * @brief ViewWallet::setTotalBalance
 * @param balance
 */
void ViewWallet::setTotalBalance(qint64 balance)
{
    m_totalBalance = balance;
}

/**
 * @brief ViewWallet::fromJson
 * @param obj
 * @return
 */
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

/**
 * @brief ViewWallet::toJson
 * @return
 */
QJsonObject ViewWallet::toJson() const
{
    QJsonObject obj;
    obj["last_pmmr_index"] = m_lastPmmrIndex;
    obj["rewind_hash"] = m_rewindHash;
    obj["total_balance"] = static_cast<double>(m_totalBalance);  // QJson doesn't support qint64 directly

    QJsonArray outputArray;
    for (const ViewWalletEntry &entry : m_entries) {
        QJsonObject entryObj;
        entryObj["commit"] = entry.commit();
        entryObj["height"] = entry.height();
        entryObj["is_coinbase"] = entry.isCoinbase();
        entryObj["lock_height"] = entry.lockHeight();
        entryObj["mmr_index"] = entry.mmrIndex();
        entryObj["value"] = static_cast<double>(entry.value());

        outputArray.append(entryObj);
    }
    obj["output_result"] = outputArray;

    return obj;
}
