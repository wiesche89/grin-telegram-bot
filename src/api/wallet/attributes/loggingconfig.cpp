#include "loggingconfig.h"

LoggingConfig::LoggingConfig() :
    m_logToStdout(false),
    m_logToFile(false),
    m_logFileAppend(false),
    m_logMaxSize(0),
    m_logMaxFiles(0),
    m_tuiRunning(false)
{
}

bool LoggingConfig::logToStdout() const
{
    return m_logToStdout;
}

void LoggingConfig::setLogToStdout(bool value)
{
    m_logToStdout = value;
}

QString LoggingConfig::stdoutLogLevel() const
{
    return m_stdoutLogLevel;
}

void LoggingConfig::setStdoutLogLevel(const QString &value)
{
    m_stdoutLogLevel = value;
}

bool LoggingConfig::logToFile() const
{
    return m_logToFile;
}

void LoggingConfig::setLogToFile(bool value)
{
    m_logToFile = value;
}

QString LoggingConfig::fileLogLevel() const
{
    return m_fileLogLevel;
}

void LoggingConfig::setFileLogLevel(const QString &value)
{
    m_fileLogLevel = value;
}

QString LoggingConfig::logFilePath() const
{
    return m_logFilePath;
}

void LoggingConfig::setLogFilePath(const QString &value)
{
    m_logFilePath = value;
}

bool LoggingConfig::logFileAppend() const
{
    return m_logFileAppend;
}

void LoggingConfig::setLogFileAppend(bool value)
{
    m_logFileAppend = value;
}

int LoggingConfig::logMaxSize() const
{
    return m_logMaxSize;
}

void LoggingConfig::setLogMaxSize(int value)
{
    m_logMaxSize = value;
}

int LoggingConfig::logMaxFiles() const
{
    return m_logMaxFiles;
}

void LoggingConfig::setLogMaxFiles(int value)
{
    m_logMaxFiles = value;
}

bool LoggingConfig::tuiRunning() const
{
    return m_tuiRunning;
}

void LoggingConfig::setTuiRunning(bool value)
{
    m_tuiRunning = value;
}

void LoggingConfig::fromJson(const QJsonObject &obj)
{
    m_logToStdout = obj.value("log_to_stdout").toBool();
    m_stdoutLogLevel = obj.value("stdout_log_level").toString();
    m_logToFile = obj.value("log_to_file").toBool();
    m_fileLogLevel = obj.value("file_log_level").toString();
    m_logFilePath = obj.value("log_file_path").toString();
    m_logFileAppend = obj.value("log_file_append").toBool();
    m_logMaxSize = obj.value("log_max_size").toInt();
    m_logMaxFiles = obj.value("log_max_files").toInt();
    m_tuiRunning = obj.value("tui_running").toBool();
}

QJsonObject LoggingConfig::toJson() const
{
    QJsonObject obj;
    obj["log_to_stdout"] = m_logToStdout;
    obj["stdout_log_level"] = m_stdoutLogLevel;
    obj["log_to_file"] = m_logToFile;
    obj["file_log_level"] = m_fileLogLevel;
    obj["log_file_path"] = m_logFilePath;
    obj["log_file_append"] = m_logFileAppend;
    obj["log_max_size"] = m_logMaxSize;
    obj["log_max_files"] = m_logMaxFiles;
    obj["tui_running"] = m_tuiRunning;
    return obj;
}
