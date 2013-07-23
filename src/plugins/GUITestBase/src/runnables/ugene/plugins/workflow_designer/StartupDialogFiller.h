#ifndef STARTUPDIALOGFILLER_H
#define STARTUPDIALOGFILLER_H

#include "GTUtilsDialog.h"

namespace U2 {

class StartupDialogFiller : public Filler
{
public:
    StartupDialogFiller(U2OpStatus &_os):Filler(_os,"StartupDialog"){}
    void run();
};
}
#endif // STARTUPDIALOGFILLER_H
