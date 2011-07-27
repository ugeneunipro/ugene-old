/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef _U2_REMOTE_MACHINE_MONITOR_DIALOG_IMPL_H_
#define _U2_REMOTE_MACHINE_MONITOR_DIALOG_IMPL_H_

#include <U2Remote/RemoteMachine.h>
#include <U2Remote/RemoteMachineMonitor.h>

#include <ui/ui_RemoteMachineMonitorDialog.h>

#include <QtGui/QMenu>
#include <QtGui/QCheckBox>

namespace U2 {

class RemoteTask;
class RetrievePublicMachinesTask;
class UpdateActiveTasks;
class RemoteMachinesMonitor;

struct RemoteMachineItemInfo {
    RemoteMachineSettingsPtr    settings;
    QString                     hostname;
    bool                        isSelected;
    
    RemoteMachineItemInfo(const RemoteMachineSettingsPtr& s) 
        : settings( s ) {
        assert( NULL != settings );
    }
    RemoteMachineItemInfo() 
        : settings( NULL ), isSelected(false)  {
    }
    
}; // RemoteMachineMonitorItemInfo

class RemoteMachineMonitorDialogImpl : public QDialog, Ui::RemoteMachineMonitorDialog {
    Q_OBJECT
private:
    static const QString OK_BUTTON_RUN;
    static const int CHECKBOX_SIZE_HINT_MAGIC_NUMBER = 2;
    static const QString SAVE_SETTINGS_FILE_DOMAIN;
    
public:
    RemoteMachineMonitorDialogImpl( QWidget * p, RemoteMachineMonitor* monitor,
                                    bool runTaskMode = false );
    
    QList< RemoteMachineItemInfo > getModel() const;
    RemoteMachineSettingsPtr getSelectedMachine() const;
    
private:
    bool addMachineSettings( const RemoteMachineSettingsPtr& machine, bool ping );
    QTreeWidgetItem * addItemToTheView( RemoteMachineItemInfo & item );
    int topLevelItemsSelectedNum() const;
    int getSelectedTopLevelRow() const;
    bool hasSameMachineInTheView( const RemoteMachineSettingsPtr& machine ) const;
    void enableItem( QTreeWidgetItem * item, bool enable );
    bool removeDialogItemAt( int row ); /* returns if item was successfully removed */
    bool checkCredentials(const RemoteMachineSettingsPtr& settings);
    void checkBoxStateChanged( QCheckBox * cb, bool enable );
    RemoteMachineItemInfo& getItemInfo(QTreeWidgetItem* item);
    void pingMachine( const RemoteMachineSettingsPtr& settings, QTreeWidgetItem * item );
    void resizeTreeWidget();
    void initMachineActionsMenu();
    void updateState();
    
private slots:
    void sl_okPushButtonClicked();
    void sl_cancelPushButtonClicked();
    void sl_addPushButtonClicked();
    void sl_removePushButtonClicked();
    void sl_modifyPushButtonClicked();
    void sl_selectionChanged();
    void sl_retrieveInfoTaskStateChanged();
    void sl_pingPushButtonClicked();
    void sl_getPublicMachinesButtonClicked();
    void sl_getPublicMachinesTaskStateChanged();
    void sl_machinesTreeMenuRequested(const QPoint&);
    void sl_showUserTasksButtonClicked();
    void sl_saveMachine();
    
private:
    QMenu* machineActionsMenu;
    int currentlySelectedItemIndex;
    QList< RemoteMachineItemInfo > machinesItemsByOrder;
    QMap< RemoteMachineSettingsPtr, QTreeWidgetItem * > pingingItems; /* ping sent to machines with this items */
    
    /* not static because QApplication must be constructed before any graphical object */
    const QPixmap PING_YES;
    const QPixmap PING_NO;
    const QPixmap PING_WAIT_FOR_RESPONSE;
    const QPixmap PING_QUESTION;
    
    RemoteMachineMonitor* rmm; 
    RetrievePublicMachinesTask * getPublicMachinesTask;
    
}; // RemoteMachineMonitorDialogImpl

} // U2

#endif // _U2_REMOTE_MACHINE_MONITOR_DIALOG_IMPL_H_
