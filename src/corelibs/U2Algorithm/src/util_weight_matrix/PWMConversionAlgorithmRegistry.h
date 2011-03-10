#ifndef _U2_PWM_CONVERSION_ALGORITHM_REGISTRY_H_
#define _U2_PWM_CONVERSION_ALGORITHM_REGISTRY_H_

#include "PWMConversionAlgorithm.h"

#include <QtCore/QStringList>

namespace U2 {

class PWMConversionAlgorithm;

class U2ALGORITHM_EXPORT PWMConversionAlgorithmRegistry : public QObject {
    Q_OBJECT

public:
    PWMConversionAlgorithmRegistry(QObject* p = NULL);

    ~PWMConversionAlgorithmRegistry();

    PWMConversionAlgorithmFactory* getAlgorithmFactory(const QString& algoId);

    void addAlgorithm(PWMConversionAlgorithmFactory* algo);

    QStringList getAlgorithmIds() const ;

    QList<PWMConversionAlgorithmFactory*> getAlgorithmFactories() const {return algorithms.values();}

private:
    QMap<QString , PWMConversionAlgorithmFactory*> algorithms;
};

}//namespace

#endif
