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
 * Gets block details given either a height, a hash or an unspent output commitment.
 * Only one parameter is required. If multiple parameters are provided only the first one in the list is used.
 * @param height
 * @param hash
 * @param commit
 * @return
 */
Result<BlockPrintable> NodeForeignApi::getBlock(int height, QString hash, QString commit)
{
    QJsonArray params;
    params.append(height == 0 ? QJsonValue::Null : QJsonValue(height));
    params.append(QJsonValue(hash));
    params.append(commit.isEmpty() ? QJsonValue::Null : QJsonValue(commit));

    auto res = JsonUtil::extractOkObject(post("get_block", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    BlockPrintable block;
    block.fromJson(okObj);

    return block;
}

/**
 * @brief NodeForeignApi::getBlocks
 * @param startHeight
 * @param endHeight
 * @param max
 * @param includeProof
 * @return
 */
Result<BlockListing> NodeForeignApi::getBlocks(int startHeight, int endHeight, int max, bool includeProof)
{
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(max));
    params.append(QJsonValue(includeProof));

    auto res = JsonUtil::extractOkObject(post("get_blocks", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    BlockListing listing;
    listing.fromJson(okObj);

    return listing;
}

/**
 * @brief NodeForeignApi::getHeader
 * Gets block header given either a height, a hash or an unspent output commitment. Only one parameter is required.
 * If multiple parameters are provided only the first one in the list is used.
 * @param height
 * @param hash
 * @param commit
 * @return
 */
Result<BlockHeaderPrintable> NodeForeignApi::getHeader(int height, QString hash, QString commit)
{
    QJsonArray params;
    params.append(height == 0 ? QJsonValue::Null : QJsonValue(height));
    params.append(QJsonValue(hash));
    params.append(commit.isEmpty() ? QJsonValue::Null : QJsonValue(commit));

    auto res = JsonUtil::extractOkObject(post("get_header", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    BlockHeaderPrintable blockHeader;
    blockHeader.fromJson(okObj);

    return blockHeader;
}

/**
 * @brief NodeForeignApi::getKernel
 * Returns a LocatedTxKernel based on the kernel excess.
 * The min_height and max_height parameters are both optional.
 * If not supplied, min_height will be set to 0 and max_height will be set to the head of the chain.
 * The method will start at the block height max_height and traverse the kernel MMR backwards,
 * until either the kernel is found or min_height is reached.
 * @param excess
 * @param minHeight
 * @param maxHeight
 * @return
 */
Result<LocatedTxKernel> NodeForeignApi::getKernel(QString excess, int minHeight, int maxHeight)
{
    QJsonArray params;
    params.append(QJsonValue(excess));
    params.append(QJsonValue(minHeight));
    params.append(QJsonValue(maxHeight));

    auto res = JsonUtil::extractOkObject(post("get_kernel", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    LocatedTxKernel locatedTxKernel;
    locatedTxKernel.fromJson(okObj);

    return locatedTxKernel;
}

/**
 * @brief NodeForeignApi::getOutputs
 * Retrieves details about specifics outputs.
 * Supports retrieval of multiple outputs in a single request.
 * Support retrieval by both commitment string and block height.
 * Last field are for whether or not the response will include rangeproof and merkle proof.
 * @param commits
 * @param startHeight
 * @param endHeight
 * @param includeProof
 * @param includeMerkleProof
 * @return
 */
Result<QList<OutputPrintable> > NodeForeignApi::getOutputs(QJsonArray commits, int startHeight, int endHeight, bool includeProof,
                                                           bool includeMerkleProof)
{
    QJsonArray params;
    params.append(commits);
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(includeProof));
    params.append(QJsonValue(includeMerkleProof));

    auto res = JsonUtil::extractOkValue(post("get_outputs", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();
    QList<OutputPrintable> list;
    for (const QJsonValue &val : okArray) {
        if (!val.isObject()) {
            continue;
        }

        OutputPrintable output;
        output.fromJson(val.toObject());
        list.append(output);
    }

    return list;
}

/**
 * @brief NodeForeignApi::getPmmrIndices
 * Retrieves the PMMR indices based on the provided block height(s).
 * @param startHeight
 * @param endHeight
 * @return
 */
Result<OutputListing> NodeForeignApi::getPmmrIndices(int startHeight, int endHeight)
{
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));

    auto res = JsonUtil::extractOkObject(post("get_pmmr_indices", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return OutputListing::fromJson(okObj);
}

/**
 * @brief NodeForeignApi::getPoolSize
 * Returns the number of transactions in the transaction pool.
 * @return
 */
Result<int> NodeForeignApi::getPoolSize()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkValue(post("get_pool_size", params));
    QJsonValue okVal;

    if (!res.unwrapOrLog(okVal)) {
        return res.error();
    }

    if (okVal.isDouble()) {
        return okVal.toInt();
    }

    return Error(ErrorType::Unknown, QString("error map get_pool_size ok value!"));
}

/**
 * @brief NodeForeignApi::getStempoolSize
 * Returns the number of transactions in the stem transaction pool.
 * @return
 */
Result<int> NodeForeignApi::getStempoolSize()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkValue(post("get_stempool_size", params));
    QJsonValue okVal;

    if (!res.unwrapOrLog(okVal)) {
        return res.error();
    }

    if (okVal.isDouble()) {
        return okVal.toInt();
    }

    return Error(ErrorType::Unknown, QString("error map get_stempool_size ok value!"));
}

/**
 * @brief NodeForeignApi::getTip
 * Returns details about the state of the current fork tip.
 * @return
 */
Result<Tip> NodeForeignApi::getTip()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkObject(post("get_tip", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Tip::fromJson(okObj);
}

/**
 * @brief NodeForeignApi::getUnconfirmedTransactions
 * Returns the unconfirmed transactions in the transaction pool.
 * Will not return transactions in the stempool.
 * @return
 */
Result<QList<PoolEntry> > NodeForeignApi::getUnconfirmedTransactions()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkValue(post("get_unconfirmed_transactions", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QList<PoolEntry> entries;
    QJsonArray arr = OkVal.toArray();
    for (const QJsonValue &v : arr) {
        if (!v.isObject()) {
            continue;
        }
        PoolEntry entry = PoolEntry::fromJson(v.toObject());
        entries.append(entry);
    }

    return entries;
}

/**
 * @brief NodeForeignApi::getUnspentOutputs
 * UTXO traversal. Retrieves last utxos since a start index until a max.
 * Last boolean is optional to whether or not return the rangeproof.
 * @param startHeight
 * @param endHeight
 * @param max
 * @param includeProof
 * @return
 */
Result<BlockListing> NodeForeignApi::getUnspentOutputs(int startHeight, int endHeight, int max, bool includeProof)
{
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(max));
    params.append(QJsonValue(includeProof));

    auto res = JsonUtil::extractOkObject(post("get_unspent_outputs", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    BlockListing listing;
    listing.fromJson(okObj);

    return listing;
}

/**
 * @brief NodeForeignApi::getVersion
 * Returns the node version and block header version (used by grin-wallet).
 * @return
 */
Result<NodeVersion> NodeForeignApi::getVersion()
{
    QJsonArray params;

    auto res = JsonUtil::extractOkObject(post("get_version", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return NodeVersion::fromJson(okObj);
}

/**
 * @brief NodeForeignApi::pushTransaction
 * Push new transaction to our local transaction pool.
 * Optional fluff boolean to bypass Dandelion relay (false by default).
 * @param tx
 * @param fluff
 * @return
 */
Result<bool> NodeForeignApi::pushTransaction(Transaction tx, bool fluff)
{
    QJsonArray params;
    params.append(tx.toJson());
    params.append(QJsonValue(fluff));

    auto res = JsonUtil::extractOkValue(post("push_transaction", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief NodeForeignApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject NodeForeignApi::post(const QString &method, const QJsonArray &params)
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
        qDebug() << "Error " << Q_FUNC_INFO << " : " << reply->errorString();
    }
    reply->deleteLater();

    return QJsonObject();
}
