#include "outputdata.h"

/**
 * @brief OutputData::OutputData
 */
OutputData::OutputData() :
    m_nChild(0),
    m_mmrIndex(0),
    m_value(0),
    m_status(OutputStatusUnknown),
    m_height(0),
    m_lockHeight(0),
    m_isCoinbase(false),
    m_txLogEntry(0)
{
}

/**
 * @brief OutputData::rootKeyId
 * @return
 */
QString OutputData::rootKeyId() const
{
    return m_rootKeyId;
}

/**
 * @brief OutputData::setRootKeyId
 * @param rootKeyId
 */
void OutputData::setRootKeyId(const QString &rootKeyId)
{
    m_rootKeyId = rootKeyId;
}

/**
 * @brief OutputData::keyId
 * @return
 */
QString OutputData::keyId() const
{
    return m_keyId;
}

/**
 * @brief OutputData::setKeyId
 * @param keyId
 */
void OutputData::setKeyId(const QString &keyId)
{
    m_keyId = keyId;
}

/**
 * @brief OutputData::nChild
 * @return
 */
uint OutputData::nChild() const
{
    return m_nChild;
}

/**
 * @brief OutputData::setNChild
 * @param nChild
 */
void OutputData::setNChild(uint nChild)
{
    m_nChild = nChild;
}

/**
 * @brief OutputData::commit
 * @return
 */
QString OutputData::commit() const
{
    return m_commit;
}

/**
 * @brief OutputData::setCommit
 * @param commit
 */
void OutputData::setCommit(const QString &commit)
{
    m_commit = commit;
}

/**
 * @brief OutputData::mmrIndex
 * @return
 */
quint64 OutputData::mmrIndex() const
{
    return m_mmrIndex;
}

/**
 * @brief OutputData::setMmrIndex
 * @param mmrIndex
 */
void OutputData::setMmrIndex(quint64 mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

/**
 * @brief OutputData::value
 * @return
 */
quint64 OutputData::value() const
{
    return m_value;
}

/**
 * @brief OutputData::setValue
 * @param value
 */
void OutputData::setValue(quint64 value)
{
    m_value = value;
}

/**
 * @brief OutputData::status
 * @return
 */
OutputData::OutputStatus OutputData::status() const
{
    return m_status;
}

/**
 * @brief OutputData::setStatus
 * @param status
 */
void OutputData::setStatus(OutputStatus status)
{
    m_status = status;
}

/**
 * @brief OutputData::height
 * @return
 */
quint64 OutputData::height() const
{
    return m_height;
}

/**
 * @brief OutputData::setHeight
 * @param height
 */
void OutputData::setHeight(quint64 height)
{
    m_height = height;
}

/**
 * @brief OutputData::lockHeight
 * @return
 */
quint64 OutputData::lockHeight() const
{
    return m_lockHeight;
}

/**
 * @brief OutputData::setLockHeight
 * @param lockHeight
 */
void OutputData::setLockHeight(quint64 lockHeight)
{
    m_lockHeight = lockHeight;
}

/**
 * @brief OutputData::isCoinbase
 * @return
 */
bool OutputData::isCoinbase() const
{
    return m_isCoinbase;
}

/**
 * @brief OutputData::setIsCoinbase
 * @param isCoinbase
 */
void OutputData::setIsCoinbase(bool isCoinbase)
{
    m_isCoinbase = isCoinbase;
}

/**
 * @brief OutputData::txLogEntry
 * @return
 */
uint OutputData::txLogEntry() const
{
    return m_txLogEntry;
}

/**
 * @brief OutputData::setTxLogEntry
 * @param txLogEntry
 */
void OutputData::setTxLogEntry(uint txLogEntry)
{
    m_txLogEntry = txLogEntry;
}

/**
 * @brief OutputData::fromJson
 * @param json
 */
void OutputData::fromJson(const QJsonObject &json)
{
    m_rootKeyId = json.value("root_key_id").toString();
    m_keyId = json.value("key_id").toString();
    m_nChild = json.value("n_child").toInt();
    m_commit = json.value("commit").toString();

    m_mmrIndex = static_cast<quint64>(json.value("mmr_index").toDouble());

    m_value = static_cast<quint64>(json.value("value").toDouble());

    QString statusStr = json.value("status").toString();
    if (statusStr == "Unspent") {
        m_status = OutputStatusUnspent;
    } else if (statusStr == "Spent") {
        m_status = OutputStatusSpent;
    } else if (statusStr == "Locked") {
        m_status = OutputStatusLocked;
    } else {
        m_status = OutputStatusUnknown;
    }

    m_height = static_cast<quint64>(json.value("height").toDouble());
    m_lockHeight = static_cast<quint64>(json.value("lock_height").toDouble());
    m_isCoinbase = json.value("is_coinbase").toBool();

    // tx_log_entry optional
    m_txLogEntry = static_cast<uint>(json.value("tx_log_entry").toInt());
}

/**
 * @brief OutputData::toJson
 * @return
 */
QJsonObject OutputData::toJson() const
{
    QJsonObject json;
    json["root_key_id"] = m_rootKeyId;
    json["key_id"] = m_keyId;
    json["n_child"] = static_cast<int>(m_nChild);

    if (!m_commit.isEmpty()) {
        json["commit"] = m_commit;
    }

    if (m_mmrIndex != 0) {
        json["mmr_index"] = static_cast<double>(m_mmrIndex);
    }

    json["value"] = static_cast<double>(m_value);

    // Status als String zurückgeben
    switch (m_status) {
    case OutputStatusUnspent:
        json["status"] = "Unspent";
        break;
    case OutputStatusSpent:
        json["status"] = "Spent";
        break;
    case OutputStatusLocked:
        json["status"] = "Locked";
        break;
    default:
        json["status"] = "Unknown";
        break;
    }

    json["height"] = static_cast<double>(m_height);
    json["lock_height"] = static_cast<double>(m_lockHeight);
    json["is_coinbase"] = m_isCoinbase;

    if (m_txLogEntry != 0) {
        json["tx_log_entry"] = static_cast<int>(m_txLogEntry);
    }

    return json;
}
