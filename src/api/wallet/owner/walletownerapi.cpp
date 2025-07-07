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
 * @return
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

/**
 * @brief WalletOwnerApi::initSendTx
 * Initiates a new transaction as the sender, creating a new Slate object containing the sender’s inputs, change outputs,
 * and public signature data. This slate can then be sent to the recipient to continue the transaction via the Foreign API’s receive_tx method.
 * When a transaction is created, the wallet must also lock inputs (and create unconfirmed outputs) corresponding to the transaction created in the slate,
 * so that the wallet doesn’t attempt to re-spend outputs that are already included in a transaction before the
 * transaction is confirmed. This method also returns a function that will perform that locking, and it is up to the
 * caller to decide the best time to call the lock function (via the tx_lock_outputs method). If the exchange method is
 * intended to be synchronous (such as via a direct http call,) then the lock call can wait until the response is confirmed.
 * If it is asynchronous, (such as via file transfer,) the lock call should happen immediately (before the file is sent to the recipient).
 * If the send_args InitTxSendArgs, of the args, field is Some, this function will attempt to send the slate
 * back to the sender using the slatepack sync send (TOR). If providing this argument, check the state field of the slate to see
 * if the sync_send was successful (it should be S2 if the sync sent successfully). It will also post the transction if the post_tx field is set.
 * @param args
 * @return
 */
