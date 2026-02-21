#include <cstdio>
#include <QDateTime>
#include <QDebug>
#include "httpserver.h"

const QByteArray HttpServer::minimal200Answer = QByteArrayLiteral(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 2\r\n"
    "Connection: close\r\n"
    "\r\n"
    "OK");

/**
 * @brief HttpServer::HttpServer
 * @param parent
 */
HttpServer::HttpServer(QObject *parent) : SSLServer(parent)
{
    QObject::connect(this, &HttpServer::connectionReady, this, &HttpServer::handleNewConnection);
}

bool HttpServer::listen(const QHostAddress &address, quint16 port)
{
    auto mark = [](const char *msg) {
        fprintf(stderr, "[WEBHOOK] %s\n", msg);
        fflush(stderr);
    };
    mark("HttpServer::listen start");
    bool ok = SSLServer::listen(address, port);
    mark("HttpServer::listen end");
    return ok;
}

/**
 * @brief HttpServer::addRewriteRule
 * @param host
 * @param path
 * @param delegate
 */
void HttpServer::addRewriteRule(QString host, QString path, QDelegate<void(HttpServerRequest, HttpServerResponse)> delegate)
{
    if (!this->rewriteRules.contains(host)) {
        this->rewriteRules.insert(host, QMultiMap<QString, QDelegate<void(HttpServerRequest, HttpServerResponse)> >());
    }
    this->rewriteRules.find(host).value().insert(path, delegate);
}

/**
 * @brief HttpServer::handleNewConnection
 */
void HttpServer::handleNewConnection()
{
    QObject::connect(this->nextPendingConnection(), &QTcpSocket::readyRead, this, &HttpServer::handleNewData);
}

/**
 * @brief HttpServer::handleNewData
 */
void HttpServer::handleNewData()
{
    // get socket where data are available
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(this->sender());
    if (!socket) {
        return;
    }

    // parse result
    HttpServerRequest request = this->pendingRequests.contains(socket)
                                ? this->pendingRequests.find(socket).value()
                                : this->pendingRequests.insert(socket, HttpServerRequest(new HttpServerRequestPrivate())).value();
    if (!this->parseRequest(*socket, request)) {
        return;
    }

    QString peerIp = socket->peerAddress().toString();
    QString method = request->method;
    QString path = request->url;
    auto sanitizePath = [](const QString &input) {
        if (input.isEmpty()) {
            return input;
        }
        QString trimmed = input;
        if (!trimmed.startsWith('/')) {
            trimmed.prepend('/');
        }
        QStringList segments = trimmed.split('/', Qt::SkipEmptyParts);
        if (segments.isEmpty()) {
            return trimmed;
        }
        if (segments.first().contains(':')) {
            segments[0] = QStringLiteral("<webhook>");
        }
        QString sanitized = '/' + segments.join('/');
        if (trimmed.endsWith('/') && !sanitized.endsWith('/')) {
            sanitized += '/';
        }
        return sanitized;
    };
    QString safePath = sanitizePath(path);
    qint64 bodyLength = request->content.size();
    qInfo() << "[WEBHOOK] IN" << peerIp << method << safePath << "len=" << bodyLength;

    qint64 t0 = QDateTime::currentMSecsSinceEpoch();
    this->sendMinimal200Response(socket);
    qint64 duration = QDateTime::currentMSecsSinceEpoch() - t0;
    qInfo() << "[WEBHOOK] OUT 200 in" << duration << "ms";

    // remove pending request
    this->pendingRequests.remove(socket);

    QString routeHost = request->host;
    if (!this->rewriteRules.contains(routeHost)) {
        if (this->rewriteRules.isEmpty()) {
            qWarning() << "[WEBHOOK] No route for host" << request->host << "original" << request->headers.value("Host");
            return;
        }
        routeHost = this->rewriteRules.begin().key();
        qInfo() << "[WEBHOOK] Falling back to webhook host" << routeHost;
    }

    // invoke routes
    HttpServerResponse response(new HttpServerResponsePrivate);
    auto &hostRoutes = this->rewriteRules.find(routeHost).value();
    for (auto itr = hostRoutes.begin(); itr != hostRoutes.end(); itr++) {
        if (itr.key().startsWith(request->url)) {
            itr.value().invoke(request, response);
        }
    }

    if (response->status) {
        qInfo() << "[WEBHOOK] Handler status" << (qint32)response->status;
    }
}

void HttpServer::sendMinimal200Response(QTcpSocket *socket)
{
    if (!socket) {
        return;
    }
    socket->write(this->minimal200Answer);
    socket->disconnectFromHost();
}

/**
 * @brief HttpServer::parseRequest
 * @param device
 * @param request
 * @return
 */
bool HttpServer::parseRequest(QTcpSocket &device, HttpServerRequest &request)
{
    // parse content
    QByteArray content = device.readAll();
    for (auto itr = content.begin(); itr != content.end(); itr++) {
        // parse method
        if (request->parseState == HttpServerRequestPrivate::Method) {
            if (*itr == ' ') {
                request->parseState = HttpServerRequestPrivate::Url;
            } else {
                request->method += *itr;
            }
        }
        // parse url
        else if (request->parseState == HttpServerRequestPrivate::Url) {
            if (*itr == ' ') {
                request->parseState = HttpServerRequestPrivate::Version;
            } else {
                request->url += *itr;
            }
        }
        // parse version
        else if (request->parseState == HttpServerRequestPrivate::Version) {
            if (*itr == '\r') {
                itr++;
                request->parseState = HttpServerRequestPrivate::Headers;
            } else {
                request->version += *itr;
            }
        }
        // parse headers
        else if (request->parseState == HttpServerRequestPrivate::Headers) {
            // switch from key to value
            if (*itr == ':') {
                request->currentHeaderValue = &request->headers.insert(request->currentHeaderKey, QString("")).value();
                itr++;
            } else {
                // end header
                if (*itr == '\r') {
                    itr++;
                    if (*(itr + 1) == '\r') {
                        itr++;
                        itr++;
                        request->parseState = HttpServerRequestPrivate::Content;
                    }
                    request->currentHeaderKey = QString();
                    request->currentHeaderValue = 0;
                }
                // append to value
                else if (request->currentHeaderValue) {
                    *request->currentHeaderValue += *itr;
                }
                // append to key
                else {
                    request->currentHeaderKey += *itr;
                }
            }
        }
        // parse content
        else if (request->parseState == HttpServerRequestPrivate::Content) {
            if (request->contentLength == -1) {
                request->contentLength = request->headers.value("Content-Length").toInt();
            }
            if (request->contentLength <= 1) {
                request->parseState = HttpServerRequestPrivate::Done;
            }

            // append to content
            if (request->contentLength) {
                request->content += *itr;
                request->contentLength--;
            }
        }
    }

    // demoralize data
    QString hostHeader = request->headers.value("Host");
    int colonPos = hostHeader.indexOf(':');
    if (colonPos != -1) {
        hostHeader = hostHeader.left(colonPos);
    }
    request->host = hostHeader;

    // if we are done return true, otherwise false
    return request->parseState == HttpServerRequestPrivate::Done;
}
