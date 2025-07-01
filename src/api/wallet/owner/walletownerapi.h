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

    QList<Account> accounts();
    BuiltOutput buildOutputs(QString features, QString amount);
    bool cancelTx(QString txSlateId, int id);
    bool changePassword(QString name, QString oldPw, QString newPw);
    bool closeWallet(QString name);
    QString createAccountPath(QString label);
    bool createConfig(Config config);
    QString createSlatepackMessage(Slate slate, QJsonArray recipients, int senderIndex);
    bool createWallet(QString name, QString mnemonic, int mnemonicLength, QString password);
    Slatepack decodeSlatepackMessage(QJsonArray secretIndicesArray, QString message);
    bool deleteWallet(QString name);
    Slate finalizeTx(const QJsonObject slate);
    QString getMnemonic(QString name, QString password);
    RewindHash getRewindHash();
    QString getSlatepackAddress(int derivationIndex);
    QString getSlatepackSecretKey(int derivationIndex);
    Slate getStoredTx(QString slateId, int id);
    QString getTopLevelDirectory();
    StatusMessage getUpdaterMessages(QString &message, quint8 &progress);
    QJsonObject initSecureApi();
    Slate initSendTx(InitTxArgs args);
    Slate issueInvoiceTx(QString amount, QString destAcctName, QString targetSlateVersion);
    NodeHeight nodeHeight();
    QString openWallet(QString name, QString password);
    bool postTx(Slate slate, bool fluff = false);
    Slate processInvoiceTx(Slate slate, QJsonObject args);
    QList<TxLogEntry> queryTxs(bool refreshFromNode, Query query);
    QList<OutputCommitMapping> retrieveOutputs(bool includeSpent, bool refreshFromNode, int txId = 0);
    PaymentProof retrievePaymentProof(bool refreshFromNode, int txId, QString txSlateId);
    WalletInfo retrieveSummaryInfo(bool refreshFromNode, int minimum_confirmations);
    QList<TxLogEntry> retrieveTxs(bool refreshFromNode, int txId, QString txSlateId);
    bool scan(int startHeight, bool deleteUnconfirmed);
    ViewWallet scanRewindHash(RewindHash rewindHash, int startHeight);
    bool setActiveAccount(QString label);
    bool setTopLevelDirectory(QString dir);
    bool setTorConfig(TorConfig torConfig);
    Slate slateFromSlatepackMessage(QString message, QJsonArray secretIndices = QJsonArray() << 0);
    bool startUpdater(int frequency);
    bool stopUpdater();
    bool txLockOutputs(Slate slate);
    VerifyPaymentProofStatus verifyPaymentProof(PaymentProof proof);

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
