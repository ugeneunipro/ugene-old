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

#ifndef _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_
#define _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_

#include <U2Remote/RemoteMachine.h>
#include <U2Remote/ProtocolUI.h>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif

#include <ui/ui_RemoteMachineSettingsDialog.h>

namespace U2 {

class RemoteMachineSettingsDialog : public QDialog, public Ui::RemoteMachineSettingsDialog {
    Q_OBJECT
public:
    RemoteMachineSettingsDialog(QWidget* parent, const RemoteMachineSettingsPtr& settings = RemoteMachineSettingsPtr());
    ~RemoteMachineSettingsDialog();
    
    RemoteMachineSettingsPtr getMachineSettings() const;

private slots:
    void sl_okPushButtonClicked();
    
private:
    void showErrorLabel(const QString& error);
    void createMachineSettings();
    RemoteMachineSettingsPtr        machineSettings;
    QString                         protoId;
    ProtocolUI*                     currentUi;
    
}; // RemoteMachineSettingsDialog

} // U2

#endif // _U2_REMOTE_MACHINE_SETTINGS_DIALOG_H_
