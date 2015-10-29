#ifndef GTDOUBLESPINBOX_H
#define GTDOUBLESPINBOX_H

#include "primitives/GTSpinBox.h"
#include "GTGlobals.h"

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QSpinBox>
#else
#include <QtWidgets/QSpinBox>
#endif

namespace U2 {
using namespace HI;

class GTDoubleSpinbox
{
public:
    static void setValue(U2OpStatus& os, QDoubleSpinBox *spinBox, double v, GTGlobals::UseMethod useMethod = GTGlobals::UseMouse);
    static void checkLimits(U2OpStatus &os, QDoubleSpinBox *spinBox, double min, double max);
};
}
#endif // GTDOUBLESPINBOX_H
