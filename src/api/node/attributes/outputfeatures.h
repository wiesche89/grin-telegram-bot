#ifndef OUTPUTFEATURES_H
#define OUTPUTFEATURES_H

#include <QObject>

class OutputFeatures
{
    Q_GADGET

public:
    enum Feature {
        Plain = 0,
        Coinbase = 1
    };
    Q_ENUM(Feature)
};

#endif // OUTPUTFEATURES_H
