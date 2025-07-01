#ifndef TXKERNELPRINTABLE_H
#define TXKERNELPRINTABLE_H

#include <QString>
#include <QJsonObject>

class TxKernelPrintable
{
public:
    TxKernelPrintable();

    // Getter
    QString features() const;
    uint8_t feeShift() const;
    quint64 fee() const;
    quint64 lockHeight() const;
    QString excess() const;
    QString excessSig() const;

    // Setter
    void setFeatures(const QString &features);
    void setFeeShift(uint8_t feeShift);
    void setFee(quint64 fee);
    void setLockHeight(quint64 lockHeight);
    void setExcess(const QString &excess);
    void setExcessSig(const QString &excessSig);

    // JSON-Parsing
    void fromJson(const QJsonObject &json);
    QJsonObject toJson() const;

private:
    QString m_features;
    uint8_t m_feeShift;
    quint64 m_fee;
    quint64 m_lockHeight;
    QString m_excess;
    QString m_excessSig;
};

#endif // TXKERNELPRINTABLE_H
