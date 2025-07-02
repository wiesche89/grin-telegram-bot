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
#include "rewindhash.h"
#include "viewwallet.h"
#include "account.h"
#include "builtoutput.h"
#include "config.h"
#include "slatepack.h"
#include "inittxargs.h"
#include "nodeheight.h"
#include "txlogentry.h"
#include "query.h"
#include "outputcommitmapping.h"
#include "paymentproof.h"
#include "walletinfo.h"
#include "verifypaymentproofstatus.h"

enum StatusMessage {
    StatusMessageUnknown,
    StatusMessageUpdatingOutputs,
    StatusMessageUpdatingTransactions,
    StatusMessageFullScanWarn,
    StatusMessageScanning,
    StatusMessageScanningComplete,
    StatusMessageUpdateWarning
};

// https://docs.rs/grin_wallet_api/5.3.3/grin_wallet_api/trait.OwnerRpc.html
class WalletOwnerApi : public QObject
{
    Q_OBJECT

public:
    WalletOwnerApi(const QString &apiUrl, const QString &apiUser, const QString &apiPassword, QObject *parent = nullptr);

    bool hasConnection() const;

    Result<QList<Account> > accounts();
    Result<BuiltOutput> buildOutputs(QString features, QString amount);
    Result<bool> cancelTx(QString txSlateId, int id);
    Result<bool> changePassword(QString name, QString oldPw, QString newPw);
    Result<bool> closeWallet(QString name);
    Result<QString> createAccountPath(QString label);
    Result<bool> createConfig(Config config);
    Result<QString> createSlatepackMessage(Slate slate, QJsonArray recipients, int senderIndex);
    Result<bool> createWallet(QString name, QString mnemonic, int mnemonicLength, QString password);
    Result<Slatepack> decodeSlatepackMessage(QJsonArray secretIndicesArray, QString message);
    Result<bool> deleteWallet(QString name);
    Result<Slate> finalizeTx(const QJsonObject slate);
    Result<QString> getMnemonic(QString name, QString password);
    Result<RewindHash> getRewindHash();
    Result<QString> getSlatepackAddress(int derivationIndex);
    Result<QString> getSlatepackSecretKey(int derivationIndex);
    Result<Slate> getStoredTx(QString slateId, int id);
    Result<QString> getTopLevelDirectory();
    Result<StatusMessage> getUpdaterMessages(QString &message, quint8 &progress);
    Result<QJsonObject> initSecureApi();
    Result<Slate> initSendTx(InitTxArgs args);
    Result<Slate> issueInvoiceTx(QString amount, QString destAcctName, QString targetSlateVersion);
    Result<NodeHeight> nodeHeight();
    Result<QString> openWallet(QString name, QString password);
    Result<bool> postTx(Slate slate, bool fluff = false);
    Result<Slate> processInvoiceTx(Slate slate, QJsonObject args);
    Result<QList<TxLogEntry> > queryTxs(bool refreshFromNode, Query query);
    Result<QList<OutputCommitMapping> > retrieveOutputs(bool includeSpent, bool refreshFromNode, int txId = 0);
    Result<PaymentProof> retrievePaymentProof(bool refreshFromNode, int txId, QString txSlateId);
    Result<WalletInfo> retrieveSummaryInfo(bool refreshFromNode, int minimum_confirmations);
    Result<QList<TxLogEntry> > retrieveTxs(bool refreshFromNode, int txId, QString txSlateId);
    Result<bool> scan(int startHeight, bool deleteUnconfirmed);
    Result<ViewWallet> scanRewindHash(RewindHash rewindHash, int startHeight);
    Result<bool> setActiveAccount(QString label);
    Result<bool> setTopLevelDirectory(QString dir);
    Result<bool> setTorConfig(TorConfig torConfig);
    Result<Slate> slateFromSlatepackMessage(QString message, QJsonArray secretIndices = QJsonArray() << 0);
    Result<bool> startUpdater(int frequency);
    Result<bool> stopUpdater();
    Result<bool> txLockOutputs(Slate slate);
    Result<VerifyPaymentProofStatus> verifyPaymentProof(PaymentProof proof);

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
