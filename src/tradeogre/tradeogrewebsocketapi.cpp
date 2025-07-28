#include "tradeogrewebsocketapi.h"


TradeOgreWebSocketApi::TradeOgreWebSocketApi(QObject *parent)
    : QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected,
            this, &TradeOgreWebSocketApi::onConnected);
    connect(&m_socket, &QWebSocket::disconnected,
            this, &TradeOgreWebSocketApi::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived,
            this, &TradeOgreWebSocketApi::onTextMessageReceived);
    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &TradeOgreWebSocketApi::onError);
}

void TradeOgreWebSocketApi::connectSocket()
{
    if (m_isConnected) {
        qWarning() << "WebSocket already connected";
        return;
    }
    QUrl wsUrl(QStringLiteral("wss://tradeogre.com:8443"));
    m_socket.open(wsUrl);
}

void TradeOgreWebSocketApi::disconnectSocket()
{
    if (m_isConnected) {
        m_socket.close();
    }
}

void TradeOgreWebSocketApi::subscribeOrderBook(const QString &market)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected!";
        return;
    }
    QJsonObject obj{{"a","subscribe"},{"e","book"},{"t",market}};
    QByteArray msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    m_socket.sendTextMessage(QString::fromUtf8(msg));
}

void TradeOgreWebSocketApi::subscribeTrades(const QString &market)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected!";
        return;
    }
    QJsonObject obj{{"a","subscribe"},{"e","trade"},{"t",market}};
    QByteArray msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    m_socket.sendTextMessage(QString::fromUtf8(msg));
}

void TradeOgreWebSocketApi::onConnected()
{
    m_isConnected = true;
    emit connected();
    qDebug() << "WebSocket connected to TradeOgre";
}

void TradeOgreWebSocketApi::onDisconnected()
{
    m_isConnected = false;
    emit disconnected();
    qDebug() << "WebSocket disconnected";
}

void TradeOgreWebSocketApi::onTextMessageReceived(const QString &message)
{
    QJsonParseError jerr;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8(), &jerr);
    if (jerr.error != QJsonParseError::NoError) {
        qWarning() << "JSON Parse error:" << jerr.errorString();
        return;
    }
    QJsonObject obj = doc.object();
    const QString event = obj.value("e").toString();
    if (event == "book") {
        emit orderBookUpdate(obj);
    } else if (event == "trade") {
        emit tradeUpdate(obj);
    } else {
        qDebug() << "Unknown event type:" << event << obj;
    }
}

void TradeOgreWebSocketApi::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(m_socket.errorString());
    qWarning() << "WebSocket error:" << m_socket.errorString();
}
