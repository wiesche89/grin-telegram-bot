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
BlockPrintable NodeForeignApi::getBlock(int height, QString hash, QString commit)
{
    BlockPrintable block;

    QJsonArray params;
    params.append(height == 0 ? QJsonValue::Null : QJsonValue(height));
    params.append(QJsonValue(hash));
    params.append(commit.isEmpty() ? QJsonValue::Null : QJsonValue(commit));

    QJsonObject rootObj = post("get_block", params);

    if (rootObj.contains("result") && rootObj["result"].isObject()) {
        QJsonObject resultObj = rootObj["result"].toObject();
        if (resultObj.contains("Ok") && resultObj["Ok"].isObject()) {
            QJsonObject okObj = resultObj["Ok"].toObject();

            block.fromJson(okObj);
        }
    }
    return block;
}

/**
 * @brief NodeForeignApi::getBlocks
 * @return
 */
BlockListing NodeForeignApi::getBlocks(int startHeight, int endHeight, int max, bool includeProof)
{
    BlockListing listing;
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(max));
    params.append(QJsonValue(includeProof));

    QJsonObject rootObj = post("get_blocks", params);

    if (rootObj.contains("result")) {
        QJsonObject resultObj = rootObj["result"].toObject();
        if (resultObj.contains("Ok")) {
            QJsonObject okObj = resultObj["Ok"].toObject();

            listing.fromJson(okObj);
        }
    }

    return listing;
}

/**
 * @brief NodeForeignApi::getHeader
 * @return
 */
BlockHeaderPrintable NodeForeignApi::getHeader(int height, QString hash, QString commit)
{
    QJsonArray params;
    params.append(height == 0 ? QJsonValue::Null : QJsonValue(height));
    params.append(QJsonValue(hash));
    params.append(commit.isEmpty() ? QJsonValue::Null : QJsonValue(commit));

    QJsonObject rootObj = post("get_header", params);

    // Pfad: rootObj -> "result" -> "Ok" -> Objekt
    QJsonObject resultObj = rootObj.value("result").toObject();
    QJsonValue okVal = resultObj.value("Ok");
    if (!okVal.isObject()) {
        return BlockHeaderPrintable();
    }

    QJsonObject blockHeaderJson = okVal.toObject();

    BlockHeaderPrintable blockHeader;
    blockHeader.fromJson(blockHeaderJson);

    return blockHeader;
}

/**
 * @brief NodeForeignApi::getKernel
 * @return
 */
LocatedTxKernel NodeForeignApi::getKernel(QString excess, int minHeight, int maxHeight)
{
    LocatedTxKernel locatedTxKernel;
    QJsonArray params;
    params.append(QJsonValue(excess));
    params.append(QJsonValue(minHeight));
    params.append(QJsonValue(maxHeight));

    QJsonObject obj = post("get_kernel", params);

    if (obj.contains("result")) {
        QJsonObject resultObj = obj["result"].toObject();
        if (resultObj.contains("Ok")) {
            QJsonObject okObj = resultObj["Ok"].toObject();
            locatedTxKernel.fromJson(okObj);
        }
    }

    return locatedTxKernel;
}

/**
 * @brief NodeForeignApi::getOutputs
 * @return
 */
QList<OutputPrintable> NodeForeignApi::getOutputs(QJsonArray commits, int startHeight, int endHeight, bool includeProof,
                                                  bool includeMerkleProof)
{
    QList<OutputPrintable> list;
    QJsonArray params;
    params.append(commits);
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(includeProof));
    params.append(QJsonValue(includeMerkleProof));

    QJsonObject root = post("get_outputs", params);

    if (!root.contains("result") || !root.value("result").isObject()) {
        return list;
    }

    QJsonObject resultObj = root.value("result").toObject();

    if (!resultObj.contains("Ok") || !resultObj.value("Ok").isArray()) {
        return list;
    }

    QJsonArray okArray = resultObj.value("Ok").toArray();

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
 * @return
 */
OutputListing NodeForeignApi::getPmmrIndices(int startHeight, int endHeight)
{
    OutputListing listing;
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));

    QJsonObject rootJson = post("get_pmmr_indices", params);

    if (!rootJson.contains("result") || !rootJson["result"].isObject()) {
        return listing;
    }
    QJsonObject resultObj = rootJson["result"].toObject();

    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return listing;
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    listing = OutputListing::fromJson(okObj);

    return listing;
}

/**
 * @brief NodeForeignApi::getPoolSize
 * @return
 */
