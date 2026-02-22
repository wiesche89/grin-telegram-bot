#include <cstdio>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QMetaObject>
#include <QTextStream>
#include <QUrl>
#include "telegrambot.h"

QMap<qint16, HttpServer *> TelegramBot::webHookWebServers = QMap<qint16, HttpServer *>();

static QString sanitizeWebhookUrl(const QString &url)
{
    if (url.isEmpty()) {
        return url;
    }
    QUrl parsed(url);
    if (!parsed.isValid()) {
        return url;
    }
    QString decodedPath = QUrl::fromPercentEncoding(parsed.path().toUtf8());
    parsed.setPath(decodedPath);
    return parsed.toString(QUrl::StripTrailingSlash);
}

static QStringList loadTelegramCidrs()
{
    static QStringList cached;
    static bool attempted = false;
    if (attempted) {
        return cached;
    }
    attempted = true;

    QString dataDir = qEnvironmentVariable("DATA_DIR");
    QString path;
    if (!dataDir.isEmpty()) {
        path = QDir(dataDir).filePath("etc/telegram_cidrs.txt");
    } else {
        path = QDir(QCoreApplication::applicationDirPath()).filePath("etc/telegram_cidrs.txt");
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open Telegram CIDR file:" << path << file.errorString();
        return cached;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        cached << line;
    }
    qInfo() << "Loaded" << cached.size() << "Telegram CIDRs from" << path;
    return cached;
}

/**
 * @brief TelegramBot::constructInlineMenu
 * @param menu
 * @param dataPattern
 * @param page
 * @param columns
 * @param limit
 * @param lastPage
 * @return
 */
TelegramKeyboardRequest TelegramBot::constructInlineMenu(QList<QString> menu, QString dataPattern, int page, int columns, int limit,
                                                         QString lastPage)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // generate time zone keyboard for continent
    TelegramKeyboardRequest keyboard;
    int column = 0;
    for (QString menuEntry : !limit ? menu : menu.mid(!page ? 0 : (page * limit) - 1, limit)) {
        if (!column || column == columns) {
            keyboard.append(QList<TelegramBotKeyboardButtonRequest>());
            column = 0;
        }
        keyboard.last().append(TelegramBot::constructInlineButton(menuEntry, QString(dataPattern).arg(menuEntry)));
        column++;
    }

    // add back/forward button?
    QString currentPage = !dataPattern.contains(".") ? QString(dataPattern).arg("") : dataPattern.left(dataPattern.lastIndexOf("."));
    bool hasBackButton = page > 0;
    bool hasForwardButton = limit && (page * limit) + limit < menu.length();

    // construct menu row
    if (!lastPage.isEmpty() || hasBackButton || hasForwardButton) {
        keyboard.append(QList<TelegramBotKeyboardButtonRequest>{});
    }
    if (!lastPage.isEmpty()) {
        keyboard.last().append({TelegramBot::constructInlineButton("Back", lastPage)});
    }
    if (hasBackButton) {
        keyboard.last().append({TelegramBot::constructInlineButton("<", QString("%1..%2").arg(currentPage).arg(page - 1))});
    }
    if (hasForwardButton) {
        keyboard.last().append({TelegramBot::constructInlineButton(">", QString("%1..%2").arg(currentPage).arg(page + 1))});
    }

    // return keyboard
    return keyboard;
}

/**
 * @brief TelegramBot::TelegramBot
 * @param apikey
 * @param parent
 */
TelegramBot::TelegramBot(QString apikey, QObject *parent) : QObject(parent),
    apiKey(apikey),
    m_webhookPath(apikey)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    this->m_webhookHealthTimer = new QTimer(this);
    this->m_webhookHealthTimer->setInterval(30 * 1000);
    connect(this->m_webhookHealthTimer, &QTimer::timeout, this, &TelegramBot::checkWebhookHealth);
    this->m_webhookHealthTimer->start();
}

/**
 * @brief TelegramBot::~TelegramBot
 */
TelegramBot::~TelegramBot()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    qDeleteAll(this->messageRoutes);
}

/**
 * @brief TelegramBot::getMe
 * @return
 */
TelegramBotUser TelegramBot::getMe()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    return TelegramBotUser(this->callApiJson("getMe").value("result").toObject());
}

/**
 * @brief TelegramBot::sendChatAction
 * @param chatId
 * @param action
 * @param response
 */
void TelegramBot::sendChatAction(QVariant chatId, TelegramBotChatAction action, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    return this->sendChatAction(chatId, action == TelegramBotChatAction::Typing ? "typing"
                                : action == TelegramBotChatAction::UploadPhoto ? "upload_photo"
                                : action == TelegramBotChatAction::RecordVideo ? "record_video"
                                : action == TelegramBotChatAction::UploadVideo ? "upload_video"
                                : action == TelegramBotChatAction::RecordAudio ? "record_audio"
                                : action == TelegramBotChatAction::UploadAudio ? "upload_audio"
                                : action == TelegramBotChatAction::UploadDocument ? "upload_document"
                                : action == TelegramBotChatAction::FindLocation ? "find_location"
                                : action == TelegramBotChatAction::RecordVideoNote ? "record_video_note"
                                : action == TelegramBotChatAction::UploadVideoNote ? "upload_video_note" : "", response);
}

/**
 * @brief TelegramBot::sendChatAction
 * @param chatId
 * @param action
 * @param response
 */
