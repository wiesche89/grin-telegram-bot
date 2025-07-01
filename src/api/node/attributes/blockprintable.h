#ifndef BLOCKPRINTABLE_H
#define BLOCKPRINTABLE_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include "blockheaderprintable.h"
#include "outputprintable.h"
#include "txkernelprintable.h"

class BlockPrintable
{
public:
    BlockPrintable();

    // Getter
    BlockHeaderPrintable header() const;
    QVector<QString> inputs() const;
    QVector<OutputPrintable> outputs() const;
    QVector<TxKernelPrintable> kernels() const;

    // Setter
    void setHeader(const BlockHeaderPrintable &header);
    void setInputs(const QVector<QString> &inputs);
    void setOutputs(const QVector<OutputPrintable> &outputs);
    void setKernels(const QVector<TxKernelPrintable> &kernels);

    // JSON parsing and serialization
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    BlockHeaderPrintable m_header;
    QVector<QString> m_inputs;
    QVector<OutputPrintable> m_outputs;
    QVector<TxKernelPrintable> m_kernels;
};

#endif // BLOCKPRINTABLE_H
