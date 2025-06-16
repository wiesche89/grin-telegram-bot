#include "nodeforeignapi.h"

NodeForeignApi::NodeForeignApi(QString apiUrl, QString apiKey) :
    m_apiUrl(apiUrl),
    m_apiKey(apiKey),
    m_networkManager(nullptr)
{
    m_networkManager = new QNetworkAccessManager();
}

/**
 * @brief NodeForeignApi::getBlock
 * @return
 */
QJsonObject NodeForeignApi::getBlock()
{
    QJsonObject params;
    return post("get_block", params);
}

/**
 * @brief NodeForeignApi::getBlocks
 * @return
 */
QJsonObject NodeForeignApi::getBlocks()
{
    QJsonObject params;
    return post("get_blocks", params);
}

/**
 * @brief NodeForeignApi::getHeader
 * @return
 */
QJsonObject NodeForeignApi::getHeader()
{
    QJsonObject params;
    return post("get_header", params);
}

/**
 * @brief NodeForeignApi::getKernel
 * @return
 */
QJsonObject NodeForeignApi::getKernel()
{
    QJsonObject params;
    return post("get_kernel", params);
}

/**
 * @brief NodeForeignApi::getOutputs
 * @return
 */
QJsonObject NodeForeignApi::getOutputs()
{
    QJsonObject params;
    return post("get_outputs", params);
}

/**
 * @brief NodeForeignApi::getPmmrIndices
 * @return
 */
QJsonObject NodeForeignApi::getPmmrIndices()
{
    QJsonObject params;
    return post("get_pmmr_indices", params);
}

/**
 * @brief NodeForeignApi::getPoolSize
 * @return
 */
QJsonObject NodeForeignApi::getPoolSize()
{
    QJsonObject params;
    return post("get_pool_size", params);
}

/**
 * @brief NodeForeignApi::getStempoolSize
 * @return
 */
QJsonObject NodeForeignApi::getStempoolSize()
{
    QJsonObject params;
    return post("get_stempool_size", params);
}

/**
 * @brief NodeForeignApi::getTip
 * @return
 */
QJsonObject NodeForeignApi::getTip()
{
    QJsonObject params;
    return post("get_tip", params);
}

/**
 * @brief NodeForeignApi::getUnconfirmedTransactions
 * @return
 */
QJsonObject NodeForeignApi::getUnconfirmedTransactions()
{
    QJsonObject params;
    return post("get_unconfirmed_transactions", params);
}

/**
 * @brief NodeForeignApi::getUnspentOutputs
 * @return
 */
QJsonObject NodeForeignApi::getUnspentOutputs()
{
    QJsonObject params;
    return post("get_unspent_outputs", params);
}

/**
 * @brief NodeForeignApi::getVersion
 * @return
 */
QJsonObject NodeForeignApi::getVersion()
{
    QJsonObject params;
    return post("get_version", params);
}

/**
 * @brief NodeForeignApi::pushTransaction
 * @return
 */
QJsonObject NodeForeignApi::pushTransaction()
{
    QJsonObject params;
    return post("push_transaction", params);
}

/**
 * @brief NodeForeignApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject NodeForeignApi::post(const QString &method, const QJsonObject &params)
{
    QUrl url(m_apiUrl);
    QNetworkRequest request(url);
    QEventLoop loop;

    QObject::connect(m_networkManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", m_apiKey.toUtf8());

    // JSON-Payload
    QJsonObject payload;
    payload["jsonrpc"] = "2.0";
    payload["method"] = method;
    payload["params"] = params;
    payload["id"] = 1;

    // JSON-Document
    QJsonDocument jsonDoc(payload);
    QByteArray jsonData = jsonDoc.toJson();

    // POST
    QNetworkReply *reply = m_networkManager->post(request, jsonData);

    loop.exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(response, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << parseError.errorString();
            return QJsonObject();
        }

        if (!jsonDoc.isObject()) {
            qWarning() << "JSON is not an object";
            return QJsonObject();
        }
        return jsonDoc.object();
    } else {
        // print error
        qDebug() << "Error: " << reply->errorString();
    }
    reply->deleteLater();

    return QJsonObject();
}