void TelegramBot::sendChatAction(QVariant chatId, QString action, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // param check
    if (action.isEmpty()) {
        return;
    }

    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("action", action);

    // call api
    this->callApiTemplate("sendChatAction", params, response);
}

/**
 * @brief TelegramBot::getFile
 * @param fileId
 * @param generateAbsoluteLink
 * @return
 */
TelegramBotFile TelegramBot::getFile(QString fileId, bool generateAbsoluteLink)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // prepare
    QDateTime validUntil = QDateTime::currentDateTime().addSecs(3600);

    // build params
    QUrlQuery params;
    params.addQueryItem("file_id", fileId);

    // construct TelegramBotFile
    TelegramBotFile file(this->callApiJson("getFile", params).value("result").toObject());
    file.validUntil = validUntil;
    if (generateAbsoluteLink && !file.filePath.isEmpty()) {
        file.link = QString("https://api.telegram.org/file/bot%1/%2").arg(this->apiKey, file.filePath);
    }
    return file;
}

/**
 * @brief TelegramBot::getUserProfilePhotos
 * @param userId
 * @param offset
 * @param limit
 * @return
 */
TelegramBotUserProfilePhotos TelegramBot::getUserProfilePhotos(qint32 userId, int offset, int limit)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("user_id", QString::number(userId));
    if (offset > 0) {
        params.addQueryItem("offset", QString::number(offset));
    }
    if (limit > 0) {
        params.addQueryItem("limit", QString::number(limit));
    }

    // call api and return constructed data
    return TelegramBotUserProfilePhotos(this->callApiJson("getUserProfilePhotos", params).value("result").toObject());
}

/**
 * @brief TelegramBot::kickChatMember
 * @param chatId
 * @param userId
 * @param response
 */
void TelegramBot::kickChatMember(QVariant chatId, qint32 userId, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    this->callApiTemplate("kickChatMember", params, response);
}

/**
 * @brief TelegramBot::unbanChatMember
 * @param chatId
 * @param userId
 * @param response
 */
void TelegramBot::unbanChatMember(QVariant chatId, qint32 userId, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    this->callApiTemplate("unbanChatMember", params, response);
}

/**
 * @brief TelegramBot::leaveChat
 * @param chatId
 * @param response
 */
void TelegramBot::leaveChat(QVariant chatId, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    this->callApiTemplate("leaveChat", params, response);
}

/**
 * @brief TelegramBot::getChat
 * @param chatId
 * @return
 */
TelegramBotChat TelegramBot::getChat(QVariant chatId)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    return TelegramBotChat(this->callApiJson("getChat", params).value("result").toObject());
}

/**
 * @brief TelegramBot::getChatAdministrators
 * @param chatId
 * @return
 */
QList<TelegramBotChatMember> TelegramBot::getChatAdministrators(QVariant chatId)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());

    // call api and parse result
    QList<TelegramBotChatMember> chatMemebers;
    JsonHelperT<TelegramBotChatMember>::jsonPathGetArray(this->callApiJson("getChatAdministrators", params), "result", chatMemebers);
    return chatMemebers;
}

/**
 * @brief TelegramBot::getChatMembersCount
 * @param chatId
 * @return
 */
int TelegramBot::getChatMembersCount(QVariant chatId)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    return this->callApiJson("getChatMembersCount", params).value("result").toInt();
}

/**
 * @brief TelegramBot::getChatMember
 * @param chatId
 * @param userId
 * @return
 */
TelegramBotChatMember TelegramBot::getChatMember(QVariant chatId, qint32 userId)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("user_id", QString::number(userId));

    return TelegramBotChatMember(this->callApiJson("getChatMember", params).value("result").toObject());
}

/**
 * @brief TelegramBot::answerCallbackQuery
 * @param callbackQueryId
 * @param text
 * @param showAlert
 * @param cacheTime
 * @param url
 * @param response
 */
void TelegramBot::answerCallbackQuery(QString callbackQueryId, QString text, bool showAlert, int cacheTime, QString url, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("callback_query_id", callbackQueryId);
    if (!text.isNull()) {
        params.addQueryItem("text", text);
    }
    if (showAlert) {
        params.addQueryItem("show_alert", "true");
    }
    if (!url.isNull()) {
        params.addQueryItem("url", url);
    }
    if (cacheTime > 0) {
        params.addQueryItem("cache_time", QString::number(cacheTime));
    }

    this->callApiTemplate("answerCallbackQuery", params, response);
}

