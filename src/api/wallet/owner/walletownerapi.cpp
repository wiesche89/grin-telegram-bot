#include "walletownerapi.h"

/**
 * @brief WalletOwnerApi::WalletOwnerApi
 * @param apiUrl
 * @param apiUser
 * @param apiPassword
 * @param parent
 */
WalletOwnerApi::WalletOwnerApi(const QString &apiUrl, const QString &apiUser, const QString &apiPassword, QObject *parent) :
    QObject(parent),
    m_apiUrl(apiUrl),
    m_apiUser(apiUser),
    m_apiPassword(apiPassword),
    m_shareSecret(QByteArray()),
    m_openWalletToken(QByteArray()),
    m_privateKey(QByteArray()),
    m_publicKey(QByteArray()),
    m_networkManager(nullptr),
    m_secpContext(nullptr)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_secpContext = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    generateKeyPair();
}

/**
 * @brief WalletOwnerApi::generateKeyPair
 */
void WalletOwnerApi::generateKeyPair()
{
    // 1. Private-Key
    m_privateKey.resize(32);
    do {
        // Random 32 Byte generate
        RAND_bytes(reinterpret_cast<unsigned char *>(m_privateKey.data()), 32);
        // Verify key
    } while (!secp256k1_ec_seckey_verify(m_secpContext,
                                         reinterpret_cast<const unsigned char *>(m_privateKey.constData())));

    // 2. Public-Key
    secp256k1_pubkey pubkey;
    if (!secp256k1_ec_pubkey_create(m_secpContext, &pubkey,
                                    reinterpret_cast<const unsigned char *>(m_privateKey.constData()))) {
        qWarning() << "Failed to generate public key";
        return;
    }

    // 3. Public-Key serialize (compressed = 33 Byte)
    unsigned char pubkeySerialized[33];
    size_t outputLength = sizeof(pubkeySerialized);
    secp256k1_ec_pubkey_serialize(m_secpContext, pubkeySerialized, &outputLength,
                                  &pubkey, SECP256K1_EC_COMPRESSED);

    m_publicKey = QByteArray(reinterpret_cast<char *>(pubkeySerialized), outputLength);
}

/**
 * @brief WalletOwnerApi::hasConnection
 * @return
 */
bool WalletOwnerApi::hasConnection() const
{
    return !m_shareSecret.isEmpty();
}

/**
 * @brief WalletOwnerApi::post
 * @param method
 * @param params
 * @return
 */
QJsonObject WalletOwnerApi::post(const QString &method, const QJsonObject &params)
{
    QUrl url(m_apiUrl);
    QNetworkRequest request(url);
    QJsonObject payload;
    QEventLoop loop;

    payload["id"] = 1;
    payload["jsonrpc"] = "2.0";
    payload["method"] = method;
    payload["params"] = params;

    QJsonDocument doc(payload);
    QByteArray body = doc.toJson();

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", generateAuthHeader());

    // POST
    QObject::connect(m_networkManager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    QNetworkReply *reply = m_networkManager->post(request, body);

    loop.exec();

    // error
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Request failed: " << reply->errorString();
    }
    // response
    else {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();

            // response init_secure_api
            if (method == "init_secure_api") {
                if (obj.contains("result") && obj["result"].toObject().contains("Ok")) {
                    // 3. Get other public key (compress, 33 Byte)
                    QByteArray theirPubKey = QByteArray::fromHex(obj["result"].toObject()["Ok"].toString().toUtf8());
                    m_shareSecret = deriveEcdhKeyOpenSSL(m_privateKey.toHex(), theirPubKey);
                }
                return obj;
            }
            // encrypted request
            else if (method == "encrypted_request_v3") {
                if (obj.contains("result") && obj["result"].toObject().contains("Ok")) {
                    QByteArray body_enc = obj["result"].toObject()["Ok"].toObject()["body_enc"].toString().toUtf8();
                    QByteArray nonce = obj["result"].toObject()["Ok"].toObject()["nonce"].toString().toUtf8();
                    nonce = QByteArray::fromHex(nonce);

                    // decrypt
                    QString result = decrypt(m_shareSecret, body_enc, nonce);
                    QJsonDocument resultDoc = QJsonDocument::fromJson(result.toUtf8());
                    return resultDoc.object();
                }
                reply->deleteLater();
            } else {
                qWarning() << "JSON Parse Error: " << parseError.errorString();
            }
        }
    }
    // cleanup
    reply->deleteLater();
    return QJsonObject();
}

