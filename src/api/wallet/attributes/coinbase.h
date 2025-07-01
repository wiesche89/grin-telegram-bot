#ifndef COINBASE_H
#define COINBASE_H

#include <QString>
#include <QJsonObject>

#include "kernel.h"
#include "output.h"

class Coinbase
{
public:
    Coinbase();
    Coinbase(const Kernel &kernel, const QString &keyId, const Output &output);

    QJsonObject toJson() const;
    static Coinbase fromJson(const QJsonObject &obj);

    Kernel kernel() const;
    void setKernel(const Kernel &kernel);

    QString keyId() const;
    void setKeyId(const QString &keyId);

    Output output() const;
    void setOutput(const Output &output);

private:
    Kernel m_kernel;
    QString m_keyId;
    Output m_output;
};

#endif // COINBASE_H
