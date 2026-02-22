#ifndef LOGGING_LOGGINGHANDLER_H
#define LOGGING_LOGGINGHANDLER_H

#include <QFile>
#include <QMutex>
#include <QString>

class LoggingHandler
{
public:
    explicit LoggingHandler(const QString &logFilePath = QString());
    ~LoggingHandler();

    void log(const QString &message);

    static QString defaultLogFilePath();
    static void installMessageHandler(LoggingHandler *handler);

private:
    bool ensureFileOpen();

    QString m_logFilePath;
    QFile m_logFile;
    QMutex m_mutex;
};

#endif // LOGGING_LOGGINGHANDLER_H