/**
 * @brief WalletOwnerApi::deriveEcdhKeyOpenSSL
 * @param secKeyHex
 * @param otherPubKeyCompressed
 * @return
 */
QByteArray WalletOwnerApi::deriveEcdhKeyOpenSSL(const QByteArray &secKeyHex, const QByteArray &otherPubKeyCompressed)
{
    //// secp256k1 = NID_secp256k1
    EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_secp256k1);
    if (!group) {
        qCritical() << "Failed to create EC_GROUP";
        return QByteArray();
    }

    // Create private key BIGNUM
    BIGNUM *priv_bn = BN_new();
    BN_hex2bn(&priv_bn, secKeyHex.constData());

    // Load public key from compressed form
    EC_POINT *pub_point = EC_POINT_new(group);
    if (!EC_POINT_oct2point(group, pub_point, reinterpret_cast<const unsigned char *>(otherPubKeyCompressed.constData()),
                            otherPubKeyCompressed.size(), nullptr)) {
        qCritical() << "Invalid public key";
        return QByteArray();
    }

    // Multiply pubkey by privkey: shared_point = priv * pub
    EC_POINT *shared_point = EC_POINT_new(group);
    if (!EC_POINT_mul(group, shared_point, nullptr, pub_point, priv_bn, nullptr)) {
        qCritical() << "ECDH multiplication failed";
        return QByteArray();
    }

    // Convert resulting point to compressed form
    unsigned char compressed[33];
    size_t len = EC_POINT_point2oct(group, shared_point, POINT_CONVERSION_COMPRESSED, compressed, sizeof(compressed), nullptr);

    // Clean up
    EC_GROUP_free(group);
    EC_POINT_free(pub_point);
    EC_POINT_free(shared_point);
    BN_free(priv_bn);

    if (len != 33) {
        qCritical() << "Unexpected compressed point length";
        return QByteArray();
    }

    // Use X coordinate (bytes 1..32) as shared key (like in Rust)
    return QByteArray(reinterpret_cast<const char *>(compressed + 1), 32);
}

/**
 * @brief WalletOwnerApi::postEncrypted
 * @param method
 * @param params
 */
QJsonObject WalletOwnerApi::postEncrypted(const QString &method, const QJsonObject &params)
{
    QByteArray nonce(12, 0);
    QByteArray json;
    QString encryptedBody;

    // random nonce
    RAND_bytes(reinterpret_cast<unsigned char *>(nonce.data()), nonce.size());

    QJsonObject payload{
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", method},
        {"params", params}
    };

    json = QJsonDocument(payload).toJson(QJsonDocument::Compact);

    // encrypt
    encryptedBody = encrypt(m_shareSecret, json, nonce);

    if (encryptedBody.isEmpty()) {
        qDebug() << "error no body";
        return QJsonObject();
    }

    QJsonObject encryptedParams{
        {"nonce", QString(nonce.toHex())},
        {"body_enc", encryptedBody}
    };

    return post("encrypted_request_v3", encryptedParams);
}

/**
 * @brief WalletOwnerApi::generateAuthHeader
 * @return
 */
QByteArray WalletOwnerApi::generateAuthHeader() const
{
    QString credentials = m_apiUser + ":" + m_apiPassword;
    QByteArray base64Creds = credentials.toUtf8().toBase64();
    return "Basic " + base64Creds;
}

/**
 * @brief WalletOwnerApi::initSendTx
 * @param args
 * @return
 */
QJsonObject WalletOwnerApi::initSendTx(const QJsonObject &args)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["args"] = args;

    return postEncrypted("init_send_tx", params);
}

/**
 * @brief WalletOwnerApi::issueInvoiceTx
 * @return
 */
QJsonObject WalletOwnerApi::issueInvoiceTx()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("issue_invoice_tx", params);
}

/**
 * @brief WalletOwnerApi::nodeHeight
 * @return
 */
QJsonObject WalletOwnerApi::nodeHeight()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("node_height", params);
}

/**
 * @brief WalletOwnerApi::finalizeTx
 * @param slate
 * @return
 */
