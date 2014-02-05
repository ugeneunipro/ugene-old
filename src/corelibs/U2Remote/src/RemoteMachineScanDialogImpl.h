/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
