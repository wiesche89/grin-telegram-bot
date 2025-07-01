#include "com.h"

/**
 * @brief Com::fromJson
 * @param obj
 * @return
 */
QString Com::c() const
{
    return m_c;
}

/**
 * @brief Com::setC
 * @param newC
 */
void Com::setC(const QString &newC)
{
    m_c = newC;
}

/**
 * @brief Com::f
 * @return
 */
int Com::f() const
{
    return m_f;
}

/**
 * @brief Com::setF
 * @param newF
 */
void Com::setF(int newF)
{
    m_f = newF;
}

/**
 * @brief Com::p
 * @return
 */
QString Com::p() const
{
    return m_p;
}

/**
 * @brief Com::setP
 * @param newP
 */
void Com::setP(const QString &newP)
{
    m_p = newP;
}

/**
 * @brief Com::fromJson
 * @param obj
 * @return
 */
Com Com::fromJson(const QJsonObject &obj)
{
    Com com;
    com.setC(obj.value("c").toString());
    if (obj.contains("f")) {
        com.setF(obj.value("f").toInt());
    }
    if (obj.contains("p")) {
        com.setP(obj.value("p").toString());
    }
    return com;
}

/**
 * @brief Com::toJson
 * @return
 */
QJsonObject Com::toJson() const
{
    QJsonObject obj;
    obj["c"] = m_c;
    if (m_f != 0) {
        obj["f"] = m_f;
    }
    if (!m_p.isEmpty()) {
        obj["p"] = m_p;
    }
    return obj;
}
