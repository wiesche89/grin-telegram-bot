#ifndef TIP_H
#define TIP_H

#include <QString>
#include <QJsonObject>

class Tip
{
public:
    Tip();
    Tip(quint64 height, const QString &lastBlockPushed, const QString &prevBlockToLast, quint64 totalDifficulty);

    // Getter
    quint64 height() const;
    QString lastBlockPushed() const;
    QString prevBlockToLast() const;
    quint64 totalDifficulty() const;

    // Setter
    void setHeight(quint64 height);
    void setLastBlockPushed(const QString &lastBlockPushed);
    void setPrevBlockToLast(const QString &prevBlockToLast);
    void setTotalDifficulty(quint64 totalDifficulty);

    // JSON
    static Tip fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    quint64 m_height;
    QString m_lastBlockPushed;
    QString m_prevBlockToLast;
    quint64 m_totalDifficulty;
};

#endif // TIP_H
