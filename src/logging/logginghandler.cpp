#include "logginghandler.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QIODevice>
#include <QMessageLogContext>
#include <QMutexLocker>
#include <QTextStream>
#include <QtGlobal>
#include <cstdio>
#include <cstdlib>

namespace {
LoggingHandler *g_loggingHandler = nullptr;

const char *messageLevelText(QtMsgType type)
{
    switch (type) {
        case QtDebugMsg:
            return "DEBUG";
        case QtInfoMsg:
            return "INFO";
        case QtWarningMsg:
            return "WARNING";
        case QtCriticalMsg:
            return "CRITICAL";
        case QtFatalMsg:
            return "FATAL";
    }
    return "UNKNOWN";
}

QString contextDescription(const QMessageLogContext &context)
{
    QString file = context.file ? QString::fromUtf8(context.file) : QStringLiteral("?");
    QString function = context.function ? QString::fromUtf8(context.function) : QStringLiteral("?");
    QString line = QString::number(context.line);
    return QString("%1:%2 (%3)").arg(file, line, function);
}

void qtMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString composed = QString("[%1] %2 - %3")
                            .arg(messageLevelText(type))
                            .arg(msg)
                            .arg(contextDescription(context));

    if (g_loggingHandler) {
        g_loggingHandler->log(composed);
    }

    QByteArray formatted = qFormatLogMessage(type, context, msg).toLocal8Bit();
    fprintf(stderr, "%s\n", formatted.constData());
    fflush(stderr);

    if (type == QtFatalMsg) {
        abort();
    }
}
}
#include <QtGlobal>

LoggingHandler::LoggingHandler(const QString &logFilePath)
    : m_logFilePath(logFilePath.isEmpty() ? defaultLogFilePath() : logFilePath),
      m_logFile(m_logFilePath)
{
    ensureFileOpen();
}

LoggingHandler::~LoggingHandler()
{
    if (m_logFile.isOpen()) {
        m_logFile.flush();
        m_logFile.close();
    }
}

QString LoggingHandler::defaultLogFilePath()
{
    QDir base(QCoreApplication::applicationDirPath());
    base.mkpath("logs");
    return base.filePath("logs/grin-telegram-bot.log");
}

void LoggingHandler::log(const QString &message)
{
    QMutexLocker locker(&m_mutex);
    if (!ensureFileOpen()) {
        return;
    }

    QTextStream stream(&m_logFile);
    stream << QDateTime::currentDateTime().toString(Qt::ISODateWithMs) << ' ' << message << '\n';
    stream.flush();
}

bool LoggingHandler::ensureFileOpen()
{
    if (m_logFile.isOpen()) {
        return true;
    }

    QFileInfo info(m_logFile);
    QDir dir = info.dir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        qWarning() << "LoggingHandler: failed to open log file" << m_logFilePath;
        return false;
    }

    return true;
}

void LoggingHandler::installMessageHandler(LoggingHandler *handler)
{
    g_loggingHandler = handler;
    qInstallMessageHandler(qtMessageHandler);
}
