#include "locatedtxkernel.h"

/**
 * @brief LocatedTxKernel::LocatedTxKernel
 */
LocatedTxKernel::LocatedTxKernel() :
    m_height(0),
    m_mmrIndex(0)
{
}

/**
 * @brief LocatedTxKernel::LocatedTxKernel
 * @param txKernel
 * @param height
 * @param mmrIndex
 */
LocatedTxKernel::LocatedTxKernel(const TxKernel &txKernel, quint64 height, quint64 mmrIndex) :
    m_txKernel(txKernel),
    m_height(height),
    m_mmrIndex(mmrIndex)
{
}

/**
 * @brief LocatedTxKernel::txKernel
 * @return
 */
TxKernel LocatedTxKernel::txKernel() const
{
    return m_txKernel;
}

/**
 * @brief LocatedTxKernel::height
 * @return
 */
quint64 LocatedTxKernel::height() const
{
    return m_height;
}

/**
 * @brief LocatedTxKernel::mmrIndex
 * @return
 */
quint64 LocatedTxKernel::mmrIndex() const
{
    return m_mmrIndex;
}

/**
 * @brief LocatedTxKernel::setTxKernel
 * @param txKernel
 */
void LocatedTxKernel::setTxKernel(const TxKernel &txKernel)
{
    m_txKernel = txKernel;
}

/**
 * @brief LocatedTxKernel::setHeight
 * @param height
 */
void LocatedTxKernel::setHeight(quint64 height)
{
    m_height = height;
}

/**
 * @brief LocatedTxKernel::setMmrIndex
 * @param mmrIndex
 */
void LocatedTxKernel::setMmrIndex(quint64 mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

/**
 * @brief LocatedTxKernel::fromJson
 * @param json
 */
void LocatedTxKernel::fromJson(const QJsonObject &json)
{
    if (json.contains("height") && json["height"].isDouble()) {
        m_height = static_cast<quint64>(json["height"].toDouble());
    }

    if (json.contains("mmr_index") && json["mmr_index"].isDouble()) {
        m_mmrIndex = static_cast<quint64>(json["mmr_index"].toDouble());
    }

    if (json.contains("tx_kernel") && json["tx_kernel"].isObject()) {
        m_txKernel.fromJson(json["tx_kernel"].toObject());
    }
}

/**
 * @brief LocatedTxKernel::toJson
 * @return
 */
QJsonObject LocatedTxKernel::toJson() const
{
    QJsonObject json;
    json["height"] = static_cast<double>(m_height);
    json["mmr_index"] = static_cast<double>(m_mmrIndex);
    json["tx_kernel"] = m_txKernel.toJson();
    return json;
}
