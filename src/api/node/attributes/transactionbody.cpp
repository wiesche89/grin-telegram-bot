#include "transactionbody.h"

TransactionBody::TransactionBody()
{
}

QVector<Input> TransactionBody::inputs() const
{
    return m_inputs;
}

void TransactionBody::setInputs(const QVector<Input> &inputs)
{
    m_inputs = inputs;
}

QVector<Output> TransactionBody::outputs() const
{
    return m_outputs;
}

void TransactionBody::setOutputs(const QVector<Output> &outputs)
{
    m_outputs = outputs;
}

QVector<TxKernel> TransactionBody::kernels() const
{
    return m_kernels;
}

void TransactionBody::setKernels(const QVector<TxKernel> &kernels)
{
    m_kernels = kernels;
}

QJsonObject TransactionBody::toJson() const
{
    QJsonObject json;

    QJsonArray inputsArray;
    for (const Input &input : m_inputs) {
        inputsArray.append(input.toJson());
    }
    json["inputs"] = inputsArray;

    QJsonArray outputsArray;
    for (const Output &output : m_outputs) {
        outputsArray.append(output.toJson());
    }
    json["outputs"] = outputsArray;

    QJsonArray kernelsArray;
    for (const TxKernel &kernel : m_kernels) {
        kernelsArray.append(kernel.toJson());
    }
    json["kernels"] = kernelsArray;

    return json;
}

TransactionBody TransactionBody::fromJson(const QJsonObject &json)
{
    TransactionBody body;

    QJsonArray inputsArray = json["inputs"].toArray();
    for (const QJsonValue &val : inputsArray) {
        body.m_inputs.append(Input::fromJson(val.toObject()));
    }

    QJsonArray outputsArray = json["outputs"].toArray();
    for (const QJsonValue &val : outputsArray) {
        body.m_outputs.append(Output::fromJson(val.toObject()));
    }

    QJsonArray kernelsArray = json["kernels"].toArray();
    for (const QJsonValue &val : kernelsArray) {
        TxKernel txkernel;
        txkernel.fromJson(val.toObject());
        body.m_kernels.append(txkernel);
    }

    return body;
}
