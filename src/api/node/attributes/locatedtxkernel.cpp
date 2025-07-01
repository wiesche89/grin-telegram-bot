#include "locatedtxkernel.h"

LocatedTxKernel::LocatedTxKernel() :
    m_height(0),
    m_mmrIndex(0)
{
}

LocatedTxKernel::LocatedTxKernel(const TxKernel &txKernel, uint64_t height, uint64_t mmrIndex) :
    m_txKernel(txKernel),
    m_height(height),
    m_mmrIndex(mmrIndex)
{
}

TxKernel LocatedTxKernel::txKernel() const
{
    return m_txKernel;
}

uint64_t LocatedTxKernel::height() const
{
    return m_height;
}

uint64_t LocatedTxKernel::mmrIndex() const
{
    return m_mmrIndex;
}

void LocatedTxKernel::setTxKernel(const TxKernel &txKernel)
{
    m_txKernel = txKernel;
}

void LocatedTxKernel::setHeight(uint64_t height)
{
    m_height = height;
}

void LocatedTxKernel::setMmrIndex(uint64_t mmrIndex)
{
    m_mmrIndex = mmrIndex;
}

void LocatedTxKernel::fromJson(const QJsonObject &json)
{
    if (json.contains("height") && json["height"].isDouble()) {
        m_height = static_cast<uint64_t>(json["height"].toDouble());
    }

    if (json.contains("mmr_index") && json["mmr_index"].isDouble()) {
        m_mmrIndex = static_cast<uint64_t>(json["mmr_index"].toDouble());
    }

    if (json.contains("tx_kernel") && json["tx_kernel"].isObject()) {
        m_txKernel.fromJson(json["tx_kernel"].toObject());
    }
}

QJsonObject LocatedTxKernel::toJson() const
{
    QJsonObject json;
    json["height"] = static_cast<double>(m_height);
    json["mmr_index"] = static_cast<double>(m_mmrIndex);
    json["tx_kernel"] = m_txKernel.toJson();
    return json;
}
