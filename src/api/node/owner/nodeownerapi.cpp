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
bool NodeOwnerApi::banPeer(QString peerAddr)
{
    QJsonArray params;
    params.append(QJsonValue(peerAddr));

    QJsonObject rpcJson = post("ban_peer", params);

    // Check if "result" exists and is an object
    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return false;
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    // The "Ok" key must exist — its value can be either null or an object
    return resultObj.contains("Ok");
}

/**
 * @brief NodeOwnerApi::compactChain
 * @return
 */
bool NodeOwnerApi::compactChain()
{
    QJsonArray params;

    QJsonObject rpcJson = post("compact_chain", params);

    // Check if "result" exists and is an object
    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return false;
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    // The "Ok" key must exist — its value can be either null or an object
    return resultObj.contains("Ok");
}

/**
 * @brief NodeOwnerApi::getConnectedPeers
 * @return
 */
QList<PeerInfoDisplay> NodeOwnerApi::getConnectedPeers()
{
    QList<PeerInfoDisplay> peers;
    QJsonArray params;

    QJsonObject root = post("get_connected_peers", params);

    if (!root.contains("result")) {
        return peers;
    }

    QJsonObject result = root["result"].toObject();
    if (!result.contains("Ok")) {
        return peers;
    }

    QJsonArray array = result["Ok"].toArray();
    for (const QJsonValue &val : array) {
        if (!val.isObject()) {
            continue;
        }

        QJsonObject peerObj = val.toObject();
        PeerInfoDisplay peer = PeerInfoDisplay::fromJson(peerObj);
        peers.append(peer);
    }

    return peers;
}

/**
 * @brief NodeOwnerApi::getPeers
 * @return
 */
QList<PeerData> NodeOwnerApi::getPeers(QString peerAddr)
{
    QList<PeerData> peers;
    QJsonArray params;

    params.append(QJsonValue(peerAddr));

    QJsonObject rootObj = post("get_peers", params);

    if (!rootObj.contains("result")) {
        return peers;
    }
    QJsonObject resultObj = rootObj["result"].toObject();

    if (!resultObj.contains("Ok")) {
        return peers;
    }
    QJsonArray okArray = resultObj["Ok"].toArray();

    for (const QJsonValue &val : okArray) {
        if (!val.isObject()) {
            continue;
        }
        QJsonObject peerObj = val.toObject();

        PeerData peer = PeerData::fromJson(peerObj);
        peers.append(peer);
    }

    return peers;
}

/**
 * @brief NodeOwnerApi::getStatus
 * @return
 */
Status NodeOwnerApi::getStatus()
{
    Status status;
    QJsonArray params;

    QJsonObject obj = post("get_status", params);

    if (!obj.contains("result") || !obj["result"].isObject()) {
        return status;
    }

    QJsonObject resultObj = obj["result"].toObject();
    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return status;
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    status = Status::fromJson(okObj);

    return status;
}

/**
 * @brief NodeOwnerApi::invalidateHeader
 * @return
 */
QJsonObject NodeOwnerApi::invalidateHeader()
{
    QJsonArray params;
    return post("invalidate_header", params);
}

/**
 * @brief NodeOwnerApi::resetChainHead
 * @return
 */
QJsonObject NodeOwnerApi::resetChainHead()
{
    QJsonArray params;
    return post("reset_chain_head", params);
}

/**
 * @brief NodeOwnerApi::unbanPeer
 * @return
 */
QJsonObject NodeOwnerApi::unbanPeer(QString peerAddr)
{
    QJsonArray params;
    return post("unban_peer", params);
}

/**
 * @brief NodeOwnerApi::validateChain
 * @return
 */
QJsonObject NodeOwnerApi::validateChain(bool assumeValidRangeproofsKernels)
{
    QJsonArray params;
    return post("validate_chain", params);
}

/**
 * @brief NodeOwnerApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject NodeOwnerApi::post(const QString &method, const QJsonArray &params)
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
