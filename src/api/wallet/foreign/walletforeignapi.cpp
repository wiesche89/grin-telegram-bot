#include "walletforeignapi.h"

/**
 * @brief WalletForeignApi::WalletForeignApi
 * @param apiUrl
 * @param user
 * @param password
 */
WalletForeignApi::WalletForeignApi(QString apiUrl) :
    m_apiUrl(apiUrl),
    m_networkManager(nullptr)
{
    m_networkManager = new QNetworkAccessManager();
}

/**
 * @brief WalletForeignApi::buildCoinbase
 * @return
 */
Coinbase WalletForeignApi::buildCoinbase(int fees, int height, QString keyId)
{
    QJsonObject params;
    params["fees"] = fees;
    params["height"] = height;
    params["keyId"] = QJsonValue::Null;

    QJsonObject rpcJson = post("build_coinbase", params);

    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return Coinbase();
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return Coinbase();
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    return Coinbase::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::checkVersion
 * @return
 */
Version WalletForeignApi::checkVersion()
{
    QJsonObject params;

    QJsonObject rpcJson = post("check_version", params);

    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return Version();
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return Version();
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    return Version::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::finalizeTx
 * @return
 */
Slate WalletForeignApi::finalizeTx(Slate slate)
{
    QJsonObject params;
    QJsonArray paramsArray;
    paramsArray.append(slate.toJson());
    params["params"] = paramsArray;

    QJsonObject rpcJson = post("finalize_tx", params);

    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return Slate();
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return Slate();
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::receiveTx
 * @param slate
 * @param destAcctName
 * @param rAddr
 * @return
 */
Slate WalletForeignApi::receiveTx(Slate slate, QString destAcctName, QString dest)
{
    Q_UNUSED(destAcctName);
    Q_UNUSED(dest);

    QJsonObject params;
    params["slate"] = slate.toJson();
    params["dest_acct_name"] = QJsonValue::Null;
    params["dest"] = QJsonValue::Null;

    QJsonObject rpcJson = post("receive_tx", params);

    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        Error err;
        Slate slate;

        err.setMessage("Unknown error: "+ QJsonDocument(rpcJson).toJson(QJsonDocument::Compact));
        slate.setError(err);

        return slate;
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {

        Error err;
        Slate slate;

        err.parseFromJson(resultObj);
        slate.setError(err);

        return slate;
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject WalletForeignApi::post(const QString &method, const QJsonObject &params)
{
    QJsonObject payload;
    payload["jsonrpc"] = "2.0";
    payload["id"] = 0;
    payload["method"] = method;
    payload["params"] = params;

    // Setup HTTP POST
    QUrl url(m_apiUrl);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QEventLoop loop;
    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error:" << reply->errorString();
        reply->deleteLater();
        return QJsonObject();
    }

    QByteArray response_data = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(response_data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return QJsonObject();
    }

    if (!jsonDoc.isObject()) {
        qWarning() << "JSON is not an object";
        return QJsonObject();
    }

    return jsonDoc.object();
}