QJsonObject WalletOwnerApi::finalizeTx(const QJsonObject slate)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate;

    return postEncrypted("finalize_tx", params);
}

/**
 * @brief WalletOwnerApi::getMnemonic
 * @return
 */
QJsonObject WalletOwnerApi::getMnemonic()
{
    QJsonObject args;
    return postEncrypted("get_mnemonic", args);
}

/**
 * @brief WalletOwnerApi::getRewindHash
 * @return
 */
QJsonObject WalletOwnerApi::getRewindHash()
{
    QJsonObject args;
    return postEncrypted("get_rewind_hash", args);
}

/**
 * @brief WalletOwnerApi::getSlatepackAddress
 * Retrieve the public slatepack address associated with the active account at the given derivation path.
 * In this case, an “address” means a Slatepack Address corresponding to a private key derived as follows:
 * e.g. The default parent account is at
 * derivation_index - The index along the derivation path to retrieve an address for
 * @return
 */
QString WalletOwnerApi::getSlatepackAddress()
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());
    args["derivation_index"] = 0;

    QJsonObject response = postEncrypted("get_slatepack_address", args);

    if (response.contains("result") && response["result"].toObject().contains("Ok")) {
        QString slatepackAddress = response["result"].toObject()["Ok"].toString();

        if (slatepackAddress.isEmpty()) {
            qWarning() << "slatepackAddress is empty!";
            return QString();
        } else {
            return slatepackAddress;
        }
    } else {
        qWarning() << "no slatepackAddress token!";
    }

    return QString();
}

/**
 * @brief WalletOwnerApi::getSlatepackSecretKey
 * @return
 */
QJsonObject WalletOwnerApi::getSlatepackSecretKey()
{
    QJsonObject args;
    return postEncrypted("get_slatepack_secret_key", args);
}

/**
 * @brief WalletOwnerApi::getStoredTx
 * @return
 */
QJsonObject WalletOwnerApi::getStoredTx(QString slateId, int id)
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());
    args["slate_id"] = slateId;
    args["id"] = QJsonValue::Null;

    qDebug()<<args;

    return postEncrypted("get_stored_tx", args);
}

/**
 * @brief WalletOwnerApi::getTopLevelDirectory
 * @return
 */
QJsonObject WalletOwnerApi::getTopLevelDirectory()
{
    QJsonObject args;
    return postEncrypted("get_top_level_directory", args);
}

/**
 * @brief WalletOwnerApi::getUpdaterMessages
 * @return
 */
QJsonObject WalletOwnerApi::getUpdaterMessages()
{
    QJsonObject args;
    return postEncrypted("get_updater_messages", args);
}

/**
 * @brief WalletOwnerApi::retrieveSummaryInfo
 * @param refreshFromNode
 * @param minimum_confirmations
 */
QJsonObject WalletOwnerApi::retrieveSummaryInfo(bool refreshFromNode, int minimum_confirmations)
{
    QJsonObject args;
    args["refresh_from_node"] = refreshFromNode;
    args["minimum_confirmations"] = minimum_confirmations;
    args["token"] = QString(m_openWalletToken.toHex());

    QJsonObject response = postEncrypted("retrieve_summary_info", args);
    QJsonArray okArray = response["result"].toObject()["Ok"].toArray();

    if(okArray.size() == 2)
    {
        QJsonObject summaryInfo = okArray[1].toObject();
        if (summaryInfo.isEmpty()) {
            qWarning() << "summaryInfo is empty!";
            return QJsonObject();
        } else {
            return summaryInfo;
        }
    }
    else
    {
        qWarning() << response;
        qWarning() << "no summaryInfo!";
    }
    return QJsonObject();
}

/**
 * @brief WalletOwnerApi::retrieveTxs
 * @return
 */
QJsonObject WalletOwnerApi::retrieveTxs()
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());
    args["refresh_from_node"]= true;
    args["tx_id"]= QJsonValue::Null;
    args["tx_slate_id"]= QJsonValue::Null;

    return postEncrypted("retrieve_txs", args);
}

/**
 * @brief WalletOwnerApi::scan
 * @return
 */
QJsonObject WalletOwnerApi::scan()
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("scan", args);
}

/**
 * @brief WalletOwnerApi::scanRewindHash
 * @return
 */