/**
 * @brief TelegramBot::sendMessage
 * @param chatId
 * @param text
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendMessage(QVariant chatId, QString text, int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard,
                              TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("text", text);
    if (flags && TelegramFlags::Markdown) {
        params.addQueryItem("parse_mode", "MarkdownV2");
    } else if (flags && TelegramFlags::Html) {
        params.addQueryItem("parse_mode", "HTML");
    }
    if (flags && TelegramFlags::DisableWebPagePreview) {
        params.addQueryItem("disable_web_page_preview", "true");
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    return this->callApiTemplate("sendMessage", params, response);
}

/**
 * @brief TelegramBot::editMessageText
 * @param chatId
 * @param messageId
 * @param text
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::editMessageText(QVariant chatId, QVariant messageId, QString text, TelegramFlags flags, TelegramKeyboardRequest keyboard,
                                  bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // if we have a null messageId, and user don't request a response, call send Message
    if (!response && messageId.isNull()) {
        return this->sendMessage(chatId, text, 0, flags, keyboard);
    }

    // determine message id type
    bool isInlineMessageId = messageId.typeId() == QMetaType::QString;

    QUrlQuery params;
    if (!isInlineMessageId && !chatId.isNull()) {
        params.addQueryItem("chat_id", chatId.toString());
    }
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());
    params.addQueryItem("text", text);
    if (flags && TelegramFlags::Markdown) {
        params.addQueryItem("parse_mode", "MarkdownV2");
    } else if (flags && TelegramFlags::Html) {
        params.addQueryItem("parse_mode", "HTML");
    }
    if (flags && TelegramFlags::DisableWebPagePreview) {
        params.addQueryItem("disable_web_page_preview", "true");
    }

    // only build inline keyboard
    if (!(flags && TelegramFlags::ReplyKeyboardMarkup) && !(flags && TelegramFlags::ForceReply)
        && !(flags && TelegramFlags::ReplyKeyboardRemove)) {
        this->hanldeReplyMarkup(params, flags, keyboard);
    }

    // call api
    this->callApiTemplate("editMessageText", params, response);
}

/**
 * @brief TelegramBot::editMessageCaption
 * @param chatId
 * @param messageId
 * @param caption
 * @param keyboard
 * @param response
 */
void TelegramBot::editMessageCaption(QVariant chatId, QVariant messageId, QString caption, TelegramKeyboardRequest keyboard, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // determine message id type
    bool isInlineMessageId = messageId.typeId() == QMetaType::QString;

    QUrlQuery params;
    if (!isInlineMessageId && !chatId.isNull()) {
        params.addQueryItem("chat_id", chatId.toString());
    }
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }

    // only build inline keyboard
    this->hanldeReplyMarkup(params, TelegramFlags(), keyboard);

    // call api
    this->callApiTemplate("editMessageCaption", params, response);
}

/**
 * @brief TelegramBot::editMessageReplyMarkup
 * @param chatId
 * @param messageId
 * @param keyboard
 * @param response
 */
void TelegramBot::editMessageReplyMarkup(QVariant chatId, QVariant messageId, TelegramKeyboardRequest keyboard, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // determine message id type
    bool isInlineMessageId = messageId.typeId() == QVariant::String;

    QUrlQuery params;
    if (!isInlineMessageId && !chatId.isNull()) {
        params.addQueryItem("chat_id", chatId.toString());
    }
    params.addQueryItem(isInlineMessageId ? "inline_message_id" : "message_id", messageId.toString());

    // only build inline keyboard
    this->hanldeReplyMarkup(params, TelegramFlags(), keyboard);

    // call api
    this->callApiTemplate("editMessageReplyMarkup", params, response);
}

/**
 * @brief TelegramBot::forwardMessage
 * @param targetChatId
 * @param fromChatId
 * @param fromMessageId
 * @param flags
 * @param response
 */
void TelegramBot::forwardMessage(QVariant targetChatId, QVariant fromChatId, qint32 fromMessageId, TelegramFlags flags,
                                 TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", targetChatId.toString());
    params.addQueryItem("from_chat_id", fromChatId.toString());
    params.addQueryItem("message_id", QString::number(fromMessageId));
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }

    this->callApiTemplate("forwardMessage", params, response);
}

/**
 * @brief TelegramBot::deleteMessage
 * @param chatId
 * @param messageId
 * @param response
 */
void TelegramBot::deleteMessage(QVariant chatId, qint32 messageId, bool *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("message_id", QString::number(messageId));

    this->callApiTemplate("deleteMessage", params, response);
}

