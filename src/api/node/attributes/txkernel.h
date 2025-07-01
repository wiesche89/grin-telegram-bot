#ifndef TXKERNEL_H
#define TXKERNEL_H

#include <QString>
#include <QJsonObject>

class TxKernel
{
public:
    TxKernel();

    // Getter
    QString features() const;
    QString excess() const;
    QString excessSig() const;

    // Setter
    void setFeatures(const QString &features);
    void setExcess(const QString &excess);
    void setExcessSig(const QString &excessSig);

    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_features;
    QString m_excess;
    QString m_excessSig;
};

#endif // TXKERNEL_H
