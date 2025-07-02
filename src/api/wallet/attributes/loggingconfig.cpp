#include "loggingconfig.h"

/**
 * @brief LoggingConfig::LoggingConfig
 */
LoggingConfig::LoggingConfig() :
    m_logToStdout(false),
    m_logToFile(false),
    m_logFileAppend(false),
    m_logMaxSize(0),
    m_logMaxFiles(0),
    m_tuiRunning(false)
{
}

/**
 * @brief LoggingConfig::logToStdout
 * @return
 */
bool LoggingConfig::logToStdout() const
{
    return m_logToStdout;
}

/**
 * @brief LoggingConfig::setLogToStdout
 * @param value
 */
void LoggingConfig::setLogToStdout(bool value)
{
    m_logToStdout = value;
}

/**
 * @brief LoggingConfig::stdoutLogLevel
 * @return
 */
QString LoggingConfig::stdoutLogLevel() const
{
    return m_stdoutLogLevel;
}

/**
 * @brief LoggingConfig::setStdoutLogLevel
 * @param value
 */
void LoggingConfig::setStdoutLogLevel(const QString &value)
{
    m_stdoutLogLevel = value;
}

/**
 * @brief LoggingConfig::logToFile
 * @return
 */
bool LoggingConfig::logToFile() const
{
    return m_logToFile;
}

/**
 * @brief LoggingConfig::setLogToFile
 * @param value
 */
void LoggingConfig::setLogToFile(bool value)
{
    m_logToFile = value;
}

/**
 * @brief LoggingConfig::fileLogLevel
 * @return
 */
QString LoggingConfig::fileLogLevel() const
{
    return m_fileLogLevel;
}

/**
 * @brief LoggingConfig::setFileLogLevel
 * @param value
 */
void LoggingConfig::setFileLogLevel(const QString &value)
{
    m_fileLogLevel = value;
}

/**
 * @brief LoggingConfig::logFilePath
 * @return
 */
QString LoggingConfig::logFilePath() const
{
    return m_logFilePath;
}

/**
 * @brief LoggingConfig::setLogFilePath
 * @param value
 */
void LoggingConfig::setLogFilePath(const QString &value)
{
    m_logFilePath = value;
}

/**
 * @brief LoggingConfig::logFileAppend
 * @return
 */
bool LoggingConfig::logFileAppend() const
{
    return m_logFileAppend;
}

/**
 * @brief LoggingConfig::setLogFileAppend
 * @param value
 */
void LoggingConfig::setLogFileAppend(bool value)
{
    m_logFileAppend = value;
}

/**
 * @brief LoggingConfig::logMaxSize
 * @return
 */
int LoggingConfig::logMaxSize() const
{
    return m_logMaxSize;
}

/**
 * @brief LoggingConfig::setLogMaxSize
 * @param value
 */
void LoggingConfig::setLogMaxSize(int value)
{
    m_logMaxSize = value;
}

/**
 * @brief LoggingConfig::logMaxFiles
 * @return
 */
int LoggingConfig::logMaxFiles() const
{
    return m_logMaxFiles;
}

/**
 * @brief LoggingConfig::setLogMaxFiles
 * @param value
 */
void LoggingConfig::setLogMaxFiles(int value)
{
    m_logMaxFiles = value;
}

/**
 * @brief LoggingConfig::tuiRunning
 * @return
 */
bool LoggingConfig::tuiRunning() const
{
    return m_tuiRunning;
}

/**
 * @brief LoggingConfig::setTuiRunning
 * @param value
 */
void LoggingConfig::setTuiRunning(bool value)
{
    m_tuiRunning = value;
}

/**
 * @brief LoggingConfig::fromJson
 * @param obj
 */
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

/**
 * @brief LoggingConfig::toJson
 * @return
 */
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