/**
 * @brief TelegramBot::sendPhoto
 * @param chatId
 * @param photo
 * @param caption
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendPhoto(QVariant chatId, QVariant photo, QString caption, int replyToMessageId, TelegramFlags flags,
                            TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("photo", "photo", photo, params);

    // call api
    this->callApiTemplate("sendPhoto", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendAudio
 * @param chatId
 * @param audio
 * @param caption
 * @param performer
 * @param title
 * @param duration
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendAudio(QVariant chatId, QVariant audio, QString caption, QString performer, QString title, int duration,
                            int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }
    if (duration >= 0) {
        params.addQueryItem("duration", QString::number(duration));
    }
    if (!performer.isNull()) {
        params.addQueryItem("performer", performer);
    }
    if (!title.isNull()) {
        params.addQueryItem("title", title);
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("audio", "audio", audio, params);

    // call api
    this->callApiTemplate("sendAudio", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendDocument
 * @param chatId
 * @param document
 * @param caption
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendDocument(QString filename, QVariant chatId, QVariant document, QString caption, int replyToMessageId,
                               TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    qDebug() << Q_FUNC_INFO << " ";
    qDebug() << filename;
    qDebug() << chatId;
    qDebug() << document;
    qDebug() << caption;
    qDebug() << replyToMessageId;

    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile(filename, "document", document, params);

    // call api
    this->callApiTemplate("sendDocument", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendSticker
 * @param chatId
 * @param sticker
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendSticker(QVariant chatId, QVariant sticker, int replyToMessageId, TelegramFlags flags,
                              TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("sticker", "sticker", sticker, params);

    // call api
    this->callApiTemplate("sendSticker", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendVideo
 * @param chatId
 * @param video
 * @param caption
 * @param duration
 * @param width
 * @param height
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendVideo(QVariant chatId, QVariant video, QString caption, int duration, int width, int height, int replyToMessageId,
                            TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }
    if (duration >= 0) {
        params.addQueryItem("duration", QString::number(duration));
    }
    if (width >= 0) {
        params.addQueryItem("width", QString::number(width));
    }
    if (height >= 0) {
        params.addQueryItem("height", QString::number(height));
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("video", "video", video, params);

    // call api
    this->callApiTemplate("sendVideo", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendVoice
 * @param chatId
 * @param voice
 * @param caption
 * @param duration
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendVoice(QVariant chatId, QVariant voice, QString caption, int duration, int replyToMessageId, TelegramFlags flags,
                            TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (!caption.isNull()) {
        params.addQueryItem("caption", caption);
    }
    if (duration >= 0) {
        params.addQueryItem("duration", QString::number(duration));
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("voice", "voice", voice, params);

    // call api
    this->callApiTemplate("sendVoice", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendVideoNote
 * @param chatId
 * @param videoNote
 * @param length
 * @param duration
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendVideoNote(QVariant chatId, QVariant videoNote, int length, int duration, int replyToMessageId, TelegramFlags flags,
                                TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    if (length >= 0) {
        params.addQueryItem("length", QString::number(length));
    }
    if (duration >= 0) {
        params.addQueryItem("duration", QString::number(duration));
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // handle file
    QHttpMultiPart *multiPart = this->handleFile("video_note", "video_note", videoNote, params);

    // call api
    this->callApiTemplate("sendVideoNote", params, response, multiPart);
}

/**
 * @brief TelegramBot::sendLocation
 * @param chatId
 * @param latitude
 * @param longitude
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendLocation(QVariant chatId, double latitude, double longitude, int replyToMessageId, TelegramFlags flags,
                               TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("latitude", QString::number(latitude));
    params.addQueryItem("longitude", QString::number(longitude));
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendLocation", params, response);
}

/**
 * @brief TelegramBot::sendVenue
 * @param chatId
 * @param latitude
 * @param longitude
 * @param title
 * @param address
 * @param foursquareId
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendVenue(QVariant chatId, double latitude, double longitude, QString title, QString address, QString foursquareId,
                            int replyToMessageId, TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("latitude", QString::number(latitude));
    params.addQueryItem("longitude", QString::number(longitude));
    params.addQueryItem("title", title);
    params.addQueryItem("address", address);
    if (!foursquareId.isNull()) {
        params.addQueryItem("foursquare_id", foursquareId);
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendVenue", params, response);
}

/**
 * @brief TelegramBot::sendContact
 * @param chatId
 * @param phoneNumber
 * @param firstName
 * @param lastName
 * @param replyToMessageId
 * @param flags
 * @param keyboard
 * @param response
 */
void TelegramBot::sendContact(QVariant chatId, QString phoneNumber, QString firstName, QString lastName, int replyToMessageId,
                              TelegramFlags flags, TelegramKeyboardRequest keyboard, TelegramBotMessage *response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    QUrlQuery params;
    params.addQueryItem("chat_id", chatId.toString());
    params.addQueryItem("phone_number", phoneNumber);
    params.addQueryItem("first_name", firstName);
    if (!lastName.isNull()) {
        params.addQueryItem("last_name", lastName);
    }
    if (flags && TelegramFlags::DisableNotfication) {
        params.addQueryItem("disable_notification", "true");
    }
    if (replyToMessageId) {
        params.addQueryItem("reply_to_message_id", QString::number(replyToMessageId));
    }

    // handle reply markup
    this->hanldeReplyMarkup(params, flags, keyboard);

    // call api
    this->callApiTemplate("sendContact", params, response);
}

/**
 * @brief TelegramBot::startMessagePulling
 * @param timeout
 * @param limit
 * @param messageTypes
 * @param offset
 */
void TelegramBot::startMessagePulling(uint timeout, uint limit, TelegramPollMessageTypes messageTypes, long offset)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // remove webhook
    this->deleteWebhookResult();

    // build url params
    long effectiveOffset = offset ? offset : this->updateId;
    qDebug() << "TelegramBot::startMessagePulling - timeout" << timeout << "limit" << limit
             << "offset" << effectiveOffset << "messageTypes" << static_cast<int>(messageTypes);
    this->pullParams.clear();
    if (offset) {
        this->pullParams.addQueryItem("offset", QString::number(offset));
    } else if (this->updateId) {
        this->pullParams.addQueryItem("offset", QString::number(this->updateId));
    }
    this->pullParams.addQueryItem("limit", QString::number(limit));
    this->pullParams.addQueryItem("timeout", QString::number(timeout));

    // allowed updates
    QStringList allowedUpdates;
    if (static_cast<int>(messageTypes) > 0) {
        if (messageTypes && TelegramPollMessageTypes::Message) {
            allowedUpdates += "message";
        }
        if (messageTypes && TelegramPollMessageTypes::EditedMessage) {
            allowedUpdates += "edited_message";
        }
        if (messageTypes && TelegramPollMessageTypes::ChannelPost) {
            allowedUpdates += "channel_post";
        }
        if (messageTypes && TelegramPollMessageTypes::EditedChannelPost) {
            allowedUpdates += "edited_channel_post";
        }
        if (messageTypes && TelegramPollMessageTypes::InlineQuery) {
            allowedUpdates += "inline_query";
        }
        if (messageTypes && TelegramPollMessageTypes::ChoosenInlineQuery) {
            allowedUpdates += "chosen_inline_result";
        }
        if (messageTypes && TelegramPollMessageTypes::CallbackQuery) {
            allowedUpdates += "callback_query";
        }
    }
    if (!allowedUpdates.isEmpty()) {
        this->pullParams.addQueryItem("allowed_updates", "[\"" + allowedUpdates.join("\",\"") + "\"]");
    }

    // start pulling
    this->pull();
}

