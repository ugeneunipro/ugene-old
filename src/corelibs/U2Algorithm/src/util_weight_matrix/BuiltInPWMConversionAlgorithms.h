#ifndef _U2_BUILT_IN_PWM_CONVERSION_ALGORITHMS_H_
#define _U2_BUILT_IN_PWM_CONVERSION_ALGORITHMS_H_

#include <U2Core/global.h>

#include <QtCore/QString>

namespace U2 {

class U2ALGORITHM_EXPORT BuiltInPWMConversionAlgorithms {
public:
    static const QString BVH_ALGO;
    static const QString LOD_ALGO;
    static const QString MCH_ALGO;
    static const QString NLG_ALGO;
};

}//namespace

#endif
