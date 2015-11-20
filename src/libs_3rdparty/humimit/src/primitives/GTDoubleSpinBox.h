#ifndef _HI_GT_DOUBLE_SPINBOX_H
#define _HI_GT_DOUBLE_SPINBOX_H

#include "primitives/GTSpinBox.h"
#include "GTGlobals.h"

#include <QSpinBox>

namespace HI {

class HI_EXPORT GTDoubleSpinbox
{
public:
    static void setValue(U2::U2OpStatus& os, QDoubleSpinBox *spinBox, double v, GTGlobals::UseMethod useMethod = GTGlobals::UseMouse);
    static void checkLimits(U2::U2OpStatus &os, QDoubleSpinBox *spinBox, double min, double max);
};
}
#endif // GTDOUBLESPINBOX_H
