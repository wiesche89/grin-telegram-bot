#include "nodeownerapi.h"

/**
 * @brief NodeOwnerApi::NodeOwnerApi
 */
NodeOwnerApi::NodeOwnerApi(QString apiUrl, QString apiKey) :
    m_apiUrl(apiUrl),
    m_apiKey(apiKey),
    m_networkManager(new QNetworkAccessManager())
{
}

/**
 * @brief NodeOwnerApi::banPeer
 * @return
 */
QJsonObject NodeOwnerApi::banPeer()
{
    QJsonObject params;
    return post("ban_peer", params);
}

/**
 * @brief NodeOwnerApi::compactChain
 * @return
 */
QJsonObject NodeOwnerApi::compactChain()
{
    QJsonObject params;
    return post("compact_chain", params);
}

/**
 * @brief NodeOwnerApi::getConnectedPeers
 * @return
 */
QJsonObject NodeOwnerApi::getConnectedPeers()
{
    QJsonObject params;
    return post("get_connected_peers", params);
}

/**
 * @brief NodeOwnerApi::getPeers
 * @return
 */
QJsonObject NodeOwnerApi::getPeers()
{
    QJsonObject params;
    return post("get_peers", params);
}

/**
 * @brief NodeOwnerApi::getStatus
 * @return
 */
QJsonObject NodeOwnerApi::getStatus()
{
    QJsonObject params;
    return post("get_status", params);
}

/**
 * @brief NodeOwnerApi::invalidateHeader
 * @return
 */
QJsonObject NodeOwnerApi::invalidateHeader()
{
    QJsonObject params;
    return post("invalidate_header", params);
}

/**
 * @brief NodeOwnerApi::resetChainHead
 * @return
 */
QJsonObject NodeOwnerApi::resetChainHead()
{
    QJsonObject params;
    return post("reset_chain_head", params);
}

/**
 * @brief NodeOwnerApi::unbanPeer
 * @return
 */
QJsonObject NodeOwnerApi::unbanPeer()
{
    QJsonObject params;
    return post("unban_peer", params);
}

/**
 * @brief NodeOwnerApi::validateChain
 * @return
 */
QJsonObject NodeOwnerApi::validateChain()
{
    QJsonObject params;
    return post("validate_chain", params);
}

/**
 * @brief NodeOwnerApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject NodeOwnerApi::post(const QString &method, const QJsonObject &params)
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
