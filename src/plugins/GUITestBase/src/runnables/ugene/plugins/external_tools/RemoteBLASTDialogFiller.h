#ifndef REMOTEBLASTDIALOGFILLER_H
#define REMOTEBLASTDIALOGFILLER_H

#include "GTUtilsDialog.h"
#include "api/GTFileDialog.h"

namespace U2 {

class RemoteBLASTDialogFiller : public Filler {
public:
    RemoteBLASTDialogFiller(U2OpStatus &os) :
        Filler(os, "RemoteBLASTDialog"){}

virtual void run();

};

}

#endif // REMOTEBLASTDIALOGFILLER_H
