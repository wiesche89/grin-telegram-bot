#include "sslserver.h"

/**
 * @brief SSLServer::SSLServer
 * @param parent
 */
SSLServer::SSLServer(QObject *parent) : QTcpServer(parent)
{
    QObject::connect(this, &SSLServer::newConnection, this, &SSLServer::handleNewConnection);
}

/**
 * @brief SSLServer::setPrivateKey
 * @param keyFile
 * @param algorithmn
 * @return
 */
bool SSLServer::setPrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn)
{
    QByteArray content = SSLServer::getFileContent(keyFile);
    return this->setPrivateKeyRaw(content, algorithmn);
}

/**
 * @brief SSLServer::setPrivateKeyRaw
 * @param keyFileContent
 * @param algorithmn
 * @return
 */
bool SSLServer::setPrivateKeyRaw(QByteArray &keyFileContent, QSsl::KeyAlgorithm algorithmn)
{
    if (keyFileContent.isEmpty()) {
        this->sslKey = QSslKey();
        return true;
    } else {
        this->sslKey = QSslKey(keyFileContent, algorithmn);
    }
    return !this->sslKey.isNull();
}

/**
 * @brief SSLServer::isSamePrivateKey
 * @param keyFile
 * @param algorithmn
 * @return
 */
bool SSLServer::isSamePrivateKey(QString keyFile, QSsl::KeyAlgorithm algorithmn)
{
    auto data = this->getFileContent(keyFile);
    return this->isSamePrivateKey(data, algorithmn);
}

/**
 * @brief SSLServer::isSamePrivateKey
 * @param keyFileContent
 * @param algorithmn
 * @return
 */
bool SSLServer::isSamePrivateKey(QByteArray &keyFileContent, QSsl::KeyAlgorithm algorithmn)
{
    return QSslKey(keyFileContent, algorithmn) == this->sslKey;
}

/**
 * @brief SSLServer::addCert
 * @param certFile
 * @param format
 * @return
 */
QSslCertificate SSLServer::addCert(QString certFile, QSsl::EncodingFormat format)
{
    QByteArray content = SSLServer::getFileContent(certFile);
    return this->addCertRaw(content, format);
}

/**
 * @brief SSLServer::addCertRaw
 * @param certData
 * @param format
 * @return
 */
QSslCertificate SSLServer::addCertRaw(QByteArray &certData, QSsl::EncodingFormat format)
{
    // param check
    if (certData.isEmpty()) {
        return QSslCertificate();
    }
    QSslCertificate cert(certData, format);
    if (cert.isNull() || cert.subjectInfo(QSslCertificate::CommonName).isEmpty()) {
        return cert;
    }
    this->sslCerts.append(cert);
    return cert;
}

/**
 * @brief SSLServer::addWhiteListHost
 * @param address
 */
void SSLServer::addWhiteListHost(QString address)
{
    this->whiteListHosts.append(qMakePair(QHostAddress(address), -1));
}

/**
 * @brief SSLServer::addWhiteListHostSubnet
 * @param subnet
 */
void SSLServer::addWhiteListHostSubnet(QString subnet)
{
    this->whiteListHosts.append(QHostAddress::parseSubnet(subnet));
}

/**
 * @brief SSLServer::handleNewConnection
 */
void SSLServer::handleNewConnection()
{
    if (this->sslKey.isNull()) {
        emit this->connectionReady();
    }
}

/**
 * @brief SSLServer::getFileContent
 * @param file
 * @return
 */
QByteArray SSLServer::getFileContent(QString file)
{
    // get key content
    QFile ffile(file);
    if (!ffile.open(QFile::ReadOnly)) {
        return QByteArray();
    }
    return ffile.readAll();
}

/**
 * @brief SSLServer::incomingConnection
 * @param socketDescriptor
 */
void SSLServer::incomingConnection(qintptr socketDescriptor)
{
    // use default QTcpServer implementation if we have no present private key
    if (this->sslKey.isNull()) {
        QTcpSocket *socket = new QTcpSocket(this);
        if (!socket->setSocketDescriptor(socketDescriptor) || !this->isConnectionAllowed(socket)) {
            delete socket;
        } else {
            this->addPendingConnection(socket);
        }
        return;
    }

    // create ssl connection
    QSslSocket *sslSocket = new QSslSocket(this);
    QObject::connect(sslSocket, &QSslSocket::disconnected, sslSocket, &QSslSocket::deleteLater);
    QObject::connect(sslSocket, SIGNAL(error(QAbstractSocket::SocketError)), sslSocket, SLOT(deleteLater()));
    if (sslSocket->setSocketDescriptor(socketDescriptor) && this->isConnectionAllowed(sslSocket)) {
        QObject::connect(sslSocket, &QSslSocket::encrypted, this, &SSLServer::connectionReady);
        sslSocket->setPeerVerifyMode(QSslSocket::VerifyNone);
        sslSocket->setLocalCertificateChain(this->sslCerts);
        sslSocket->setPrivateKey(this->sslKey);
        sslSocket->setProtocol(QSsl::TlsV1_2OrLater);
        sslSocket->startServerEncryption();
        this->addPendingConnection(sslSocket);
    } else {
        delete sslSocket;
    }
}

/**
 * @brief SSLServer::isConnectionAllowed
 * @param socket
 * @return
 */
bool SSLServer::isConnectionAllowed(QTcpSocket *socket)
{
    // param check
    if (!socket) {
        return false;
    }
    if (this->whiteListHosts.isEmpty()) {
        return true;
    }

    // check if ip is whitelisted, if not return false
    QHostAddress addressIpv4(socket->peerAddress().toIPv4Address());
    for (QPair<QHostAddress, int> &host : this->whiteListHosts) {
        if (host.second == -1 && addressIpv4.isEqual(host.first)) {
            return true;
        } else if (addressIpv4.isInSubnet(host)) {
            return true;
        }
    }

    qDebug("SSLServer::isConnectionAllowed: Host %s Rejected", qPrintable(addressIpv4.toString()));
    return false;
}
