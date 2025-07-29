#include "tradeogrewebsocketapi.h"

/**
 * @brief Constructor
 * Initializes connections for WebSocket events: connected, disconnected, message received, and error
 */
TradeOgreWebSocketApi::TradeOgreWebSocketApi(QObject *parent) :
    QObject(parent)
{
    connect(&m_socket, &QWebSocket::connected,
            this, &TradeOgreWebSocketApi::onConnected);
    connect(&m_socket, &QWebSocket::disconnected,
            this, &TradeOgreWebSocketApi::onDisconnected);
    connect(&m_socket, &QWebSocket::textMessageReceived,
            this, &TradeOgreWebSocketApi::onTextMessageReceived);


    // Connect WebSocket signals to handlers
    #if defined(Q_CC_GNU) || defined(Q_CC_CLANG)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    #elif defined(Q_CC_MSVC)
    #pragma warning(push)
    #pragma warning(disable: 4996)
    #endif

    connect(&m_socket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &TradeOgreWebSocketApi::onError);

    #if defined(Q_CC_GNU) || defined(Q_CC_CLANG)
    #pragma GCC diagnostic pop
    #elif defined(Q_CC_MSVC)
    #pragma warning(pop)
    #endif
}

/**
 * @brief Connect to TradeOgre WebSocket endpoint
 * Prevents reconnecting if already connected
 */
void TradeOgreWebSocketApi::connectSocket()
{
    if (m_isConnected) {
        qWarning() << "WebSocket already connected";
        return;
    }
    QUrl wsUrl(QStringLiteral("wss://tradeogre.com:8443"));
    m_socket.open(wsUrl);
}

/**
 * @brief Gracefully close WebSocket connection
 */
void TradeOgreWebSocketApi::disconnectSocket()
{
    if (m_isConnected) {
        m_socket.close();
    }
}

/**
 * @brief Subscribe to live order book updates for a specific market
 * @param market Market identifier (e.g., "GRIN-BTC")
 */
void TradeOgreWebSocketApi::subscribeOrderBook(const QString &market)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected!";
        return;
    }
    QJsonObject obj{{"a", "subscribe"}, {"e", "book"}, {"t", market}};
    QByteArray msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    qDebug() << Q_FUNC_INFO << QString::fromUtf8(msg);
    m_socket.sendTextMessage(QString::fromUtf8(msg));
}

/**
 * @brief Subscribe to live trade updates for a specific market
 * @param market Market identifier
 */
void TradeOgreWebSocketApi::subscribeTrades(const QString &market)
{
    if (!m_isConnected) {
        qWarning() << "WebSocket not connected!";
        return;
    }
    QJsonObject obj{{"a", "subscribe"}, {"e", "trade"}, {"t", market}};
    QByteArray msg = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    qDebug() << Q_FUNC_INFO << QString::fromUtf8(msg);
    m_socket.sendTextMessage(QString::fromUtf8(msg));
}

/**
 * @brief Called when WebSocket connection is established
 */
void TradeOgreWebSocketApi::onConnected()
{
    m_isConnected = true;
    emit connected();
    qDebug() << "WebSocket connected to TradeOgre";
}

/**
 * @brief Called when WebSocket is closed/disconnected
 */
void TradeOgreWebSocketApi::onDisconnected()
{
    m_isConnected = false;
    emit disconnected();
    qDebug() << "WebSocket disconnected";
}

/**
 * @brief Handles incoming WebSocket messages
 * Parses the message and emits appropriate signals based on event type
 * @param message The raw JSON string received
 */
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
        emit orderBookUpdate(obj);  // Emits signal for order book update
    } else if (event == "trade") {
        emit tradeUpdate(obj);      // Emits signal for trade update
    } else {
        // Unknown event type
        // qDebug() << "Unknown event type:" << event << obj;
    }
}

/**
 * @brief Called on WebSocket error
 * @param error The socket error code (unused)
 */
void TradeOgreWebSocketApi::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    emit errorOccurred(m_socket.errorString());
    qWarning() << "WebSocket error:" << m_socket.errorString();
}
