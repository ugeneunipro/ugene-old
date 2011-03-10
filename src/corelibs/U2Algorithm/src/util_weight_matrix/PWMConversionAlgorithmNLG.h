#ifndef _U2_PWM_CONVERSION_ALGORITHM_NLG_H_
#define _U2_PWM_CONVERSION_ALGORITHM_NLG_H_

#include <U2Core/global.h>

#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>

#include "PWMConversionAlgorithm.h"
#include "BuiltInPWMConversionAlgorithms.h"

namespace U2 {

class U2ALGORITHM_EXPORT PWMConversionAlgorithmFactoryNLG : public PWMConversionAlgorithmFactory {
    Q_OBJECT
public:
    PWMConversionAlgorithmFactoryNLG(QObject* p = NULL);
    
    virtual PWMConversionAlgorithm* createAlgorithm(QObject* parent);
    
    virtual QString getDescription() const;

    virtual QString getName() const;

};

class U2ALGORITHM_EXPORT PWMConversionAlgorithmNLG : public PWMConversionAlgorithm {
    Q_OBJECT
public:
    PWMConversionAlgorithmNLG(PWMConversionAlgorithmFactory* factory, QObject* p = NULL);

    virtual PWMatrix convert(const PFMatrix& matrix);

};

}//namespace

#endif