QJsonObject WalletOwnerApi::scanRewindHash()
{
    QJsonObject args;
    return postEncrypted("scan_rewind_hash", args);
}

/**
 * @brief WalletOwnerApi::setActiveAccount
 * @return
 */
QJsonObject WalletOwnerApi::setActiveAccount()
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("set_active_account", args);
}

/**
 * @brief WalletOwnerApi::setTopLevelDirectory
 * @return
 */
QJsonObject WalletOwnerApi::setTopLevelDirectory()
{
    QJsonObject args;

    return postEncrypted("set_top_level_directory", args);
}

/**
 * @brief WalletOwnerApi::setTorConfig
 * @return
 */
QJsonObject WalletOwnerApi::setTorConfig()
{
    QJsonObject args;

    return postEncrypted("set_tor_config", args);
}

/**
 * @brief WalletOwnerApi::slateFromSlatepackMessage
 * @return
 */
QJsonObject WalletOwnerApi::slateFromSlatepackMessage(QString message)
{
    QJsonObject args;
    QJsonArray si;
    si.append(0);
    args["token"] = QString(m_openWalletToken.toHex());
    args["secret_indices"] = si;
    args["message"] = message;

    QJsonObject response = postEncrypted("slate_from_slatepack_message", args);

    if (response.contains("result") && response["result"].toObject().contains("Ok")) {
        QJsonObject slate = response["result"].toObject()["Ok"].toObject();

        if (slate.isEmpty()) {
            qWarning() << "slate is empty!";
            return QJsonObject();
        } else {
            return slate;
        }
    } else {
        qWarning() << response;
        qWarning() << "no slate!";
    }

    return QJsonObject();
}

/**
 * @brief WalletOwnerApi::startUpdater
 * @return
 */
QJsonObject WalletOwnerApi::startUpdater()
{
    QJsonObject args;

    return postEncrypted("start_updater", args);
}

/**
 * @brief WalletOwnerApi::stopUpdater
 * @return
 */
QJsonObject WalletOwnerApi::stopUpdater()
{
    QJsonObject args;

    return postEncrypted("stop_updater", args);
}

/**
 * @brief WalletOwnerApi::httpSend
 * @param amountStr
 * @param url
 * @param ttl_blocks
 * @return
 */
QJsonObject WalletOwnerApi::httpSend(QString amount, QString url, QVariant ttlBlocks)
{
    QJsonObject txData;

    ///----------------------------------------------------------------------------------
    /// The API method to start a transaction is: init_send_tx. This initiates a new
    /// transaction as the sender, creating a new Slate object containing the sender's
    /// inputs, change outputs, and public signature data. When a transaction is created,
    /// the wallet must also lock inputs (and create unconfirmed outputs) corresponding
    /// to the transaction created in the slate. This is so the wallet doesn't attempt
    /// to re-spend outputs that are already included in a transaction before the
    /// transaction is confirmed.
    ///----------------------------------------------------------------------------------
    QJsonObject slate;
    QString tx_slate_id;

    if (!ttlBlocks.isValid()) {
        txData["ttl_blocks"] = QJsonValue::Null;
    } else {
        txData["ttl_blocks"] = QJsonValue::Null;
    }
    txData["src_acct_name"] = QJsonValue::Null;
    txData["amount"] = amount;
    txData["minimum_confirmations"] = 10;
    txData["max_outputs"] = 500;
    txData["num_change_outputs"] = 1;
    txData["selection_strategy_is_use_all"] = false;
    txData["target_slate_version"] = QJsonValue::Null;
    txData["payment_proof_recipient_address"] = QJsonValue::Null;
    txData["send_args"] = QJsonValue::Null;

    slate = initSendTx(txData);
    qDebug() << "repsonse initSendTx: " << slate;

    tx_slate_id = slate["id"].toString();
    qDebug() << "tx_slate_id: " << tx_slate_id;

    ///----------------------------------------------------------------------------------
    ///To create the Slatepack Message from the slate, we need to call
    /// create_slatepack_message and pass the slate like this:
    ///----------------------------------------------------------------------------------
    QString slatepack = createSlatepackMessage(slate, QJsonArray(), 0);
    qDebug() << "slatepack: " << slatepack;

    ///----------------------------------------------------------------------------------
    ///Next, we need to call the method: receive_tx from the Foreign API which receives
    /// a transaction created by another party, returning the modified Slate object,
    /// modified with the recipient's output for the transaction amount, and public
    /// signature data. This slate can then be sent back to the sender to finalize
    /// the transaction via the Owner API's finalize_tx method. This function creates
    /// a single output for the full amount and sets to a status of 'Awaiting finalization'.
    /// It will remain in this state until the wallet finds the corresponding output on the
    /// chain, at which point it will become 'Unspent'. The slate will be updated with the
    /// results of signing round 1 and 2, adding the recipient's public nonce, public
    /// excess value, and partial signature to the slate.
    /// Also creates a corresponding Transaction Log Entry in the wallet's transaction log.
    /// The positional parameters for this method are the next:
    /// slate - The transaction Slate. The slate should contain the results of the sender's
    /// round 1 (e.g, public nonce and public excess value).
    /// dest_acct_name - The name of the account into which the slate should be received.
    /// If None, the default account is used.
    /// r_addr - If included, attempts to send the slate back to the sender using
    /// the Slatepack sync send (TOR). If providing this argument, check the state field
    /// of the slate to see if the sync_send was successful (it should be S3 if the
    /// synced send sent successfully).
    ///----------------------------------------------------------------------------------
    WalletForeignApi walletForeignApi(url);
    QJsonObject slate2 = walletForeignApi.receiveTx(slate, "", "");

    QJsonObject txLockOutput = txLockOutputs(slate);

    QJsonObject finalized = finalizeTx(slate2);

    QJsonObject posted = postTx(finalized, false);

    return {
               {"tx_slate_id", tx_slate_id},
               {"finalized", finalized},
               {"posted", posted}
    };
}

