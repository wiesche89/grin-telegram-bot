#include "gateioworker.h"
#include <QDateTime>
#include <QPainter>
#include <QStandardPaths>
#include <QDir>

/**
 * @brief GateIoWorker::GateIoWorker
 * @param bot
 * @param settings
 */
GateIoWorker::GateIoWorker(TelegramBot *bot, QSettings *settings)
    : m_bot(bot), m_settings(settings), m_client(nullptr)
{
}

/**
 * @brief GateIoWorker::init
 * @return
 */
bool GateIoWorker::init()
{
    if (!m_client) {
        const QString apiKey = m_settings->value("gateio/apiKey").toString();
        const QString apiSecret = m_settings->value("gateio/apiSecret").toString();
        m_client = new GateIoClient(apiKey, apiSecret, this);

        connect(m_client, &GateIoClient::errorOccurred, this, [](const QString &msg) {
            qWarning() << "[GateIoClient]" << msg;
        });
    }

    connect(m_bot, SIGNAL(newMessage(TelegramBotUpdate)), this, SLOT(onMessage(TelegramBotUpdate)));
    return true;
}

/**
 * @brief GateIoWorker::onMessage
 * @param update
 */
void GateIoWorker::onMessage(TelegramBotUpdate update)
{
    if (update->type != TelegramBotMessageType::Message) return;
    TelegramBotMessage &message = *update->message;

    //-------/price----------
    if (message.text.startsWith("/price")) {
        QMetaObject::Connection *conn = new QMetaObject::Connection;
        *conn = connect(m_client, &GateIoClient::tickerReceived, this, [this, message, conn](const QJsonArray &tickers) {
            disconnect(*conn);
            delete conn;
            for (const auto &val : tickers) {
                QJsonObject obj = val.toObject();
                QString pair = obj["currency_pair"].toString();
                if (pair != "GRIN_USDT") continue;

                double last         = obj["last"].toString().toDouble();
                double high         = obj["high_24h"].toString().toDouble();
                double low          = obj["low_24h"].toString().toDouble();
                double change       = obj["change_percentage"].toString().toDouble();
                double bid          = obj["highest_bid"].toString().toDouble();
                double ask          = obj["lowest_ask"].toString().toDouble();
                double baseVolume   = obj["base_volume"].toString().toDouble();
                double quoteVolume  = obj["quote_volume"].toString().toDouble();

                QLocale locale(QLocale::English);
                QString text;
                text += "📊 GRIN/USDT Market Overview\n\n";
                text += QString("• Last Price:      %1   (%2%)\n")
                            .arg(locale.toString(last, 'f', 8), -14)
                            .arg(locale.toString(change, 'f', 2));
                text += QString("• 24h High:        %1\n").arg(locale.toString(high, 'f', 8));
                text += QString("• 24h Low:         %1\n").arg(locale.toString(low, 'f', 8));
                text += QString("• Best Bid:        %1\n").arg(locale.toString(bid, 'f', 8));
                text += QString("• Best Ask:        %1\n").arg(locale.toString(ask, 'f', 8));
                text += QString("• Volume (GRIN):   %1\n").arg(locale.toString(baseVolume, 'f', 2));
                text += QString("• Volume (USDT):   %1\n").arg(locale.toString(quoteVolume, 'f', 2));

                m_bot->sendMessage(message.chat.id, escapeMarkdownV2(text), 0, TelegramBot::Markdown);
                break;
            }
        });

        m_client->getTicker("GRIN_USDT");
        return;
    }

    //-------/orderbook----------
    if (message.text.startsWith("/orderbook")) {
        QMetaObject::Connection *conn = new QMetaObject::Connection;

        *conn = connect(m_client, &GateIoClient::orderBookReceived, this, [=](const QJsonObject &book) mutable {
            disconnect(*conn);
            delete conn;

            QJsonArray asks = book["asks"].toArray();
            QJsonArray bids = book["bids"].toArray();

            QString text;
            text += "📘 Order Book: GRIN/USDT\n\n";
            text += "Asks (Highest 10):\n";
            text += "    Price       ×     Quantity     =     USDT\n";

            for (int i = qMin(10, asks.size()) - 1; i >= 0; --i) {
                QJsonArray a = asks[i].toArray();
                double price = a[0].toString().toDouble();
                double qty = a[1].toString().toDouble();
                double total = price * qty;

                text += QString("  %1  ×  %2  =  %3\n")
                            .arg(QString::number(price, 'f', 5), 10)
                            .arg(QString::number(qty, 'f', 2), 12)
                            .arg(QString::number(total, 'f', 2), 10);
            }

            text += "\nBids (Highest 10):\n";
            text += "     Price       ×     Quantity     =     USDT\n";

            for (int i = 0; i < qMin(10, bids.size()); ++i) {
                QJsonArray b = bids[i].toArray();
                double price = b[0].toString().toDouble();
                double qty = b[1].toString().toDouble();
                double total = price * qty;

                text += QString("  %1  ×  %2  =  %3\n")
                            .arg(QString::number(price, 'f', 5), 10)
                            .arg(QString::number(qty, 'f', 2), 12)
                            .arg(QString::number(total, 'f', 2), 10);
            }

            m_bot->sendMessage(message.chat.id, escapeMarkdownV2(text), 0, TelegramBot::TelegramFlags::Markdown);
        });

        m_client->getOrderBook("GRIN_USDT",10);
        return;
    }

    //-------/chart----------
    if (message.text.startsWith("/chart")) {
        QMetaObject::Connection *conn = new QMetaObject::Connection;

        *conn = connect(m_client, &GateIoClient::candlesticksReceived, this, [=](const QJsonArray &chart) mutable {
            if (chart.isEmpty()) return;

            disconnect(*conn);
            delete conn;

            QString path = renderChartToFile(chart, "GRIN_USDT");
            qDebug() << "Chart path:" << path;
            qDebug() << "Exists:" << QFile::exists(path);
            if (!path.isEmpty()) {
                QFile file(path);
                if (file.open(QIODevice::ReadOnly)) {
                    QByteArray data = file.readAll();
                    m_bot->sendPhoto(message.chat.id, data, "📈 GRIN/USDT 4h Chart");
                    file.close();
                } else {
                    sendUserMessage(message, "❌ Failed to read chart image.", false);
                }
            } else {
                sendUserMessage(message, "❌ Could not generate GRIN/USDT chart.", false);
            }
        });

        m_client->getCandlesticks("GRIN_USDT", "4h", 150);
        return;
    }

    //-------/history----------
    if (message.text.startsWith("/history")) {
        QMetaObject::Connection *conn = new QMetaObject::Connection;

        *conn = connect(m_client, &GateIoClient::tradesReceived, this, [=](const QJsonArray &data) mutable {
            if (data.isEmpty()) return;

            disconnect(*conn);
            delete conn;

            QStringList out;
            out << "📈 Recent Trades for GRIN/USDT\n";
            out << "Date                 Side   Amount         Price";

            for (int i = 0; i < qMin(12, data.size()); ++i) {
                QJsonObject obj = data.at(i).toObject();
                QString time   = QDateTime::fromSecsSinceEpoch(obj["create_time"].toString().toLongLong()).toString("yyyy-MM-dd hh:mm:ss");
                QString side   = obj["side"].toString().toUpper();
                QString amount = QString::number(obj["amount"].toString().toDouble(), 'f', 2);
                QString price  = QString::number(obj["price"].toString().toDouble(), 'f', 5);
                QString usdt =  QString::number(obj["price"].toString().toDouble()*obj["amount"].toString().toDouble(),'f',2);
                out << QString("%1  %2  %3  @ %4  (%5 USDT)")
                           .arg(time, -20)
                           .arg(side, -6)
                           .arg(amount, 10)
                           .arg(price)
                           .arg(usdt);
            }

            m_bot->sendMessage(message.chat.id, escapeMarkdownV2(out.join("\n")), 0, TelegramBot::TelegramFlags::Markdown);
        });

        m_client->getTrades("GRIN_USDT", 20);
        return;
    }

}