/**
 * @brief TelegramBot::stopMessagePulling
 * @param instantly
 */
void TelegramBot::stopMessagePulling(bool instantly)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    this->pullParams.clear();
    if (instantly && this->replyPull) {
        this->replyPull->abort();
    }
}

/**
 * @brief TelegramBot::pull
 */
void TelegramBot::pull()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // if we pull is disabled, exit
    if (this->pullParams.isEmpty()) {
        return;
    }

    //qDebug() << "TelegramBot::pull - issuing getUpdates with" << this->pullParams.toString();

    // cleanup
    if (this->replyPull) {
        this->replyPull->deleteLater();
    }

    // call api
    this->replyPull = this->callApi("getUpdates", this->pullParams, false);
    QObject::connect(this->replyPull, &QNetworkReply::finished, this, &TelegramBot::handlePullResponse);
}

/**
 * @brief TelegramBot::handlePullResponse
 */
void TelegramBot::handlePullResponse()
{
    if (!this->replyPull) {
        qDebug() << "TelegramBot::handlePullResponse - missing reply, scheduling next pull";
        QTimer::singleShot(1000, this, &TelegramBot::pull);
        return;
    }

    // remove update id from request
    this->pullParams.removeQueryItem("offset");

    QNetworkReply::NetworkError networkError = this->replyPull->error();
    if (networkError != QNetworkReply::NoError) {
        qDebug() << "TelegramBot::handlePullResponse - network error" << networkError
                 << this->replyPull->errorString();
        if (this->updateId) {
            this->pullParams.addQueryItem("offset", QString::number(this->updateId + 1));
        }
        QTimer::singleShot(1000, this, &TelegramBot::pull);
        return;
    }

    // parse response
    QByteArray data = this->replyPull->readAll();
    //qDebug() << "TelegramBot::handlePullResponse - received" << data.size() << "bytes";
    this->parseMessage(data);

    // add update id to request
    if (this->updateId) {
        this->pullParams.addQueryItem("offset", QString::number(this->updateId + 1));
    }

    // continue pulling
    this->pull();
}

/**
 * @brief TelegramBot::setHttpServerWebhook
 * @param listenPort
 * @param publicHost
 * @param publicPort
 * @param scheme
 * @param maxConnections
 * @param messageTypes
 * @return
 */
bool TelegramBot::setHttpServerWebhook(qint16 listenPort, QString publicHost, qint16 publicPort, QString scheme,
                                       int maxConnections, TelegramPollMessageTypes messageTypes)
{
    if (publicHost.isEmpty()) {
        qWarning() << "TelegramBot::setHttpServerWebhook - publicHost is empty";
        return false;
    }
    scheme = scheme.toLower();
    if (scheme != "http" && scheme != "https") {
        scheme = "https";
    }
    HttpServer *httpServer = nullptr;
    auto mark = [](const char *msg) {
        fprintf(stderr, "[WEBHOOK] %s\n", msg);
        fflush(stderr);
    };
    mark("A: entering setHttpServerWebhook");
    if (this->webHookWebServers.contains(listenPort)) {
        httpServer = this->webHookWebServers.find(listenPort).value();
    }
    // if no webserver exist, create it
    else {
        std::unique_ptr<HttpServer> scopedHttpServer = std::make_unique<HttpServer>();
        httpServer = scopedHttpServer.get();

        // permit only telegram connections
        QStringList cidrs = loadTelegramCidrs();
        for (const QString &cidr : cidrs) {
            httpServer->addWhiteListHostSubnet(cidr);
        }
        //Home
        httpServer->addWhiteListHostSubnet("192.168.178.0/24");
        // Docker bridge (Host/Proxy -> Container)
        httpServer->addWhiteListHostSubnet("172.17.0.0/16");

        // start listener
        if (!httpServer->listen(QHostAddress::Any, listenPort)) {
            qCritical() << "Webhook listen failed:" << httpServer->errorString();
            EXIT_FAILED("TelegramBot::setHttpServerWebhook - Cannot listen on port %i, webhook installation failed...", listenPort)
        }
        qInfo() << "Webhook listening on" << httpServer->serverAddress() << httpServer->serverPort();
        mark("D: after listen()");

        // everything is okay, so register http server
        this->webHookWebServers.insert(listenPort, scopedHttpServer.release());
    }

    mark("B: after listener start");

    QString host = publicHost;
    QString path = this->m_webhookPath.trimmed();
    if (path.startsWith('/')) {
        path.remove(0, 1);
    }

    if (path.isEmpty()) {
        path = this->apiKey;
    }

    // add rewrite rule
    httpServer->addRewriteRule(host, "/" + path, {this, &TelegramBot::handleServerWebhookResponse});
    mark("C: after route registration");

    // build server webhook request
    QUrlQuery query;
    QString webhookUrl = scheme + "://" + host;
    bool portIncluded = (scheme == "https" && publicPort != 443) || (scheme == "http" && publicPort != 80);
    if (portIncluded) {
        webhookUrl += ":" + QString::number(publicPort);
    }
    webhookUrl += "/" + path;
    query.addQueryItem("url", webhookUrl);
    if (maxConnections) {
        query.addQueryItem("max_connections", QString::number(maxConnections));
    }

    // allowed updates
    QStringList allowedUpdates;
    if (static_cast<int>(messageTypes) > 0) {
        if (messageTypes && TelegramPollMessageTypes::Message) {
            allowedUpdates += "message";
        }
        if (messageTypes && TelegramPollMessageTypes::EditedMessage) {
            allowedUpdates += "edited_message";
        }
        if (messageTypes && TelegramPollMessageTypes::ChannelPost) {
            allowedUpdates += "channel_post";
        }
        if (messageTypes && TelegramPollMessageTypes::EditedChannelPost) {
            allowedUpdates += "edited_channel_post";
        }
        if (messageTypes && TelegramPollMessageTypes::InlineQuery) {
            allowedUpdates += "inline_query";
        }
        if (messageTypes && TelegramPollMessageTypes::ChoosenInlineQuery) {
            allowedUpdates += "chosen_inline_result";
        }
        if (messageTypes && TelegramPollMessageTypes::CallbackQuery) {
            allowedUpdates += "callback_query";
        }
    }
    if (!allowedUpdates.isEmpty()) {
        query.addQueryItem("allowed_updates", "[\"" + allowedUpdates.join("\",\"") + "\"]");
    }

    // call api
    QJsonObject apiResponse = this->callApiJson("setWebhook", query);
    bool result = apiResponse.value("result").toBool();
    if (!result) {
        QString description = apiResponse.value("description").toString();
        qWarning() << "Telegram setWebhook failed:" << description;
        mark("E: after callApiJson (failed)");
    } else {
        mark("E: after callApiJson (succeeded)");
    }
    mark("F: leaving setHttpServerWebhook");
    return result;
}

