#ifndef LOCATEDTXKERNEL_H
#define LOCATEDTXKERNEL_H

#include "txkernel.h"

class LocatedTxKernel
{
public:
    LocatedTxKernel();
    LocatedTxKernel(const TxKernel &txKernel, quint64 height, quint64 mmrIndex);

    // Getter
    TxKernel txKernel() const;
    quint64 height() const;
    quint64 mmrIndex() const;

    // Setter
    void setTxKernel(const TxKernel &txKernel);
    void setHeight(quint64 height);
    void setMmrIndex(quint64 mmrIndex);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    TxKernel m_txKernel;
    quint64 m_height;
    quint64 m_mmrIndex;
};

#endif // LOCATEDTXKERNEL_H
