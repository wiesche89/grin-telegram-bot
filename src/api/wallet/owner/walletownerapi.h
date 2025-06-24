#ifndef WALLETOWNERAPI_H
#define WALLETOWNERAPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QByteArray>
#include <QUrl>
#include <3rdParty/secp256k1/include/secp256k1.h>
#include <3rdParty/secp256k1/include/secp256k1_ecdh.h>
#include <openssl/rand.h>
#include <openssl/ecdh.h>

#include "walletforeignapi.h"

// https://docs.rs/grin_wallet_api/5.3.3/grin_wallet_api/trait.OwnerRpc.html
class WalletOwnerApi : public QObject
{
    Q_OBJECT

public:
    WalletOwnerApi(const QString &apiUrl, const QString &apiUser, const QString &apiPassword, QObject *parent = nullptr);

    bool hasConnection() const;

    QJsonObject accounts();
    QJsonObject buildOutputs();
    QJsonObject cancelTx(QString txSlateId, int id);
    QJsonObject changePassword();
    QJsonObject closeWallet();
    QJsonObject createAccountPath();
    QJsonObject createConfig();
    QString createSlatepackMessage(QJsonObject slate, QJsonArray recipients, int senderIndex);
    QJsonObject createWallet();
    QJsonObject decodeSlatepackMessage();
    QJsonObject deleteWallet();
    QJsonObject finalizeTx(const QJsonObject slate);
    QJsonObject getMnemonic();
    QJsonObject getRewindHash();
    QString getSlatepackAddress();
    QJsonObject getSlatepackSecretKey();
    QJsonObject getStoredTx(QString slateId, int id);
    QJsonObject getTopLevelDirectory();
    QJsonObject getUpdaterMessages();
    QJsonObject initSecureApi();
    QJsonObject initSendTx(const QJsonObject &args);
    QJsonObject issueInvoiceTx();
    QJsonObject nodeHeight();
    QJsonObject openWallet(QString name, QString password);
    QJsonObject postTx(QJsonObject slate, bool fluff = false);
    QJsonObject processInvoiceTx(QJsonObject slate, QJsonObject args);
    QJsonObject queryTxs();
    QJsonObject retrieveOutputs();
    QJsonObject retrievePaymentProof();
    QJsonObject retrieveSummaryInfo(bool refreshFromNode, int minimum_confirmations);
    QJsonArray retrieveTxs();
    QJsonObject scan();
    QJsonObject scanRewindHash();
    QJsonObject setActiveAccount();
    QJsonObject setTopLevelDirectory();
    QJsonObject setTorConfig();
    QJsonObject slateFromSlatepackMessage(QString message);
    QJsonObject startUpdater();
    QJsonObject stopUpdater();
    QJsonObject txLockOutputs(QJsonObject slate);
    QJsonObject verifyPaymentProof();

    QJsonObject httpSend(QString amount, QString url, QVariant ttlBlocks = QVariant());

private:
    QJsonObject post(const QString &method, const QJsonObject &params);
    QJsonObject postEncrypted(const QString &method, const QJsonObject &params);

    QString encrypt(const QByteArray &key, const QString &msg, const QByteArray &nonce);
    QString decrypt(const QByteArray &key, const QString &cipher_b64, const QByteArray &nonce);

    QByteArray deriveEcdhKeyOpenSSL(const QByteArray &secKeyHex, const QByteArray &otherPubKeyCompressed);
    QByteArray generateAuthHeader() const;
    void generateKeyPair();

    QString m_apiUrl;
    QString m_apiUser;
    QString m_apiPassword;
    QByteArray m_shareSecret;
    QByteArray m_openWalletToken;
    QByteArray m_privateKey;
    QByteArray m_publicKey;

    QNetworkAccessManager *m_networkManager;
    secp256k1_context *m_secpContext;
};

#endif // WALLETOWNERAPI_H
