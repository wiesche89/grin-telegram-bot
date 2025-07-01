#ifndef TRANSACTIONBODY_H
#define TRANSACTIONBODY_H

#include <QJsonObject>
#include <QVector>
#include <QJsonArray>

#include "input.h"
#include "output.h"
#include "txkernel.h"

class TransactionBody
{
public:
    TransactionBody();

    QVector<Input> inputs() const;
    void setInputs(const QVector<Input> &inputs);

    QVector<Output> outputs() const;
    void setOutputs(const QVector<Output> &outputs);

    QVector<TxKernel> kernels() const;
    void setKernels(const QVector<TxKernel> &kernels);

    QJsonObject toJson() const;
    static TransactionBody fromJson(const QJsonObject &json);

private:
    QVector<Input> m_inputs;
    QVector<Output> m_outputs;
    QVector<TxKernel> m_kernels;
};

#endif // TRANSACTIONBODY_H
