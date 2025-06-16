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
QJsonObject WalletForeignApi::buildCoinbase()
{
    QJsonObject params;
    return post("build_coinbase", params);
}

/**
 * @brief WalletForeignApi::checkVersion
 * @return
 */
QJsonObject WalletForeignApi::checkVersion()
{
    QJsonObject params;
    return post("check_version", params);
}

/**
 * @brief WalletForeignApi::finalizeTx
 * @return
 */
QJsonObject WalletForeignApi::finalizeTx()
{
    QJsonObject params;
    return post("finalize_tx", params);
}

/**
 * @brief WalletForeignApi::receiveTx
 * @param slate
 * @param destAcctName
 * @param rAddr
 * @return
 */
QJsonObject WalletForeignApi::receiveTx(QJsonObject slate, QString destAcctName, QString dest)
{
    Q_UNUSED(destAcctName);
    Q_UNUSED(dest);

    QJsonObject params;
    params["slate"] = slate;
    params["dest_acct_name"] = QJsonValue::Null;
    params["dest"] = QJsonValue::Null;

    QJsonObject response = post("receive_tx", params);
    qDebug()<<response;

    if (response.contains("result") && response["result"].toObject().contains("Ok")) {
        QJsonObject slate = response["result"].toObject()["Ok"].toObject();

        if (slate.isEmpty()) {
            qWarning() << "slate is empty!";
            return QJsonObject();
        } else {
            return slate;
        }
    } else {
        if (response.contains("result") && response["result"].toObject().contains("Err")) {
            QJsonObject err = response["result"].toObject()["Err"].toObject();

            if (err.isEmpty()) {
                qWarning() << "error by read error message!";
                return QJsonObject();
            } else {
                return err;
            }
        }
    }

    return QJsonObject();
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