/**
 * @brief WalletOwnerApi::cancelTx
 * @param id
 */
QJsonObject WalletOwnerApi::cancelTx(QString txSlateId, int id)
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());
    if(txSlateId.isEmpty())
    {
        args["tx_slate_id"] = QJsonValue::Null;
    }
    else
    {
        args["tx_slate_id"] = txSlateId;
    }
    args["tx_id"] = id;

    return postEncrypted("cancel_tx", args);
}

/**
 * @brief WalletOwnerApi::changePassword
 * @return
 */
QJsonObject WalletOwnerApi::changePassword()
{
    QJsonObject args;
    return postEncrypted("change_password", args);
}

/**
 * @brief WalletOwnerApi::closeWallet
 * @return
 */
QJsonObject WalletOwnerApi::closeWallet()
{
    QJsonObject args;
    return postEncrypted("close_wallet", args);
}

/**
 * @brief WalletOwnerApi::createAccountPath
 * @return
 */
QJsonObject WalletOwnerApi::createAccountPath()
{
    QJsonObject args;
    return postEncrypted("create_account_path", args);
}

QJsonObject WalletOwnerApi::createConfig()
{
    QJsonObject args;
    return postEncrypted("create_config", args);
}

/**
 * @brief WalletOwnerApi::postTx
 * @param finalize
 * @return
 */
QJsonObject WalletOwnerApi::postTx(QJsonObject slate, bool fluff)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate;
    params["fluff"] = fluff;

    return postEncrypted("post_tx", params);
}

/**
 * @brief WalletOwnerApi::processInvoiceTx
 * @return
 */
QJsonObject WalletOwnerApi::processInvoiceTx()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("process_invoice_tx", params);
}

/**
 * @brief WalletOwnerApi::queryTxs
 * @return
 */
QJsonObject WalletOwnerApi::queryTxs()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("query_txs", params);
}

/**
 * @brief WalletOwnerApi::retrieveOutputs
 * @return
 */
QJsonObject WalletOwnerApi::retrieveOutputs()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("retrieve_outputs", params);
}

/**
 * @brief WalletOwnerApi::retrievePaymentProof
 * @return
 */
QJsonObject WalletOwnerApi::retrievePaymentProof()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    return postEncrypted("retrieve_payment_proof", params);
}

/**
 * @brief WalletOwnerApi::txLockOutputs
 * @param slate
 * @return
 */
QJsonObject WalletOwnerApi::txLockOutputs(QJsonObject slate)
{
    QJsonObject args;
    args["token"] = QString(m_openWalletToken.toHex());
    args["slate"] = slate;
    return postEncrypted("tx_lock_outputs", args);
}

