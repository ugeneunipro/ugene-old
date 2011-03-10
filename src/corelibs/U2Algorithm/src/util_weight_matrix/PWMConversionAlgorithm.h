#ifndef _U2_PWM_CONVERSION_ALGORITHM_H_
#define _U2_PWM_CONVERSION_ALGORITHM_H_

#include <U2Core/global.h>

#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>

namespace U2 {

class PWMConversionAlgorithm;

class U2ALGORITHM_EXPORT PWMConversionAlgorithmFactory : public QObject {
    Q_OBJECT
public:
    PWMConversionAlgorithmFactory(const QString& algoId, QObject* p = NULL);
    
    virtual PWMConversionAlgorithm* createAlgorithm(QObject* parent = NULL) = 0;
    
    QString getId() const {return algorithmId;}

    virtual QString getDescription() const = 0;

    virtual QString getName() const = 0;

private:
    QString algorithmId;

};

class U2ALGORITHM_EXPORT PWMConversionAlgorithm : public QObject {
    Q_OBJECT
public:
    PWMConversionAlgorithm(PWMConversionAlgorithmFactory* factory, QObject* p = NULL);

    virtual PWMatrix convert (const PFMatrix& matrix) = 0;

    QString getId() const {return factory->getId();}
private:
    PWMConversionAlgorithmFactory* factory;

};

}//namespace

#endif