int NodeForeignApi::getPoolSize()
{
    QJsonArray params;
    QJsonObject rootJson = post("get_pool_size", params);

    if (!rootJson.contains("result") || !rootJson["result"].isObject()) {
        return -1;
    }
    QJsonObject resultObj = rootJson["result"].toObject();

    if (!resultObj.contains("Ok")) {
        return -1;
    }

    // "Ok"
    QJsonValue okVal = resultObj["Ok"];
    if (okVal.isDouble()) {
        return okVal.toInt();
    }

    return -1;
}

/**
 * @brief NodeForeignApi::getStempoolSize
 * @return
 */
int NodeForeignApi::getStempoolSize()
{
    QJsonArray params;
    QJsonObject rootJson = post("get_stempool_size", params);

    if (!rootJson.contains("result") || !rootJson["result"].isObject()) {
        return -1;
    }
    QJsonObject resultObj = rootJson["result"].toObject();

    if (!resultObj.contains("Ok")) {
        return -1;
    }

    // "Ok"
    QJsonValue okVal = resultObj["Ok"];
    if (okVal.isDouble()) {
        return okVal.toInt();
    }

    return -1;
}

/**
 * @brief NodeForeignApi::getTip
 * @return
 */
Tip NodeForeignApi::getTip()
{
    QJsonArray params;
    QJsonObject root = post("get_tip", params);

    if (!root.contains("result") || !root["result"].isObject()) {
        qWarning() << "Kein result-Objekt";
        return Tip();
    }

    QJsonObject result = root["result"].toObject();
    if (!result.contains("Ok") || !result["Ok"].isObject()) {
        qWarning() << "Kein Ok-Objekt in result";
        return Tip();
    }

    QJsonObject ok = result["Ok"].toObject();

    return Tip::fromJson(ok);
}

/**
 * @brief NodeForeignApi::getUnconfirmedTransactions
 * @return
 */
QList<PoolEntry> NodeForeignApi::getUnconfirmedTransactions()
{
    QList<PoolEntry> entries;
    QJsonArray params;

    QJsonObject root = post("get_unconfirmed_transactions", params);

    if (root.contains("result") && root["result"].isObject()) {
        QJsonObject resultObj = root["result"].toObject();
        if (resultObj.contains("Ok")) {
            QJsonValue okValue = resultObj["Ok"];

            QJsonArray arr = okValue.toArray();
            for (const QJsonValue &v : arr) {
                if (!v.isObject()) {
                    continue;
                }
                PoolEntry entry = PoolEntry::fromJson(v.toObject());
                entries.append(entry);
            }
            return entries;
        }
    }

    return entries;
}

/**
 * @brief NodeForeignApi::getUnspentOutputs
 * @return
 */
BlockListing NodeForeignApi::getUnspentOutputs(int startHeight, int endHeight, int max, bool includeProof)
{
    BlockListing listing;
    QJsonArray params;
    params.append(QJsonValue(startHeight));
    params.append(QJsonValue(endHeight));
    params.append(QJsonValue(max));
    params.append(QJsonValue(includeProof));

    QJsonObject rootObj = post("get_unspent_outputs", params);

    if (rootObj.contains("result")) {
        QJsonObject resultObj = rootObj["result"].toObject();
        if (resultObj.contains("Ok")) {
            QJsonObject okObj = resultObj["Ok"].toObject();

            listing.fromJson(okObj);
        }
    }

    return listing;
}

/**
 * @brief NodeForeignApi::getVersion
 * @return
 */
NodeVersion NodeForeignApi::getVersion()
{
    QJsonArray params;

    QJsonObject rootObj = post("get_version", params);
    if (!rootObj.contains("result") || !rootObj["result"].isObject()) {
        return NodeVersion();
    }

    QJsonObject resultObj = rootObj["result"].toObject();
    if (!resultObj.contains("Ok") || !resultObj["Ok"].isObject()) {
        return NodeVersion();
    }

    QJsonObject okObj = resultObj["Ok"].toObject();

    return NodeVersion::fromJson(okObj);
}

/**
 * @brief NodeForeignApi::pushTransaction
 * @return
 */
bool NodeForeignApi::pushTransaction(Transaction tx, bool fluff)
{
    QJsonArray params;
    params.append(tx.toJson());
    params.append(QJsonValue(fluff));

    QJsonObject rpcJson = post("push_transaction", params);

    // Check if "result" exists and is an object
    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return false;
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    // The "Ok" key must exist — its value can be either null or an object
    return resultObj.contains("Ok");
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
        qDebug() << "Error: " << reply->errorString();
    }
    reply->deleteLater();

    return QJsonObject();
}