/**
 * @brief WalletOwnerApi::verifyPaymentProof
 * @return
 */
QJsonObject WalletOwnerApi::verifyPaymentProof()
{
    QJsonObject args;
    return postEncrypted("verify_payment_proof", args);
}

/**
 * @brief WalletApi::initSecure
 * Initializes the secure JSON-RPC API. This function must be called and a shared key
 * established before any other OwnerAPI JSON-RPC function can be called. The shared key will
 * be derived using ECDH with the provided public key on the secp256k1 curve.
 * This function will return its public key used in the derivation,
 * which the caller should multiply by its private key to derive the shared key.
 * Once the key is established, all further requests and responses are encrypted and
 * decrypted with the following parameters:
 * AES-256 in GCM mode with 128-bit tags and 96 bit nonces
 * 12 byte nonce which must be included in each request/response to use on the decrypting side
 * Empty vector for additional data
 * Suffix length = AES-256 GCM mode tag length = 16 bytes
 */
QJsonObject WalletOwnerApi::initSecureApi()
{
    QJsonObject args;
    args["ecdh_pubkey"] = QString(m_publicKey.toHex());
    qDebug() << "args" << args;

    return post("init_secure_api", args);
}

/**
 * @brief WalletOwnerApi::accounts
 * @return
 */
QJsonObject WalletOwnerApi::accounts()
{
    QJsonObject args;
    return postEncrypted("accounts", args);
}

/**
 * @brief WalletOwnerApi::buildOutputs
 * @return
 */
QJsonObject WalletOwnerApi::buildOutputs()
{
    QJsonObject args;
    return postEncrypted("build_output", args);
}

/**
 * @brief WalletOwnerApi::openWallet
 * @param name
 * @param password
 */
QJsonObject WalletOwnerApi::openWallet(QString name, QString password)
{
    Q_UNUSED(name);

    QJsonObject args;
    args["name"] = QJsonValue::Null;
    args["password"] = password;

    QJsonObject response = postEncrypted("open_wallet", args);

    if (response.contains("result") && response["result"].toObject().contains("Ok")) {
        QByteArray token;
        token = QByteArray::fromHex(response["result"].toObject()["Ok"].toString().toUtf8());
        if (token.isEmpty()) {
            qWarning() << "token is empty!";
        }
        m_openWalletToken = token;
    } else {
        qWarning() << "no open_wallet token!";
    }

    return response;
}

/**
 * @brief WalletOwnerApi::createSlatepackMessage
 * @param slate
 * @param recipients
 * @param senderIndex
 * @return
 */
QString WalletOwnerApi::createSlatepackMessage(QJsonObject slate, QJsonArray recipients, int senderIndex)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate;
    params["recipients"] = recipients;
    params["sender_index"] = senderIndex;

    qDebug()<<params;

    QJsonObject response = postEncrypted("create_slatepack_message", params);

    if (response.contains("result") && response["result"].toObject().contains("Ok")) {
        QString slatepack;
        slatepack = response["result"].toObject()["Ok"].toString();
        if (slatepack.isEmpty()) {
            qWarning() << "slatepack is empty!";
        }
        return slatepack;
    } else {
        qWarning() << "no slatepack";
    }

    return QString();
}

/**
 * @brief WalletOwnerApi::createWallet
 * @return
 */
QJsonObject WalletOwnerApi::createWallet()
{
    QJsonObject args;
    return postEncrypted("create_wallet", args);
}

/**
 * @brief WalletOwnerApi::decodeSlatepackMessage
 * @return
 */
QJsonObject WalletOwnerApi::decodeSlatepackMessage()
{
    QJsonObject args;
    return postEncrypted("decode_slatepack_message", args);
}

/**
 * @brief WalletOwnerApi::deleteWallet
 * @return
 */
QJsonObject WalletOwnerApi::deleteWallet()
{
    QJsonObject args;
    return postEncrypted("delete_wallet", args);
}

/**
 * @brief WalletOwnerApi::encrypt
 * @param key
 * @param msg
 * @param nonce
 * @return
 */