/**
 * @brief TelegramBot::deleteWebhook
 */
void TelegramBot::deleteWebhook()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    this->callApi("deleteWebhook");
}

/**
 * @brief TelegramBot::deleteWebhookResult
 * @return
 */
TelegramBotOperationResult TelegramBot::deleteWebhookResult()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    return TelegramBotOperationResult(this->callApiJson("deleteWebhook"));
}

/**
 * @brief TelegramBot::getWebhookInfo
 * @return
 */
TelegramBotWebHookInfo TelegramBot::getWebhookInfo()
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    return TelegramBotWebHookInfo(this->callApiJson("getWebhookInfo").value("result").toObject());
}

void TelegramBot::setWebhookPath(const QString &path)
{
    QString normalized = path.trimmed();
    if (normalized.startsWith('/')) {
        normalized.remove(0, 1);
    }

    if (normalized.isEmpty()) {
        normalized = this->apiKey;
    }

    this->m_webhookPath = normalized;
}

QString TelegramBot::webhookPath() const
{
    return this->m_webhookPath.isEmpty() ? this->apiKey : this->m_webhookPath;
}

/**
 * @brief TelegramBot::messageRouterRegister
 * @param startWith
 * @param delegate
 * @param type
 */
void TelegramBot::messageRouterRegister(QString startWith, QDelegate<bool(TelegramBotUpdate)> delegate, TelegramBotMessageType type)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // save message route
    this->messageRoutes.append(new MessageRoute{
        type,
        startWith,
        delegate
    });
}

/**
 * @brief TelegramBot::parseMessage
 * @param data
 * @param singleMessage
 */
void TelegramBot::parseMessage(QByteArray &data, bool singleMessage)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // parse result
    QJsonParseError jError;
    QJsonObject oUpdate = QJsonDocument::fromJson(data, &jError).object();

    // handle parse error
    if (jError.error != QJsonParseError::NoError) {
        qDebug("TelegramBot::parseMessage - Parse Error: %s", qPrintable(jError.errorString()));
        return;
    }

    if (!singleMessage && !JsonHelper::jsonPathGet(oUpdate, "ok").toBool()) {
        return (void)qDebug("TelegramBot::parseMessage - Receive Error: %i - %s",
                            JsonHelper::jsonPathGet(oUpdate, "error_code").toInt(),
                            qPrintable(JsonHelper::jsonPathGet(oUpdate, "description").toString()));
    }

    // loop results
    for (QJsonValue result : singleMessage ? QJsonArray({oUpdate}) : oUpdate.value("result").toArray()) {
        QJsonObject update = result.toObject();

        // parse result
        TelegramBotUpdate updateMessage(new TelegramBotUpdatePrivate);
        updateMessage->fromJson(update);

        // save update id
        this->updateId = updateMessage->updateId;

        // send Message to outside world
        emit this->newMessage(updateMessage);

        // call message routes
        QString routeData = updateMessage->inlineQuery ? updateMessage->inlineQuery->query
                            : updateMessage->chosenInlineResult ? updateMessage->chosenInlineResult->query
                            : updateMessage->callbackQuery ? updateMessage->callbackQuery->data
                            : updateMessage->message ? updateMessage->message->text : QString();
        if (routeData.isNull()) {
            continue;
        }
        for (auto itrRoute = this->messageRoutes.begin(); itrRoute != this->messageRoutes.end(); itrRoute++) {
            MessageRoute *route = *itrRoute;
            if (route->type && updateMessage->type != updateMessage->type) {
                continue;
            }
            if (!routeData.startsWith(route->startWith)) {
                continue;
            }
            if (!route->delegate.invoke(updateMessage).first()) {
                break;
            }
        }
    }
}

