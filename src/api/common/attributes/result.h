#ifndef RESULT_H
#define RESULT_H

#include "error.h"

template<typename T>
class Result
{
public:
    /**
     * @brief Result
     * @param value
     */
    Result(const T &value) :
        m_value(value),
        m_hasError(false)
    {
    }

    /**
     * @brief Result
     * @param error
     */
    Result(const Error &error) :
        m_error(error),
        m_hasError(true)
    {
    }

    /**
     * @brief hasError
     * @return
     */
    bool hasError() const
    {
        return m_hasError;
    }

    /**
     * @brief error
     * @return
     */
    Error error() const
    {
        return m_error;
    }

    /**
     * @brief value
     * @return
     */
    T value() const
    {
        return m_value;
    }

    /**
     * @brief unwrapOrLog
     * @param out
     * @param funcInfo function identifier that is always logged on failure
     * @param info optional additional context to append to the log
     * @return
     */
    bool unwrapOrLog(T &out, const char *funcInfo, const QString &info = QString()) const
    {
        if (m_hasError) {
            QString msg = m_error.message;
            QString prefix = funcInfo ? QString::fromUtf8(funcInfo) : QString();
            if (info.isEmpty()) {
                qDebug() << prefix << "- Error:" << msg;
            } else {
                qDebug() << prefix << "(" << info << ")" << "- Error:" << msg;
            }
            return false;
        }
        out = m_value;
        return true;
    }

    /**
     * @brief errorMessage
     * @return
     */
    QString errorMessage() const
    {
        QString msg = m_error.message;
        return m_hasError ? msg : QString();
    }

private:
    T m_value{};
    Error m_error;
    bool m_hasError;
};

#endif // RESULT_H