QString WalletOwnerApi::encrypt(const QByteArray &key, const QString &msg, const QByteArray &nonce)
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    QByteArray plaintext = msg.toUtf8();
    QByteArray ciphertext(plaintext.size() + 16, 0); // 16 bytes extra for GCM tag
    QByteArray tag(16, 0);

    int len = 0, ciphertext_len = 0;
    bool success = true;

    if (!ctx || !EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
        qWarning() << "EVP_EncryptInit_ex failed!";
        success = false;
    }

    if (success && !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr)) {
        qWarning() << "EVP_CIPHER_CTX_ctrl failed!";
        success = false;
    }

    if (success && !EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                       reinterpret_cast<const unsigned char *>(key.constData()),
                                       reinterpret_cast<const unsigned char *>(nonce.constData()))) {
        qWarning() << "EVP_EncryptInit_ex failed!";
        success = false;
    }

    if (success && !EVP_EncryptUpdate(ctx,
                                      reinterpret_cast<unsigned char *>(ciphertext.data()),
                                      &len,
                                      reinterpret_cast<const unsigned char *>(plaintext.constData()),
                                      plaintext.size())) {
        qWarning() << "EVP_EncryptUpdate failed!";
        success = false;
    }

    ciphertext_len = len;

    if (success && !EVP_EncryptFinal_ex(ctx,
                                        reinterpret_cast<unsigned char *>(ciphertext.data()) + len,
                                        &len)) {
        qWarning() << "EVP_EncryptFinal_ex failed!";
        success = false;
    }

    ciphertext_len += len;

    if (success && !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, tag.size(), tag.data())) {
        qWarning() << "EVP_CIPHER_CTX_ctrl failed!";
        success = false;
    }

    EVP_CIPHER_CTX_free(ctx);

    if (!success) {
        qWarning() << "Encryption failed!";
        return QString();
    }

    ciphertext.resize(ciphertext_len);
    ciphertext.append(tag);

    return ciphertext.toBase64();
}

/**
 * @brief WalletOwnerApi::decrypt
 * @param key
 * @param encryptedBase64
 * @param nonce
 * @return
 */
QString WalletOwnerApi::decrypt(const QByteArray &key, const QString &encryptedBase64, const QByteArray &nonce)
{
    QByteArray ciphertextWithTag = QByteArray::fromBase64(encryptedBase64.toUtf8());
    if (ciphertextWithTag.size() < 16) {
        qWarning() << "Ciphertext too short to contain GCM tag.";
        return QString();
    }

    QByteArray tag = ciphertextWithTag.right(16);
    QByteArray ciphertext = ciphertextWithTag.left(ciphertextWithTag.size() - 16);
    QByteArray plaintext(ciphertext.size(), 0);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, plaintext_len = 0;
    bool success = true;

    if (!ctx || !EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) {
        qWarning() << "EVP_DecryptInit_ex failed!";
        success = false;
    }

    if (success && !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, nonce.size(), nullptr)) {
        qWarning() << "EVP_CIPHER_CTX_ctrl failed!";
        success = false;
    }

    if (success && !EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                                       reinterpret_cast<const unsigned char *>(key.constData()),
                                       reinterpret_cast<const unsigned char *>(nonce.constData()))) {
        qWarning() << "EVP_DecryptInit_ex failed!";
        success = false;
    }

    if (success && !EVP_DecryptUpdate(ctx,
                                      reinterpret_cast<unsigned char *>(plaintext.data()),
                                      &len,
                                      reinterpret_cast<const unsigned char *>(ciphertext.constData()),
                                      ciphertext.size())) {
        qWarning() << "EVP_DecryptUpdate failed!";
        success = false;
    }

    plaintext_len = len;

    if (success && !EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, tag.size(), const_cast<char *>(tag.constData()))) {
        qWarning() << "EVP_CIPHER_CTX_ctrl failed!";
        success = false;
    }

    // Final decrypt
    if (success && EVP_DecryptFinal_ex(ctx,
                                       reinterpret_cast<unsigned char *>(plaintext.data()) + len,
                                       &len) <= 0) {
        success = false;  // Authentication failed
        qDebug() << "Authentication failed!";
    }

    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);

    if (!success) {
        qWarning() << "Decryption failed!";
        return QString();
    }

    plaintext.resize(plaintext_len);
    return QString::fromUtf8(plaintext);
}
