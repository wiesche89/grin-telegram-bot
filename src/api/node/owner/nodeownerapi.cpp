#include "nodeownerapi.h"

/**
 * @brief NodeOwnerApi::NodeOwnerApi
 * @param apiUrl
 * @param apiKey
 */
NodeOwnerApi::NodeOwnerApi(QString apiUrl, QString apiKey) :
    m_apiUrl(apiUrl),
    m_apiKey(apiKey),
    m_networkManager(new QNetworkAccessManager())
{
}

/**
 * @brief NodeOwnerApi::banPeer
 * Bans a specific peer.
 * @param peerAddr
 * @return
 */
Result<bool> NodeOwnerApi::banPeer(QString peerAddr)
{
    QJsonArray params;
    params.append(QJsonValue(peerAddr));

    auto res = JsonUtil::extractOkValue(post("ban_peer", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief NodeOwnerApi::compactChain
 * Trigger a compaction of the chain state to regain storage space.
 * @return
 */
Result<bool> NodeOwnerApi::compactChain()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkValue(post("compact_chain", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief NodeOwnerApi::getConnectedPeers
 * Retrieves a list of all connected peers.
 * @return
 */
Result<QList<PeerInfoDisplay> > NodeOwnerApi::getConnectedPeers()
{
    QList<PeerInfoDisplay> peers;
    QJsonArray params;

    auto res = JsonUtil::extractOkValue(post("get_connected_peers", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray array = OkVal.toArray();
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
 * Retrieves information about peers. If null is provided, get_peers will list all stored peers.
 * @param peerAddr
 * @return
 */
Result<QList<PeerData> > NodeOwnerApi::getPeers(QString peerAddr)
{
    QList<PeerData> peers;
    QJsonArray params;

    params.append(QJsonValue(peerAddr));

    auto res = JsonUtil::extractOkValue(post("get_peers", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();

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
 * Returns various information about the node, the network and the current sync status.
 * @return
 */
Result<Status> NodeOwnerApi::getStatus()
{
    Status status;
    QJsonArray params;

    auto res = JsonUtil::extractOkObject(post("get_status", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    status = Status::fromJson(okObj);

    return status;
}

/**
 * @brief NodeOwnerApi::unbanPeer
 * Unbans a specific peer.
 * @param peerAddr
 * @return
 */
Result<bool> NodeOwnerApi::unbanPeer(QString peerAddr)
{
    QJsonArray params;
    params.append(peerAddr);

    auto res = JsonUtil::extractOkValue(post("unban_peer", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief NodeOwnerApi::validateChain
 * Trigger a validation of the chain state.
 * @param assumeValidRangeproofsKernels
 * @return
 */
Result<bool> NodeOwnerApi::validateChain(bool assumeValidRangeproofsKernels)
{
    QJsonArray params;
    params.append(QJsonValue(assumeValidRangeproofsKernels));

    auto res = JsonUtil::extractOkValue(post("validate_chain", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
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
