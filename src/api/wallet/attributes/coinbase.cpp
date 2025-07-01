#include "coinbase.h"

/**
 * @brief Coinbase::Coinbase
 */
Coinbase::Coinbase() = default;

/**
 * @brief Coinbase::Coinbase
 * @param kernel
 * @param keyId
 * @param output
 */
Coinbase::Coinbase(const Kernel &kernel, const QString &keyId, const Output &output) :
    m_kernel(kernel),
    m_keyId(keyId),
    m_output(output)
{
}

/**
 * @brief Coinbase::toJson
 * @return
 */
QJsonObject Coinbase::toJson() const
{
    QJsonObject obj;
    obj["kernel"] = m_kernel.toJson();
    obj["key_id"] = m_keyId;
    obj["output"] = m_output.toJson();
    return obj;
}

/**
 * @brief Coinbase::fromJson
 * @param obj
 * @return
 */
Coinbase Coinbase::fromJson(const QJsonObject &obj)
{
    return Coinbase(
        Kernel::fromJson(obj["kernel"].toObject()),
        obj["key_id"].toString(),
        Output::fromJson(obj["output"].toObject())
        );
}

/**
 * @brief Coinbase::kernel
 * @return
 */
Kernel Coinbase::kernel() const
{
    return m_kernel;
}

/**
 * @brief Coinbase::setKernel
 * @param kernel
 */
void Coinbase::setKernel(const Kernel &kernel)
{
    m_kernel = kernel;
}

/**
 * @brief Coinbase::keyId
 * @return
 */
QString Coinbase::keyId() const
{
    return m_keyId;
}

/**
 * @brief Coinbase::setKeyId
 * @param keyId
 */
void Coinbase::setKeyId(const QString &keyId)
{
    m_keyId = keyId;
}

/**
 * @brief Coinbase::output
 * @return
 */
Output Coinbase::output() const
{
    return m_output;
}

/**
 * @brief Coinbase::setOutput
 * @param output
 */
void Coinbase::setOutput(const Output &output)
{
    m_output = output;
}