/**
 * @brief TelegramBot::handleServerWebhookResponse
 * @param request
 * @param response
 */
void TelegramBot::handleServerWebhookResponse(HttpServerRequest request, HttpServerResponse response)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // parse response
    QByteArray payload = request->content;
    qInfo() << "[WEBHOOK] handler received" << payload.size() << "bytes from" << request->headers.value("Host");
    QMetaObject::invokeMethod(this, [this, payload]() mutable {
        QByteArray data = payload;
        this->parseMessage(data, true);
    }, Qt::QueuedConnection);

    // reply to server with status OK
    response->status = HttpServerResponsePrivate::OK;
}

void TelegramBot::checkWebhookHealth()
{
    if (this->m_webhookFallbackTriggered) {
        return;
    }

    TelegramBotWebHookInfo info = this->getWebhookInfo();
    qDebug() << "TelegramBot::checkWebhookHealth - webhook" << sanitizeWebhookUrl(info.url)
             << "ipAddress" << info.ipAddress
             << "lastErrorDate" << info.lastErrorDate
             << "lastErrorMessage" << info.lastErrorMessage
             << "pendingUpdates" << info.pendingUpdateCount
             << "maxConnections" << info.maxConnections
             << "allowedUpdates" << info.allowedUpdates;

    if (info.url.isEmpty()) {
        return;
    }

    if (info.lastErrorMessage.isEmpty()) {
        return;
    }

    if (info.lastErrorDate <= this->m_lastWebhookErrorDate) {
        return;
    }

    this->m_lastWebhookErrorDate = info.lastErrorDate;
    qDebug() << "TelegramBot::checkWebhookHealth - detected webhook error, switching to polling";
    this->deleteWebhook();
    this->startMessagePulling();
    this->m_webhookFallbackTriggered = true;
    if (this->m_webhookHealthTimer) {
        this->m_webhookHealthTimer->stop();
    }
}

template<typename T>
typename std::enable_if<std::is_base_of<TelegramBotObject, T>::value>::type TelegramBot::callApiTemplate(QString method, QUrlQuery params,
                                                                                                         T *response,
                                                                                                         QHttpMultiPart *multiPart)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // if no response was provided, just call the api
    if (!response) {
        // qDebug()<<Q_FUNC_INFO <<" "<<"response";
        return (void)this->callApi(method, params, true, multiPart);
    }

    // get result and parse it
    QJsonObject object = QJsonObject(this->callApiJson(method, params, multiPart)).value("result").toObject();
    // qDebug()<<Q_FUNC_INFO <<" "<<object;
    response->fromJson(object);
}

template<typename T>
typename std::enable_if<!std::is_base_of<TelegramBotObject, T>::value>::type TelegramBot::callApiTemplate(QString method, QUrlQuery params,
                                                                                                          T *response,
                                                                                                          QHttpMultiPart *multiPart)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // if no response was provided, just call the api
    if (!response) {
        return (void)this->callApi(method, params, true, multiPart);
    }

    // get result and (if possible) convert it to T
    QVariant result = QJsonObject(this->callApiJson(method, params, multiPart)).value("result").toVariant();
    if (result.canConvert<T>()) {
        *response = result.value<T>();
    }
}

/**
 * @brief TelegramBot::callApi
 * @param method
 * @param params
 * @param deleteOnFinish
 * @param multiPart
 * @return
 */
QNetworkReply *TelegramBot::callApi(QString method, QUrlQuery params, bool deleteOnFinish, QHttpMultiPart *multiPart)
{
    // build url
    // qDebug()<<Q_FUNC_INFO <<" "<<method;
    QUrl url(QString("https://api.telegram.org/bot%1/%2").arg(this->apiKey, method));
    url.setQuery(params);

    // execute
    QNetworkRequest request(url);
    QNetworkReply *reply = multiPart ? this->aManager.post(request, multiPart) : this->aManager.get(request);
    if (multiPart) {
        multiPart->setParent(reply);
    }
    if (deleteOnFinish) {
        QObject::connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
    }
    return reply;
}

/**
 * @brief TelegramBot::callApiJson
 * @param method
 * @param params
 * @param multiPart
 * @return
 */
QJsonObject TelegramBot::callApiJson(QString method, QUrlQuery params, QHttpMultiPart *multiPart)
{
    // exec request
    // qDebug()<<Q_FUNC_INFO <<" "<<method;
    QNetworkReply *reply = this->callApi(method, params, true, multiPart);

    // wait async for answer
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // parse answer
    return QJsonDocument::fromJson(reply->readAll()).object();
}

/**
 * @brief TelegramBot::createUploadFile
 * @param name
 * @param fileName
 * @param content
 * @param detectMimeType
 * @param multiPart
 * @return
 */
