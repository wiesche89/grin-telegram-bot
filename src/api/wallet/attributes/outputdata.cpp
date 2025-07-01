#include "outputdata.h"

// Konstruktor mit sinnvollen Defaultwerten
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

// Getter/Setter
QString OutputData::rootKeyId() const
{
    return m_rootKeyId;
}

void OutputData::setRootKeyId(const QString &rootKeyId)
{
    m_rootKeyId = rootKeyId;
}

QString OutputData::keyId() const
{
    return m_keyId;
}

void OutputData::setKeyId(const QString &keyId)
{
    m_keyId = keyId;
}

uint OutputData::nChild() const
{
    return m_nChild;
}

void OutputData::setNChild(uint nChild)
{
    m_nChild = nChild;
}

QString OutputData::commit() const
{
    return m_commit;
}

void OutputData::setCommit(const QString &commit)
{
    m_commit = commit;
}

quint64 OutputData::mmrIndex() const
{
    return m_mmrIndex;
}

void OutputData::setMmrIndex(quint64 mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

quint64 OutputData::value() const
{
    return m_value;
}

void OutputData::setValue(quint64 value)
{
    m_value = value;
}

OutputData::OutputStatus OutputData::status() const
{
    return m_status;
}

void OutputData::setStatus(OutputStatus status)
{
    m_status = status;
}

quint64 OutputData::height() const
{
    return m_height;
}

void OutputData::setHeight(quint64 height)
{
    m_height = height;
}

quint64 OutputData::lockHeight() const
{
    return m_lockHeight;
}

void OutputData::setLockHeight(quint64 lockHeight)
{
    m_lockHeight = lockHeight;
}

bool OutputData::isCoinbase() const
{
    return m_isCoinbase;
}

void OutputData::setIsCoinbase(bool isCoinbase)
{
    m_isCoinbase = isCoinbase;
}

uint OutputData::txLogEntry() const
{
    return m_txLogEntry;
}

void OutputData::setTxLogEntry(uint txLogEntry)
{
    m_txLogEntry = txLogEntry;
}

// JSON Methoden
void OutputData::fromJson(const QJsonObject &json)
{
    m_rootKeyId = json.value("root_key_id").toString();
    m_keyId = json.value("key_id").toString();
    m_nChild = json.value("n_child").toInt();

    // commit optional als String, default leer
    m_commit = json.value("commit").toString();

    // mmr_index optional, falls nicht da 0
    m_mmrIndex = static_cast<quint64>(json.value("mmr_index").toDouble());

    m_value = static_cast<quint64>(json.value("value").toDouble());

    // Status aus String oder int (hier als String)
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

    // tx_log_entry optional, falls nicht da 0
    m_txLogEntry = static_cast<uint>(json.value("tx_log_entry").toInt());
}

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
