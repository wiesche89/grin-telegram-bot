#include "walletforeignapi.h"

/**
 * @brief WalletForeignApi::WalletForeignApi
 * @param apiUrl
 */
WalletForeignApi::WalletForeignApi(QString apiUrl) :
    m_apiUrl(apiUrl),
    m_networkManager(nullptr)
{
    m_networkManager = new QNetworkAccessManager();
}

/**
 * @brief WalletForeignApi::buildCoinbase
 * Builds a new unconfirmed coinbase output in the wallet, generally for inclusion in a
 * potential new block’s coinbase output during mining.
 * All potential coinbase outputs are created as ‘Unconfirmed’ with the coinbase flag set.
 * If a potential coinbase output is found on the chain after a wallet update, it status is
 * set to Unsent and a Transaction Log Entry will be created. Note the output will be
 * unspendable until the coinbase maturity period has expired.
 * @param fees
 * @param height
 * @param keyId
 * @return
 */
Result<Coinbase> WalletForeignApi::buildCoinbase(int fees, int height, QString keyId)
{
    QJsonObject params;
    params["fees"] = fees;
    params["height"] = height;
    params["key_id"] = keyId.isEmpty() ? QJsonValue::Null : QJsonValue(keyId);

    auto res = JsonUtil::extractOkObject(post("build_coinbase", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Coinbase::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::checkVersion
 * Return the version capabilities of the running ForeignApi Node
 * @return
 */
Result<Version> WalletForeignApi::checkVersion()
{
    QJsonObject params;

    auto res = JsonUtil::extractOkObject(post("check_version", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Version::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::finalizeTx
 * Finalizes a (standard or invoice) transaction initiated by this wallet’s Owner api. This step assumes
 * the paying party has completed round 1 and 2 of slate creation, and added their partial signatures.
 * This wallet will verify and add their partial sig, then create the finalized transaction, ready to post to a node.
 * This function posts to the node if the post_automatically argument is sent to true. Posting can be
 * done in separately via the post_tx function. This function also stores the final transaction in
 * the user’s wallet files for retrieval via the get_stored_tx function.
 * @param slate
 * @return
 */
Result<Slate> WalletForeignApi::finalizeTx(Slate slate)
{
    QJsonObject params;
    QJsonArray paramsArray;
    paramsArray.append(slate.toJson());
    params["params"] = paramsArray;

    auto res = JsonUtil::extractOkObject(post("finalize_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletForeignApi::receiveTx
 * Recieve a tranaction created by another party, returning the modified Slate object, modified with the recipient’s
 * output for the transaction amount, and public signature data. This slate can then be sent back to the sender to
 * finalize the transaction via the Owner API’s finalize_tx method.
 * This function creates a single output for the full amount, set to a status of ‘Awaiting finalization’.
 * It will remain in this state until the wallet finds the corresponding output on the chain, at which point it
 * will become ‘Unspent’. The slate will be updated with the results of Signing round 1 and 2, adding the recipient’s
 * public nonce, public excess value, and partial signature to the slate.
 * Also creates a corresponding Transaction Log Entry in the wallet’s transaction log.
 * @param slate
 * @param destAcctName
 * @param dest
 * @return
 */
Result<Slate> WalletForeignApi::receiveTx(Slate slate, QString destAcctName, QString dest)
{
    Q_UNUSED(destAcctName);
    Q_UNUSED(dest);

    QJsonObject params;
    params["slate"] = slate.toJson();
    params["dest_acct_name"] = QJsonValue::Null;
    params["dest"] = QJsonValue::Null;

    auto res = JsonUtil::extractOkObject(post("receive_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

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
