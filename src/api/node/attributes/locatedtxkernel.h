#ifndef LOCATEDTXKERNEL_H
#define LOCATEDTXKERNEL_H

#include <cstdint>
#include "txkernel.h"

class LocatedTxKernel
{
public:
    LocatedTxKernel();
    LocatedTxKernel(const TxKernel &txKernel, uint64_t height, uint64_t mmrIndex);

    // Getter
    TxKernel txKernel() const;
    uint64_t height() const;
    uint64_t mmrIndex() const;

    // Setter
    void setTxKernel(const TxKernel &txKernel);
    void setHeight(uint64_t height);
    void setMmrIndex(uint64_t mmrIndex);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    TxKernel m_txKernel;
    uint64_t m_height;
    uint64_t m_mmrIndex;
};

#endif // LOCATEDTXKERNEL_H
