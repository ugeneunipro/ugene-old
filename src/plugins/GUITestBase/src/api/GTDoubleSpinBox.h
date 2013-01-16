#ifndef GTDOUBLESPINBOX_H
#define GTDOUBLESPINBOX_H

#include "GTSpinBox.h"
#include "api/GTGlobals.h"
#include <QtGui/QSpinBox>

namespace U2 {

class GTDoubleSpinbox
{
public:
    static void setValue(U2OpStatus& os, QDoubleSpinBox *spinBox, double v, GTGlobals::UseMethod useMethod = GTGlobals::UseMouse);
    GTDoubleSpinbox();
};
}
#endif // GTDOUBLESPINBOX_H
