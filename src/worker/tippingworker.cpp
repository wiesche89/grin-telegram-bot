#include "tippingworker.h"

/**
 * @brief TippingWorker::TippingWorker
 * @param bot
 * @param settings
 */
TippingWorker::TippingWorker(TelegramBot *bot, QSettings *settings)
{

}

/**
 * @brief TippingWorker::init
 * @return
 */
bool TippingWorker::init()
{
    bool success = true;

    return success;
}

/**
 * @brief TippingWorker::onMessage
 * @param update
 */
void TippingWorker::onMessage(TelegramBotUpdate update)
{

}

/**
 * @brief TippingWorker::handleSlateS1State
 * @param slate
 * @param message
 * @return
 */
Result<QString> TippingWorker::handleSlateS1State(Slate slate, TelegramBotMessage message)
{


    return QString();
}

/**
 * @brief TippingWorker::handleSlateI1State
 * @param slate
 * @param message
 * @return
 */
Result<QString> TippingWorker::handleSlateI1State(Slate slate, TelegramBotMessage message)
{

    return QString();
}

/**
 * @brief TippingWorker::sendUserMessage
 * @param message
 * @param content
 * @param plain
 */
void TippingWorker::sendUserMessage(TelegramBotMessage message, QString content, bool plain)
{


}
