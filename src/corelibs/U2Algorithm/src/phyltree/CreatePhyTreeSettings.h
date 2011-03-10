#ifndef  _U2_CREATE_PHYTREE_SETTINGS_H
#define  _U2_CREATE_PHYTREE_SETTINGS_H

#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <QString>
#include <QList>

namespace U2{

class U2ALGORITHM_EXPORT CreatePhyTreeSettings{
public:
    CreatePhyTreeSettings();
    QString algorithmId;
    QString matrixId;
    GUrl    fileUrl;
    bool useGammaDistributionRates;
    double alphaFactor;
    double ttRatio; // transition transversion ratio
    bool bootstrap;
    int replicates;
};

}

#endif 
