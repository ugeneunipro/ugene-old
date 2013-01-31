/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "RemoteMachineMonitorDialogController.h"
#include "RemoteMachineMonitorDialogImpl.h"

#include <QtGui/QMessageBox>

/* TRANSLATOR U2::RemoteMachineMonitorDialogImpl */

namespace U2 {

RemoteMachineSettingsPtr RemoteMachineMonitorDialogController::selectRemoteMachine(
    RemoteMachineMonitor* monitor, bool runTaskMode /*= false */ )
{
    RemoteMachineMonitorDialogImpl dlg( QApplication::activeWindow(), monitor, runTaskMode);

    int ret = dlg.exec();
    if(ret == QDialog::Rejected) {
        return RemoteMachineSettingsPtr();
    }
    assert(ret == QDialog::Accepted);

    RemoteMachineSettingsPtr rms = dlg.getSelectedMachine();
    if (runTaskMode && !rms) {
        QMessageBox::critical(QApplication::activeWindow(), 
            RemoteMachineMonitorDialogImpl::tr("Selecting machines error!"), 
            RemoteMachineMonitorDialogImpl::tr("You didn't select a machine to run remote task!"));
    }
    return rms;
}
}//ns