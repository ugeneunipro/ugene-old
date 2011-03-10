#ifndef _U2_REMOTE_MACHINE_SCAN_DIALOG_IMPL_H_
#define _U2_REMOTE_MACHINE_SCAN_DIALOG_IMPL_H_

#include <QtCore/QTimer>
#include <QtGui/QDialog>

#include <U2Remote/RemoteMachine.h>
#include <ui/ui_RemoteMachineScanDialog.h>

namespace U2 {

typedef QList< RemoteMachineSettings * > RemoteMachineScanDialogModel;

class RemoteMachineScanDialogImpl : public QDialog, public Ui::RemoteMachineScanDialog {
    Q_OBJECT
public:
    static const int SCAN_UPDATE_TIME = 500; /* 0.5 seconds */
    
public:
    RemoteMachineScanDialogImpl();
    ~RemoteMachineScanDialogImpl();
    
    RemoteMachineScanDialogModel getModel() const;
    
private:
    void addMachines( const QList< RemoteMachineSettings* > newMachines );
    void addMachine( RemoteMachineSettings * machine );
    void addNextMachineToTable( RemoteMachineSettings * );
    void cleanup();
    bool hasSameMachineInTheList( RemoteMachineSettings* ) const;
    void resizeTable();
    
private slots:
    void sl_cancelPushButtonClicked();
    void sl_okPushButtonClicked();
    void sl_startScan( const QString & protoId );
    void sl_updatePushButtonClicked();
    
private:
    RemoteMachineScanDialogModel        model; /* machines by order as in the table */
    QList< RemoteMachineScanner * >     runningScanners;
    QTimer                              updateTimer;
    
}; // RemoteMachineScanDialogImpl

} // U2

#endif // _U2_REMOTE_MACHINE_SCAN_DIALOG_IMPL_H_
