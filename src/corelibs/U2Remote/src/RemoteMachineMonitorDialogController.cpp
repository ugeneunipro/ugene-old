#include "RemoteMachineMonitorDialogController.h"
#include "RemoteMachineMonitorDialogImpl.h"

#include <QtGui/QMessageBox>

/* TRANSLATOR U2::RemoteMachineMonitorDialogImpl */

namespace U2 {

RemoteMachineSettings* RemoteMachineMonitorDialogController::selectRemoteMachine(
    RemoteMachineMonitor* monitor, bool runTaskMode /*= false */ )
{
    RemoteMachineMonitorDialogImpl dlg( QApplication::activeWindow(), monitor, runTaskMode);

    int ret = dlg.exec();
    if(ret == QDialog::Rejected) {
        return NULL;
    }
    assert(ret == QDialog::Accepted);

    RemoteMachineSettings *rms = dlg.getSelectedMachine();
    if (runTaskMode && !rms) {
        QMessageBox::critical(QApplication::activeWindow(), 
            RemoteMachineMonitorDialogImpl::tr("Selecting machines error!"), 
            RemoteMachineMonitorDialogImpl::tr("You didn't select a machine to run remote task!"));
    }
    return rms;
}
}//ns