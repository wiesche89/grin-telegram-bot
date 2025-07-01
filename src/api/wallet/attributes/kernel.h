#ifndef KERNEL_H
#define KERNEL_H

#include <QString>
#include <QJsonObject>

class Kernel
{
public:
    Kernel();
    Kernel(const QString &excess, const QString &excessSig, const QString &features);

    QJsonObject toJson() const;
    static Kernel fromJson(const QJsonObject &obj);

    QString excess() const;
    void setExcess(const QString &excess);

    QString excessSig() const;
    void setExcessSig(const QString &excessSig);

    QString features() const;
    void setFeatures(const QString &features);

private:
    QString m_excess;
    QString m_excessSig;
    QString m_features;
};

#endif // KERNEL_H
