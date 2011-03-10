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
    RemoteMachineSettings * settings;
    QString                 hostname;
    bool                    isSelected;
    
    RemoteMachineItemInfo( RemoteMachineSettings * s) 
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
    RemoteMachineSettings* getSelectedMachine() const;
    
private:
    bool addMachine( RemoteMachineSettings * machine, bool ping );
    QTreeWidgetItem * addItemToTheView( RemoteMachineItemInfo & item );
    int topLevelItemsSelectedNum() const;
    int getSelectedTopLevelRow() const;
    bool hasSameMachineInTheView( RemoteMachineSettings * machine ) const;
    void enableItem( QTreeWidgetItem * item, bool enable );
    bool removeDialogItemAt( int row ); /* returns if item was successfully removed */
    bool checkCredentials(RemoteMachineSettings* settings);
    void checkBoxStateChanged( QCheckBox * cb, bool enable );
    RemoteMachineItemInfo& getItemInfo(QTreeWidgetItem* item);
    void pingMachine( RemoteMachineSettings * settings, QTreeWidgetItem * item );
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
    QMap< RemoteMachineSettings *, QTreeWidgetItem * > pingingItems; /* ping sent to machines with this items */
    
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