Result<Slate> WalletOwnerApi::initSendTx(InitTxArgs args)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["args"] = args.toJson();

    auto res = JsonUtil::extractOkObject(postEncrypted("init_send_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::issueInvoiceTx
 * Issues a new invoice transaction slate, essentially a request for payment.
 * The slate created by this function will contain the amount, an output for the amount,
 * as well as round 1 of singature creation complete. The slate should then be send to the payer,
 * who should add their inputs and signature data and return the slate via the Foreign API’s finalize_tx method.
 * @return
 */
Result<Slate> WalletOwnerApi::issueInvoiceTx(QString amount, QString destAcctName, QString targetSlateVersion)
{
    QJsonObject params;
    QJsonObject args;

    args["amount"] = amount;
    args["dest_acct_name"] = destAcctName.isEmpty() ? QJsonValue::Null : QJsonValue(destAcctName);
    args["target_slate_version"] = targetSlateVersion.isEmpty() ? QJsonValue::Null : QJsonValue(targetSlateVersion);

    params["token"] = QString(m_openWalletToken.toHex());
    params["args"] = args;

    auto res = JsonUtil::extractOkObject(postEncrypted("issue_invoice_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::nodeHeight
 * Retrieves the last known height known by the wallet. This is determined as follows:
 * If the wallet can successfully contact its configured node, the reported node height is returned,
 * and the updated_from_node field in the response is true
 * If the wallet cannot contact the node, this function returns the maximum height of all outputs contained within the wallet,
 * and the updated_from_node fields in the response is set to false.
 * Clients should generally ensure the updated_from_node field is returned as true before assuming the height for any operation.
 * @return
 */
Result<NodeHeight> WalletOwnerApi::nodeHeight()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    auto res = JsonUtil::extractOkObject(postEncrypted("node_height", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    NodeHeight nh;
    nh.fromJson(okObj);

    return nh;
}

/**
 * @brief WalletOwnerApi::finalizeTx
 * Finalizes a transaction, after all parties have filled in both rounds of Slate generation.
 * This step adds all participants partial signatures to create the final signature, resulting in a final transaction that is ready to post to a node.
 * Note that this function DOES NOT POST the transaction to a node for validation. This is done in separately via the post_tx function.
 * This function also stores the final transaction in the user’s wallet files for retrieval via the get_stored_tx function.
 * @param slate
 * @return
 */
Result<Slate> WalletOwnerApi::finalizeTx(const Slate slate)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate.toJson();

    auto res = JsonUtil::extractOkObject(postEncrypted("finalize_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::getMnemonic
 * Return the BIP39 mnemonic for the given wallet. This function will decrypt the wallet’s seed
 * file with the given password, and thus does not need the wallet to be open.
 * @return
 */
Result<QString> WalletOwnerApi::getMnemonic(QString name, QString password)
{
    QJsonObject params;
    params["name"] = name.isEmpty() ? QJsonValue::Null : QJsonValue(name);
    params["password"] = password;

    auto res = JsonUtil::extractOkValue(postEncrypted("get_mnemonic", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // if ok
    if (OkVal.isString()) {
        return OkVal.toString();
    }

    return Error(ErrorType::Unknown, QString("Error get_mnemonic string!"));
}

/**
 * @brief WalletOwnerApi::getRewindHash
 * @return Return the rewind hash of the wallet. The rewind hash when shared,
 * help third-party to retrieve informations (outputs, balance, …) that belongs to this wallet.
 */
Result<RewindHash> WalletOwnerApi::getRewindHash()
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());

    auto res = JsonUtil::extractOkValue(postEncrypted("get_rewind_hash", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    return RewindHash(OkVal.toString());
}

/**
 * @brief WalletOwnerApi::getSlatepackAddress
 * Retrieve the public slatepack address associated with the active account at the given derivation path.
 * In this case, an “address” means a Slatepack Address corresponding to a private key derived as follows:
 * e.g. The default parent account is at
 * m/0/0
 * With output blinding factors created as
 * m/0/0/0 m/0/0/1 etc…
 * The corresponding public address derivation path would be at:
 * m/0/1
 * With addresses created as:
 * m/0/1/0 m/0/1/1 etc…
 * Note that these addresses correspond to the public keys used in the addresses of TOR hidden services
 * configured by the wallet listener.
 * @return
 */
Result<QString> WalletOwnerApi::getSlatepackAddress(int derivationIndex)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["derivation_index"] = derivationIndex;

    auto res = JsonUtil::extractOkValue(postEncrypted("get_slatepack_address", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    return OkVal.toString();
}

/**
 * @brief WalletOwnerApi::getSlatepackSecretKey
 * Retrieve the private ed25519 slatepack key at the given derivation index. Currently used to decrypt encrypted slatepack messages.
 * @return
 */
Result<QString> WalletOwnerApi::getSlatepackSecretKey(int derivationIndex)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["derivation_index"] = derivationIndex;

    auto res = JsonUtil::extractOkValue(postEncrypted("get_slatepack_secret_key", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    return OkVal.toString();
}

/**
 * @brief WalletOwnerApi::getStoredTx
 * Retrieves the stored transaction associated with a TxLogEntry.
 * Can be used even after the transaction has completed.
 * Either the Transaction Log ID or the Slate UUID must be supplied.
 * If both are supplied, the Transaction Log ID is preferred.
 * @return
 */
Result<Slate> WalletOwnerApi::getStoredTx(QString slateId, int id)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate_id"] = slateId;
    if (id == 0) {
        params["id"] = QJsonValue::Null;
    } else {
        params["id"] = id;
    }

    auto res = JsonUtil::extractOkObject(postEncrypted("get_stored_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::getTopLevelDirectory
 * Retrieve the top-level directory for the wallet.
 * This directory should contain the grin-wallet.toml file and the wallet_data directory that contains the wallet seed + data files.
 * Future versions of the wallet API will support multiple wallets within the top level directory.
 * The top level directory defaults to (in order of precedence):
 * The current directory, from which grin-wallet or the main process was run, if it contains a grin-wallet.toml file.
 * ~/.grin// otherwise
 * @return
 */
Result<QString> WalletOwnerApi::getTopLevelDirectory()
{
    QJsonObject params;

    auto res = JsonUtil::extractOkValue(postEncrypted("get_top_level_directory", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    return OkVal.toString();
}

/**
 * @brief WalletOwnerApi::getUpdaterMessages
 * Retrieve messages from the updater thread, up to count number of messages.
 * The resulting array will be ordered newest messages first. The updater will store a maximum of 10,000 messages,
 * after which it will start removing the oldest messages as newer ones are created.
 * Messages retrieved via this method are removed from the internal queue, so calling this function at a specified
 * interval should result in a complete message history.
 * @return
 */
Result<StatusMessage> WalletOwnerApi::getUpdaterMessages(QString &message, quint8 &progress)
{
    QJsonObject params;

    message.clear();
    progress = 0;

    auto res = JsonUtil::extractOkObject(postEncrypted("get_updater_messages", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    QString key = okObj.keys().first();
    QJsonValue value = okObj.value(key);

    if (key == "UpdatingOutputs") {
        return StatusMessage::StatusMessageUpdatingOutputs;
    }
    if (key == "UpdatingTransactions") {
        return StatusMessage::StatusMessageUpdatingTransactions;
    }
    if (key == "FullScanWarn") {
        return StatusMessage::StatusMessageFullScanWarn;
    }
    if (key == "Scanning") {
        if (value.isArray()) {
            QJsonArray arr = value.toArray();
            if (arr.size() == 2 && arr[0].isString() && arr[1].isDouble()) {
                message = arr[0].toString();
                progress = static_cast<quint8>(arr[1].toInt());
            }
        }
        return StatusMessage::StatusMessageScanning;
    }
    if (key == "ScanningComplete") {
        return StatusMessage::StatusMessageScanningComplete;
    }
    if (key == "UpdateWarning") {
        return StatusMessage::StatusMessageUpdateWarning;
    }

    return StatusMessage::StatusMessageUnknown;
}

/**
 * @brief WalletOwnerApi::retrieveSummaryInfo
 * Returns summary information from the active account in the wallet.
 * @param refreshFromNode
 * @param minimum_confirmations
 */
Result<WalletInfo> WalletOwnerApi::retrieveSummaryInfo(bool refreshFromNode, int minimum_confirmations)
{
    QJsonObject params;
    params["refresh_from_node"] = refreshFromNode;
    params["minimum_confirmations"] = minimum_confirmations;
    params["token"] = QString(m_openWalletToken.toHex());

    auto res = JsonUtil::extractOkValue(postEncrypted("retrieve_summary_info", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // Array: [true, {...}]
    QJsonArray okArray = OkVal.toArray();

    if (okArray.size() < 2) {
        return Error(ErrorType::Unknown, QString("retrieve_summary_info : okArray.size() < 2"));
    }

    QJsonObject walletInfoJson = okArray.at(1).toObject();

    WalletInfo walletInfo;
    walletInfo.fromJson(walletInfoJson);

    return walletInfo;
}

/**
 * @brief WalletOwnerApi::retrieveTxs
 * Returns a list of Transaction Log Entries from the active account in the wallet.
 * @return
 */
Result<QList<TxLogEntry> > WalletOwnerApi::retrieveTxs(bool refreshFromNode, int txId, QString txSlateId)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["refresh_from_node"] = refreshFromNode;
    params["tx_id"] = (txId == 0) ? QJsonValue(QJsonValue::Null) : QJsonValue(txId);
    params["tx_slate_id"] = (txSlateId == 0) ? QJsonValue(QJsonValue::Null) : QJsonValue(txSlateId);

    auto res = JsonUtil::extractOkValue(postEncrypted("retrieve_txs", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();

    // [true, [Array TxLogEntry objs]]
    if (okArray.size() < 2 || !okArray[1].isArray()) {
        return Error(ErrorType::Unknown, QString("retrieve_summary_info : okArray.size() < 2 || !okArray[1].isArray()"));
    }

    QJsonArray txArray = okArray[1].toArray();
    QList<TxLogEntry> entries;

    for (const QJsonValue &val : txArray) {
        if (val.isObject()) {
            TxLogEntry entry;
            entry.fromJson(val.toObject());
            entries.append(entry);
        }
    }

    return entries;
}

/**
 * @brief WalletOwnerApi::scan
 * Scans the entire UTXO set from the node, identify which outputs belong to the given wallet update the wallet state to be consistent
 * with what’s currently in the UTXO set.
 * This function can be used to repair wallet state, particularly by restoring outputs that may be missing if
 * the wallet owner has cancelled transactions locally that were then successfully posted to the chain.
 * This operation scans the entire chain, and is expected to be time intensive. It is imperative that no other processes should be trying
 * to use the wallet at the same time this function is running.
 * When an output is found that doesn’t exist in the wallet, a corresponding TxLogEntry is created.
 * @return
 */
Result<bool> WalletOwnerApi::scan(int startHeight, bool deleteUnconfirmed)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["start_height"] = startHeight;
    params["delete_unconfirmed"] = deleteUnconfirmed;

    auto res = JsonUtil::extractOkValue(postEncrypted("scan", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::scanRewindHash
 * Scans the entire UTXO set from the node, identify which outputs belong to the given rewind hash view wallet.
 * This function can be used to retrieve outputs informations (outputs, balance, …) from a rewind hash view wallet.
 * This operation scans the entire chain, and is expected to be time intensive. It is imperative that no other processes
 * should be trying to use the wallet at the same time this function is running.
 * @return
 */
Result<ViewWallet> WalletOwnerApi::scanRewindHash(RewindHash rewindHash, int startHeight)
{
    QJsonObject params;
    params["rewind_hash"] = rewindHash.rewindHash();
    params["start_height"] = startHeight;

    auto res = JsonUtil::extractOkObject(postEncrypted("scan_rewind_hash", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return ViewWallet::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::setActiveAccount
 * Sets the wallet’s currently active account. This sets the BIP32 parent path used for most key-derivation operations.
 * @return
 */
Result<bool> WalletOwnerApi::setActiveAccount(QString label)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["label"] = label;

    auto res = JsonUtil::extractOkValue(postEncrypted("set_active_account", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::setTopLevelDirectory
 * Set the top-level directory for the wallet. This directory can be empty, and will be created during a subsequent calls to create_config
 * Set get_top_level_directory for a description of the top level directory and default paths.
 * @return
 */
Result<bool> WalletOwnerApi::setTopLevelDirectory(QString dir)
{
    QJsonObject params;
    params["dir"] = dir;

    auto res = JsonUtil::extractOkValue(postEncrypted("set_top_level_directory", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::setTorConfig
 * Set the TOR configuration for this instance of the OwnerAPI, used during init_send_tx when send args are present and a TOR address is specified
 * @return
 */
Result<bool> WalletOwnerApi::setTorConfig(TorConfig torConfig)
{
    // Nest "tor_config" inside a "params" object
    QJsonObject params;
    params["tor_config"] = torConfig.toJson();

    auto res = JsonUtil::extractOkValue(postEncrypted("set_tor_config", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::slateFromSlatepackMessage
 * Extract the slate from the given slatepack. If the slatepack payload is encrypted, attempting to decrypt with keys at the given
 * address derivation path indices.
 * @return
 */
Result<Slate> WalletOwnerApi::slateFromSlatepackMessage(QString message, QJsonArray secretIndices)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["secret_indices"] = secretIndices;
    params["message"] = message;

    auto res = JsonUtil::extractOkObject(postEncrypted("slate_from_slatepack_message", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::startUpdater
 * Starts a background wallet update thread, which performs the wallet update process automatically at the frequency specified.
 * The updater process is as follows:
 * Reconcile the wallet outputs against the node’s current UTXO set, confirming transactions if needs be.
 * Look up transactions by kernel in cases where it’s necessary (for instance, when there are no change outputs for a
 * transaction and transaction status can’t be inferred from the output state.
 * Incrementally perform a scan of the UTXO set, correcting outputs and transactions where their local
 * state differs from what’s on-chain. The wallet stores the last position scanned, and will scan back 100 blocks worth of UTXOs on each update,
 * to correct any differences due to forks or otherwise.
 * Note that an update process can take a long time, particularly when the entire UTXO set is being scanned for correctness.
 * The wallet status can be determined by calling the get_updater_messages.
 * @return
 */
Result<bool> WalletOwnerApi::startUpdater(int frequency)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["frequency"] = frequency;

    auto res = JsonUtil::extractOkValue(postEncrypted("start_updater", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::stopUpdater
 * Stops the background update thread. If the updater is currently updating, the thread will stop after the next update
 * @return
 */
Result<bool> WalletOwnerApi::stopUpdater()
{
    QJsonObject params;

    auto res = JsonUtil::extractOkValue(postEncrypted("stop_updater", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::cancelTx
 * Cancels a transaction. This entails:
 * Setting the transaction status to either TxSentCancelled or TxReceivedCancelled
 * Deleting all change outputs or recipient outputs associated with the transaction
 * Setting the status of all assocatied inputs from Locked to Spent so they can be used in new transactions.
 * Transactions can be cancelled by transaction log id or slate id (call with either set to Some, not both)
 * @param id
 */
Result<bool> WalletOwnerApi::cancelTx(QString txSlateId, int id)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["tx_slate_id"] = txSlateId.isEmpty() ? QJsonValue() : QJsonValue(txSlateId);
    params["tx_id"] = id;

    auto res = JsonUtil::extractOkValue(postEncrypted("cancel_tx", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::changePassword
 * Changes a wallet’s password, meaning the old seed file is decrypted with the old password,
 * and a new seed file is created with the same mnemonic and encrypted with the new password.
 * This function temporarily backs up the old seed file until a test-decryption of the new file is
 * confirmed to contain the same seed as the original seed file, at which point the backup is deleted.
 * If this operation fails for an unknown reason, the backup file will still exist in the wallet’s data
 * directory encrypted with the old password.
 * @return
 */
Result<bool> WalletOwnerApi::changePassword(QString name, QString oldPw, QString newPw)
{
    QJsonObject params;
    params["name"] = name.isEmpty() ? QJsonValue::Null : QJsonValue(name);
    params["old"] = oldPw;
    params["new"] = newPw;

    auto res = JsonUtil::extractOkValue(postEncrypted("change_password", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::closeWallet
 * Close a wallet, removing the master seed from memory.
 * @return
 */
Result<bool> WalletOwnerApi::closeWallet(QString name)
{
    QJsonObject params;
    params["name"] = name.isEmpty() ? QJsonValue::Null : QJsonValue(name);

    auto res = JsonUtil::extractOkValue(postEncrypted("close_wallet", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // clear wallet token
    m_openWalletToken.clear();

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::createAccountPath
 * Creates a new ‘account’, which is a mapping of a user-specified label to a BIP32 path
 * @return
 */
Result<QString> WalletOwnerApi::createAccountPath(QString label)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["label"] = label;

    auto res = JsonUtil::extractOkValue(postEncrypted("create_account_path", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // if ok
    if (OkVal.isString()) {
        return OkVal.toString();
    }

    return Error(ErrorType::Unknown, QString("error: create_account_path"));
}

/**
 * @brief WalletOwnerApi::createConfig
 * Create a grin-wallet.toml configuration file in the top-level directory for the specified chain type.
 * A custom WalletConfig and/or grin LoggingConfig may optionally be provided, otherwise defaults will be used.
 * Paths in the configuration file will be updated to reflect the top level directory,
 * so path-related values in the optional configuration structs will be ignored.
 * @return
 */
Result<bool> WalletOwnerApi::createConfig(Config config)
{
    QJsonObject params;
    params["chain_type"] = config.chainType();
    params["wallet_config"] = config.walletConfig().toJson();
    params["logging_config"] = config.loggingConfig().toJson();
    params["tor_config"] = config.torConfig().toJson();

    QJsonObject rpcJson = postEncrypted("create_config", params);

    // Check if "result" exists and is an object
    if (!rpcJson.contains("result") || !rpcJson["result"].isObject()) {
        return false;
    }

    QJsonObject resultObj = rpcJson["result"].toObject();

    // The "Ok" key must exist — its value can be either null or an object
    return resultObj.contains("Ok");
}

/**
 * @brief WalletOwnerApi::postTx
 * Posts a completed transaction to the listening node for validation and inclusion in a block for mining.
 * @param
 * @return
 */
Result<bool> WalletOwnerApi::postTx(Slate slate, bool fluff)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate.toJson();
    params["fluff"] = fluff;

    auto res = JsonUtil::extractOkValue(postEncrypted("post_tx", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::processInvoiceTx
 * Processes an invoice tranaction created by another party, essentially a request for payment.
 * The incoming slate should contain a requested amount, an output created by the invoicer convering the amount,
 * and part 1 of signature creation completed. This function will add inputs equalling the amount + fees,
 * as well as perform round 1 and 2 of signature creation.
 * Callers should note that no prompting of the user will be done by this function it is up to the caller to present
 * the request for payment to the user and verify that payment should go ahead.
 * If the send_args InitTxSendArgs, of the args, field is Some, this function will attempt
 * to send the slate back to the initiator using the slatepack sync send (TOR). If providing this argument,
 * check the state field of the slate to see if the sync_send was successful (it should be I3 if the sync sent successfully).
 * This function also stores the final transaction in the user’s wallet files for retrieval via the get_stored_tx function.
 * @return
 */
Result<Slate> WalletOwnerApi::processInvoiceTx(Slate slate, QJsonObject args)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate.toJson();
    params["args"] = args;

    auto res = JsonUtil::extractOkObject(postEncrypted("process_invoice_tx", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return Slate::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::queryTxs
 * Networked version of Owner::retrieve_txs, which passes only the tx_query_args parameter.
 */
Result<QList<TxLogEntry> > WalletOwnerApi::queryTxs(bool refreshFromNode, Query query)
{
    QJsonObject params;

    params["token"] = QString(m_openWalletToken.toHex());
    params["refresh_from_node"] = refreshFromNode;
    params["query"] = query.toJson();

    auto res = JsonUtil::extractOkValue(postEncrypted("query_txs", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();

    // [true, [Array TxLogEntry objs]]
    if (okArray.size() < 2 || !okArray[1].isArray()) {
        return Error(ErrorType::Unknown, QString("error query_txs : okArray.size() < 2 || !okArray[1].isArray()"));
    }

    QJsonArray txArray = okArray[1].toArray();
    QList<TxLogEntry> entries;
    for (const QJsonValue &val : txArray) {
        if (val.isObject()) {
            TxLogEntry entry;
            entry.fromJson(val.toObject());
            entries.append(entry);
        }
    }

    return entries;
}

/**
 * @brief WalletOwnerApi::retrieveOutputs
 * Returns a list of outputs from the active account in the wallet.
 * @return
 */
Result<QList<OutputCommitMapping> > WalletOwnerApi::retrieveOutputs(bool includeSpent, bool refreshFromNode, int txId)
{
    QJsonObject params;

    params["token"] = QString(m_openWalletToken.toHex());
    params["include_spent"] = includeSpent;
    params["refresh_from_node"] = refreshFromNode;
    params["tx_id"] = (txId == 0) ? QJsonValue(QJsonValue::Null) : QJsonValue(txId);

    auto res = JsonUtil::extractOkValue(postEncrypted("retrieve_outputs", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();

    // [ true, [ {...}, {...} ] ]
    if (okArray.size() != 2 || !okArray[1].isArray()) {
        return Error(ErrorType::Unknown, QString("error retrieve_outputs : okArray.size() != 2 || !okArray[1].isArray()"));
    }

    QJsonArray itemsArray = okArray[1].toArray();
    QList<OutputCommitMapping> list;
    for (const QJsonValue &val : itemsArray) {
        if (!val.isObject()) {
            continue;
        }

        QJsonObject itemObj = val.toObject();
        OutputCommitMapping mapping;
        mapping.fromJson(itemObj);
        list.append(mapping);
    }

    return list;
}

/**
 * @brief WalletOwnerApi::retrievePaymentProof
 * Returns a single, exportable PaymentProof from a completed transaction within the wallet.
 * The transaction must have been created with a payment proof, and the transaction must be complete in order for a payment proof to be returned.
 * Either the tx_id or tx_slate_id argument must be provided, or the function will return an error.
 * @return
 */
Result<PaymentProof> WalletOwnerApi::retrievePaymentProof(bool refreshFromNode, int txId, QString txSlateId)
{
    QJsonObject params;

    params["token"] = QString(m_openWalletToken.toHex());
    params["refresh_from_node"] = refreshFromNode;
    params["tx_id"] = (txId == 0) ? QJsonValue(QJsonValue::Null) : QJsonValue(txId);
    params["tx_slate_id"] = (txSlateId == 0) ? QJsonValue(QJsonValue::Null) : QJsonValue(txSlateId);

    auto res = JsonUtil::extractOkObject(postEncrypted("retrieve_payment_proof", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    PaymentProof proof;
    proof.fromJson(okObj);
    return proof;
}

/**
 * @brief WalletOwnerApi::txLockOutputs
 * Locks the outputs associated with the inputs to the transaction in the given Slate, making them unavailable for use in further transactions.
 * This function is called by the sender, (or more generally, all parties who have put inputs into the transaction,)
 * and must be called before the corresponding call to finalize_tx that completes the transaction.
 * Outputs will generally remain locked until they are removed from the chain, at which point they will become Spent.
 * It is commonplace for transactions not to complete for various reasons over which a particular wallet has no control.
 * For this reason, cancel_tx can be used to manually unlock outputs and return them to the Unspent state.
 * @param slate
 * @return
 */
Result<bool> WalletOwnerApi::txLockOutputs(Slate slate)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate.toJson();

    auto res = JsonUtil::extractOkValue(postEncrypted("tx_lock_outputs", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::verifyPaymentProof
 * Finalizes a transaction, after all parties have filled in both rounds of Slate generation. This step adds all participants partial
 * signatures to create the final signature, resulting in a final transaction that is ready to post to a node.
 * Note that this function DOES NOT POST the transaction to a node for validation. This is done in separately via the post_tx function.
 * This function also stores the final transaction in the user’s wallet files for retrieval via the get_stored_tx function.
 * @return
 */
Result<VerifyPaymentProofStatus> WalletOwnerApi::verifyPaymentProof(PaymentProof proof)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["proof"] = proof.toJson();

    auto res = JsonUtil::extractOkValue(postEncrypted("verify_payment_proof", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray okArray = OkVal.toArray();
    VerifyPaymentProofStatus status;
    if (okArray.size() >= 2) {
        bool senderBelongs = okArray[0].toBool(false);
        bool recipientBelongs = okArray[1].toBool(false);
        status.setSenderBelongsToWallet(senderBelongs);
        status.setRecipientBelongsToWallet(recipientBelongs);
        return status;
    }

    return Error(ErrorType::Unknown, QString("error verify_payment_proof size okArray"));
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
Result<QJsonObject> WalletOwnerApi::initSecureApi()
{
    QJsonObject args;
    args["ecdh_pubkey"] = QString(m_publicKey.toHex());
    qDebug() << "args" << args;

    return post("init_secure_api", args);
}

/**
 * @brief WalletOwnerApi::accounts
 * Returns a list of accounts stored in the wallet (i.e. mappings between user-specified labels and BIP32 derivation paths.
 * @return
 */
Result<QList<Account> > WalletOwnerApi::accounts()
{
    QJsonObject params;
    QList<Account> accounts;

    params["token"] = QString(m_openWalletToken.toHex());

    auto res = JsonUtil::extractOkValue(postEncrypted("accounts", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QJsonArray accountsArray = OkVal.toArray();

    for (const QJsonValue &val : accountsArray) {
        if (val.isObject()) {
            Account account = Account::fromJson(val.toObject());
            accounts.append(account);
        }
    }

    return accounts;
}

/**
 * @brief WalletOwnerApi::buildOutputs
 * Builds an output
 * @return
 */
Result<BuiltOutput> WalletOwnerApi::buildOutputs(QString features, QString amount)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["features"] = features;
    params["amount"] = amount;

    auto res = JsonUtil::extractOkObject(postEncrypted("build_output", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    return BuiltOutput::fromJson(okObj);
}

/**
 * @brief WalletOwnerApi::openWallet
 * Opens a wallet, populating the internal keychain with the encrypted seed, and optionally returning a keychain_mask token to the
 * caller to provide in all future calls. If using a mask, the seed will be stored in-memory XORed against
 * the keychain_mask, and will not be useable if the mask is not provided.
 * @param name
 * @param password
 */
Result<QString> WalletOwnerApi::openWallet(QString name, QString password)
{
    Q_UNUSED(name);

    QJsonObject params;
    params["name"] = (name.isEmpty()) ? QJsonValue(QJsonValue::Null) : QJsonValue(name);
    params["password"] = password;

    auto res = JsonUtil::extractOkValue(postEncrypted("open_wallet", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    QByteArray token;
    token = QByteArray::fromHex(OkVal.toString().toUtf8());
    if (token.isEmpty()) {
        return Error(ErrorType::Unknown, QString("error open_wallet no token!"));
    }
    m_openWalletToken = token;
    return QString(token);
}

/**
 * @brief WalletOwnerApi::createSlatepackMessage
 * Create a slatepack from a given slate, optionally encoding the slate with the provided recipient public keys
 * @param slate
 * @param recipients
 * @param senderIndex
 * @return
 */
Result<QString> WalletOwnerApi::createSlatepackMessage(Slate slate, QJsonArray recipients, int senderIndex)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["slate"] = slate.toJson();
    params["recipients"] = recipients;
    params["sender_index"] = senderIndex;

    auto res = JsonUtil::extractOkValue(postEncrypted("create_slatepack_message", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // if ok
    if (OkVal.isString()) {
        return OkVal.toString();
    }

    return Error(ErrorType::Unknown, QString("error create_slatepack_message no slatepack!"));
}

/**
 * @brief WalletOwnerApi::createWallet
 * Creates a new wallet seed and empty wallet database in the wallet_data directory of the top level directory.
 * Paths in the configuration file will be updated to reflect the top level directory, so path-related values in
 * the optional configuration structs will be ignored.
 * The wallet files must not already exist, and ~The grin-wallet.toml file must
 * exist in the top level directory (can be created via a call to create_config)
 * @return
 */
Result<bool> WalletOwnerApi::createWallet(QString name, QString mnemonic, int mnemonicLength, QString password)
{
    QJsonObject params;
    params["name"] = name.isEmpty() ? QJsonValue::Null : QJsonValue(name);
    params["mnemonic"] = mnemonic.isEmpty() ? QJsonValue::Null : QJsonValue(mnemonic);
    params["mnemonic_length"] = mnemonicLength;
    params["password"] = password;

    auto res = JsonUtil::extractOkValue(postEncrypted("create_wallet", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}

/**
 * @brief WalletOwnerApi::decodeSlatepackMessage
 * Decode an armored slatepack, returning a Slatepack object that can be viewed, manipulated, output as json, etc.
 * The resulting slatepack will be decrypted by this wallet if possible
 * @return
 */
Result<Slatepack> WalletOwnerApi::decodeSlatepackMessage(QJsonArray secretIndicesArray, QString message)
{
    QJsonObject params;
    params["token"] = QString(m_openWalletToken.toHex());
    params["secret_indices"] = secretIndicesArray;
    params["message"] = message.isEmpty() ? QJsonValue::Null : QJsonValue(message);

    auto res = JsonUtil::extractOkObject(postEncrypted("decode_slatepack_message", params));
    QJsonObject okObj;

    if (!res.unwrapOrLog(okObj)) {
        return res.error();
    }

    Slatepack sp;
    sp.fromJson(okObj);
    return sp;
}

/**
 * @brief WalletOwnerApi::deleteWallet
 * Deletes a wallet, removing the config file, seed file and all data files.
 * Obviously, use with extreme caution and plenty of user warning
 * Highly recommended that the wallet be explicitly closed first via the close_wallet function.
 * @return
 */
Result<bool> WalletOwnerApi::deleteWallet(QString name)
{
    QJsonObject params;
    params["name"] = name.isEmpty() ? QJsonValue::Null : QJsonValue(name);

    auto res = JsonUtil::extractOkValue(postEncrypted("delete_wallet", params));
    QJsonValue OkVal;

    if (!res.unwrapOrLog(OkVal)) {
        return res.error();
    }

    // The "Ok" key must exist — its value can be either null or an object
    return OkVal.isNull() || OkVal.isObject();
}
