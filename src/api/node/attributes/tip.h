#ifndef TIP_H
#define TIP_H

#include <QString>
#include <cstdint>
#include <QJsonObject>

class Tip
{
public:
    Tip();
    Tip(uint64_t height, const QString &lastBlockPushed, const QString &prevBlockToLast, uint64_t totalDifficulty);

    // Getter
    uint64_t height() const;
    QString lastBlockPushed() const;
    QString prevBlockToLast() const;
    uint64_t totalDifficulty() const;

    // Setter
    void setHeight(uint64_t height);
    void setLastBlockPushed(const QString &lastBlockPushed);
    void setPrevBlockToLast(const QString &prevBlockToLast);
    void setTotalDifficulty(uint64_t totalDifficulty);

    // JSON
    static Tip fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    uint64_t m_height;
    QString m_lastBlockPushed;
    QString m_prevBlockToLast;
    uint64_t m_totalDifficulty;
};

#endif // TIP_H