QHttpMultiPart *TelegramBot::createUploadFile(QString name, QString fileName, QByteArray &content, bool detectMimeType,
                                              QHttpMultiPart *multiPart)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // construct instance if not provided
    if (!multiPart) {
        multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    }

    // append multipart multipart
    QHttpPart contentPart;
    contentPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                          QString("form-data; name=\"%1\"; filename=\"%2\"").arg(name, fileName));
    contentPart.setHeader(QNetworkRequest::ContentTypeHeader,
                          detectMimeType ? QMimeDatabase().mimeTypeForData(content).name() : QString("application/octet-stream"));
    contentPart.setBody(content);
    multiPart->append(contentPart);

    return multiPart;
}

/**
 * @brief TelegramBot::hanldeReplyMarkup
 * @param params
 * @param flags
 * @param keyboard
 */
void TelegramBot::hanldeReplyMarkup(QUrlQuery &params, TelegramFlags flags, TelegramKeyboardRequest &keyboard)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // handle types
    QString replyMarkup;
    if (flags && TelegramFlags::ForceReply) {
        replyMarkup = "{\"force_reply\":true";
        if (flags && TelegramFlags::Selective) {
            replyMarkup += ",\"selective\":true";
        }
        replyMarkup += "}";
    } else if (flags && TelegramFlags::ReplyKeyboardRemove) {
        replyMarkup = "{\"remove_keyboard\":true";
        if (flags && TelegramFlags::Selective) {
            replyMarkup += ",\"selective\":true";
        }
        replyMarkup += "}";
    }
    // build keyboard
    else if (!keyboard.isEmpty()) {
        QString keyboardContent = "[";
        bool firstRow = true;
        for (QList<TelegramBotKeyboardButtonRequest> &row : keyboard) {
            if (!firstRow) {
                keyboardContent += ",";
            }
            keyboardContent += "[";
            bool firstColumn = true;
            for (TelegramBotKeyboardButtonRequest &column : row) {
                keyboardContent += QString("%1{\"text\":\"%2\"").arg(firstColumn ? "" : ",", column.text);
                if (flags && TelegramFlags::ReplyKeyboardMarkup) {
                    if (column.requestContact) {
                        keyboardContent += ",\"request_contact\":true";
                    }
                    if (column.requestLocation) {
                        keyboardContent += ",\"request_location\":true";
                    }
                } else {
                    if (!column.url.isEmpty()) {
                        keyboardContent += QString(",\"url\":\"%1\"").arg(column.url);
                    }
                    if (!column.callbackData.isEmpty()) {
                        keyboardContent += QString(",\"callback_data\":\"%1\"").arg(column.callbackData);
                    }
                    if (!column.switchInlineQuery.isEmpty()) {
                        keyboardContent += QString(",\"switch_inline_query\":\"%1\"").arg(column.switchInlineQuery);
                    }
                    if (!column.switchInlineQueryCurrentChat.isEmpty()) {
                        keyboardContent += QString(",\"switch_inline_query_current_chat\":\"%1\"").arg(column.switchInlineQueryCurrentChat);
                    }
                }
                keyboardContent += "}";
                firstColumn = false;
            }
            keyboardContent += "]";
            firstRow = false;
        }
        keyboardContent += "]";

        if (flags && TelegramFlags::ReplyKeyboardMarkup) {
            replyMarkup += "{\"keyboard\":" + keyboardContent;
            if (flags && TelegramFlags::ResizeKeyboard) {
                replyMarkup += ",\"resize_keyboard\":true";
            }
            if (flags && TelegramFlags::OneTimeKeyboard) {
                replyMarkup += ",\"one_time_keyboard\":true";
            }
            if (flags && TelegramFlags::Selective) {
                replyMarkup += ",\"selective\":true";
            }
        } else {
            replyMarkup += "{\"inline_keyboard\":" + keyboardContent;
        }
        replyMarkup += "}";
    }
    if (!replyMarkup.isEmpty()) {
        params.addQueryItem("reply_markup", replyMarkup);
    }
}

/**
 * @brief TelegramBot::handleFile
 * @param fieldName
 * @param file
 * @param params
 * @param multiPart
 * @return
 */
QHttpMultiPart *TelegramBot::handleFile(QString filename, QString fieldName, QVariant file, QUrlQuery &params, QHttpMultiPart *multiPart)
{
    // qDebug()<<Q_FUNC_INFO <<" ";
    // handle content
    if (file.typeId() == QMetaType::QByteArray) {
        QByteArray content = file.value<QByteArray>();
        multiPart = this->createUploadFile(fieldName, filename, content, true, multiPart);
    }
    // handle url
    else if (file.typeId() == QMetaType::QString) {
        QUrl url = QUrl::fromUserInput(file.toString());

        // upload the local file to telegram
        if (url.isLocalFile() || url.isRelative()) {
            QFile fFile(file.toString());
            if (!fFile.open(QFile::ReadOnly)) {
                qWarning("TelegramBot::handleFile - Cannot open file \"%s\"", qPrintable(file.toString()));
                return multiPart;
            }
            QByteArray content = fFile.readAll();
            QFileInfo fInfo(fFile);
            multiPart = this->createUploadFile(fieldName, fInfo.fileName(), content, true, multiPart);
        }
        // we have a link given, so just set it
        else {
            params.addQueryItem(fieldName, file.toString());
        }
    }
    // otherwise we interpret it as telegram file id
    else {
        params.addQueryItem(fieldName, file.toString());
    }

    return multiPart;
}
