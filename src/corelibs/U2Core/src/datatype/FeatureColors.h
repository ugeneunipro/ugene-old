#ifndef _U2_FEATURE_COLORS_H_
#define _U2_FEATURE_COLORS_H_

#include <U2Core/global.h>

#include <QtGui/QColor>

namespace U2 {

class U2CORE_EXPORT FeatureColors {
public:
    static QColor genLightColor(const QString& name);

};

} //namespace

#endif