/**
 * @brief GateIoWorker::sendUserMessage
 * @param message
 * @param content
 * @param plain
 */
void GateIoWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{
    QString msg;
    if (plain) {
        msg = content;
    } else {
        msg = QString("Hi "
                      + message.from.firstName
                      + ",\n"
                      + content);
    }

    m_bot->sendMessage(message.chat.id,
                       msg,
                       0,
                       TelegramBot::NoFlag,
                       TelegramKeyboardRequest(),
                       nullptr);
}

/**
 * @brief GateIoWorker::renderChartToFile
 * @param chart
 * @param market
 * @return
 */
QString GateIoWorker::renderChartToFile(const QJsonArray &chart, const QString &market)
{
    const int width = 900;
    const int height = 500;
    const int volumeHeight = 80;
    const int marginLeft = 60;
    const int marginRight = 20;
    const int marginTop = 20;
    const int marginBottom = 40;

    QImage image(width, height, QImage::Format_ARGB32);
    image.fill(QColor("#222222"));

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);

    struct Candle {
        qint64 ts;
        double open, high, low, close, volume;
    };

    QVector<Candle> candles;
    double minPrice = std::numeric_limits<double>::max();
    double maxPrice = std::numeric_limits<double>::lowest();
    double maxVolume = 0;

    for (const QJsonValue &v : chart) {
        QJsonArray e = v.toArray();
        if (e.size() < 8 || e[7].toString() != "true") continue; // only complete candles

        Candle c;
        c.ts     = e[0].toString().toLongLong();
        c.open   = e[5].toString().toDouble();
        c.high   = e[3].toString().toDouble();
        c.low    = e[4].toString().toDouble();
        c.close  = e[2].toString().toDouble();
        c.volume = e[1].toString().toDouble();

        minPrice = qMin(minPrice, c.low);
        maxPrice = qMax(maxPrice, c.high);
        maxVolume = qMax(maxVolume, c.volume);

        candles.append(c);
    }

    if (candles.isEmpty()) return QString();

    //2% Padding
    double paddedMinPrice = minPrice - (maxPrice - minPrice) * 0.02;
    double paddedMaxPrice = maxPrice + (maxPrice - minPrice) * 0.02;
    double priceRange = paddedMaxPrice - paddedMinPrice;

    const int plotTop = marginTop;
    const int plotBottom = height - marginBottom - volumeHeight;
    const int plotLeft = marginLeft;
    const int plotRight = width - marginRight;
    const int plotHeight = plotBottom - plotTop;
    const int plotWidth = plotRight - plotLeft;
    const int candleWidth = qMax(2, plotWidth / candles.size());

    auto toY = [&](double price) -> int {
        return plotBottom - ((price - paddedMinPrice) / priceRange) * plotHeight;
    };

    // Frame
    p.setPen(QPen(Qt::white, 1));
    p.drawRect(0, 0, width - 1, height - 1);
    p.drawLine(plotLeft, plotTop, plotLeft, plotBottom);
    p.drawLine(plotLeft, plotBottom, plotRight, plotBottom);


    int ySteps = 6;
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 9));
    for (int i = 0; i <= ySteps; ++i) {
        double price = paddedMinPrice + (priceRange * i / ySteps);
        int y = toY(price);
        p.drawLine(plotLeft - 5, y, plotLeft, y);
        p.drawText(2, y + 4, QString::number(price, 'f', 4));
    }

    // X
    int xSteps = 6;
    for (int i = 0; i <= xSteps; ++i) {
        int idx = (candles.size() - 1) * i / xSteps;
        int x = plotLeft + idx * candleWidth;
        QString label = QDateTime::fromSecsSinceEpoch(candles[idx].ts).toString("dd.MM");
        p.drawLine(x, plotBottom, x, plotBottom + 5);
        p.drawText(x - 20, plotBottom + 18, label);
    }

    // Candlesticks
    for (int i = 0; i < candles.size(); ++i) {
        const Candle &c = candles[i];
        int x = plotLeft + i * candleWidth;
        int centerX = x + candleWidth / 2;

        int yOpen  = toY(c.open);
        int yClose = toY(c.close);
        int yHigh  = toY(c.high);
        int yLow   = toY(c.low);

        QColor color = (c.close >= c.open) ? QColor("#00FF00") : QColor("#FF4040");
        p.setPen(color);
        p.setBrush(color);

        // Wick
        p.drawLine(centerX, yHigh, centerX, yLow);

        // Body
        int bodyTop = qMin(yOpen, yClose);
        int bodyBottom = qMax(yOpen, yClose);
        QRect body(centerX - candleWidth / 4, bodyTop, candleWidth / 2, qMax(1, bodyBottom - bodyTop));
        p.drawRect(body);
    }

    // Volume
    int volumeTop = plotBottom + 20;
    int volumeBottom = height - marginBottom;
    int volumeHeightReal = volumeBottom - volumeTop;

    for (int i = 0; i < candles.size(); ++i) {
        const Candle &c = candles[i];
        int x = plotLeft + i * candleWidth;
        int volHeight = (maxVolume > 0) ? (c.volume / maxVolume) * volumeHeightReal : 0;
        QColor color = (c.close >= c.open) ? QColor("#00FF00") : QColor("#FF4040");

        p.setPen(Qt::NoPen);
        p.setBrush(color);
        p.drawRect(x + 1, volumeBottom - volHeight, candleWidth - 2, volHeight);
    }

    // Titel
    p.setPen(Qt::yellow);
    p.setFont(QFont("Arial", 14, QFont::Bold));
    p.drawText(QRect(0, 0, width, 25), Qt::AlignCenter, market + " 4h Chart");

    p.end();

    QString path = QString("chart_%1.png").arg(market);
    if (!image.save(path)) {
        qWarning() << "Failed to save chart image to" << path;
        return QString();
    }

    return path;
}

/**
 * @brief GateIoWorker::escapeMarkdownV2
 * @param text
 * @return
 */
QString GateIoWorker::escapeMarkdownV2(const QString &text)
{
    QString escaped = text;
    static const QStringList specialChars = {
        "_", "*", "[", "]", "(", ")", "~", "`", ">", "#", "+", "-", "=", "|", "{", "}", ".", "!"
    };
    for (const QString &c : specialChars) {
        escaped.replace(c, "\\" + c);
    }
    return escaped;
}
