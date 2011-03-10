#include "RemoteMachineScanDialogImpl.h"
#include "RemoteMachineSettingsDialog.h"
#include "RemoteMachineMonitorDialogImpl.h"

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Misc/DialogUtils.h>
#include <U2Gui/GUIUtils.h>
#include <U2Misc/AuthenticationDialog.h>
#include <U2Remote/RemoteMachineTasks.h>
#include <U2Remote/RemoteMachineMonitor.h>
#include <U2Gui/LogView.h>
#include <U2Core/LogCache.h>

#include <QtGui/QLabel>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include <memory>

namespace U2 {

const QString RemoteMachineMonitorDialogImpl::OK_BUTTON_RUN = RemoteMachineMonitorDialogImpl::tr( "Run" );
const QString RemoteMachineMonitorDialogImpl::SAVE_SETTINGS_FILE_DOMAIN = "rservice";


static LogFilter prepareLogFilter() {
    LogFilter res;
    res.filters.append(LogFilterItem(ULOG_CAT_REMOTE_SERVICE, LogLevel_DETAILS));
    return res;
}


RemoteMachineMonitorDialogImpl::RemoteMachineMonitorDialogImpl( QWidget * p, RemoteMachineMonitor* monitor, 
                                                               bool runTaskMode )
: QDialog( p ), PING_YES( ":core/images/remote_machine_ping_yes.png" ), PING_NO( ":core/images/remote_machine_ping_no.png" ), 
PING_WAIT_FOR_RESPONSE( ":core/images/remote_machine_ping_waiting_response.png" ), PING_QUESTION(":core/images/question.png"),
rmm(monitor), getPublicMachinesTask( NULL ) {
    setupUi( this );
    
    // add log-view widget
    QVBoxLayout* logLayout = new QVBoxLayout();
    logViewHolder->setLayout(logLayout);
    LogViewWidget* logView = new LogViewWidget(prepareLogFilter());
    logView->setSearchBoxMode(LogViewSearchBox_Hidden);
    logLayout->addWidget(logView);
    
    
    currentlySelectedItemIndex = -1;
    
    assert(rmm != NULL);

    QList< RemoteMachineMonitorItem > monitorItems = rmm->getRemoteMachineMonitorItems();

    int sz = monitorItems.size();
    for( int i = 0; i < sz; ++i ) {
        RemoteMachineMonitorItem item = monitorItems.at( i );
        addMachine( item.machine, false );
    }
    rsLog.details(tr("Found %1 remote machine records").arg(sz));
    
    connect( okPushButton, SIGNAL( clicked() ), SLOT( sl_okPushButtonClicked() ) );
    connect( cancelPushButton, SIGNAL( clicked() ), SLOT( sl_cancelPushButtonClicked() ) );
    connect( addPushButton, SIGNAL( clicked() ), SLOT( sl_addPushButtonClicked() ) );
    connect( removePushButton, SIGNAL( clicked() ), SLOT( sl_removePushButtonClicked() ) );
    connect( modifyPushButton, SIGNAL( clicked() ), SLOT( sl_modifyPushButtonClicked() ) );
    connect( showTasksButton, SIGNAL(clicked()), SLOT(sl_showUserTasksButtonClicked()) );
    connect( machinesTreeWidget, SIGNAL( itemSelectionChanged() ), SLOT( sl_selectionChanged() ) );
    connect( pingPushButton, SIGNAL( clicked() ), SLOT( sl_pingPushButtonClicked() ) );
    connect( getPublicMachinesButton, SIGNAL( clicked() ), SLOT( sl_getPublicMachinesButtonClicked() ) );
    
    okPushButton->setDefault( true );
    
    QHeaderView * header = machinesTreeWidget->header();
    header->setClickable( false );
    header->setStretchLastSection( false );
    header->setResizeMode( 1, QHeaderView::Stretch );
    
    if( runTaskMode ) {
        okPushButton->setText( OK_BUTTON_RUN );
    }
    
    updateState();
    initMachineActionsMenu();
}

void RemoteMachineMonitorDialogImpl::initMachineActionsMenu() {
    machinesTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(machinesTreeWidget, SIGNAL(customContextMenuRequested(const QPoint &)), SLOT(sl_machinesTreeMenuRequested(const QPoint&)));
    
    machineActionsMenu = new QMenu(this);
    
    QAction * removeMachineAction = machineActionsMenu->addAction(removePushButton->text());
    connect(removeMachineAction, SIGNAL(triggered()), SLOT(sl_removePushButtonClicked()));

    QAction * modifyMachineAction = machineActionsMenu->addAction(modifyPushButton->text());
    connect(modifyMachineAction, SIGNAL(triggered()), SLOT(sl_modifyPushButtonClicked()));
    
    QAction * pingMachineAction = machineActionsMenu->addAction(pingPushButton->text());
    connect(pingMachineAction, SIGNAL(triggered()), SLOT(sl_pingPushButtonClicked()));

    QAction * saveMachineAction = machineActionsMenu->addAction(tr("Save machine..."));;
    connect(saveMachineAction, SIGNAL(triggered()), SLOT(sl_saveMachine()));
}

bool RemoteMachineMonitorDialogImpl::addMachine( RemoteMachineSettings * settings, bool ping ) {
    assert( NULL != settings );
    if( hasSameMachineInTheView( settings ) ) {
        rsLog.error(tr( "Can't add %1 machine. The machine is already registered" ).arg( settings->getName() ) );
        return false;
    }
    
    RemoteMachineItemInfo item( settings);
    
    machinesItemsByOrder << item;
    QTreeWidgetItem * widgetItem = addItemToTheView( item );
    

    assert( NULL != widgetItem );
    if (ping) {
        pingMachine( settings, widgetItem );
    } else {
        widgetItem->setIcon(2, PING_QUESTION);
        widgetItem->setIcon(3, PING_QUESTION);

    }
    
    rmm->addMachine(settings, false);

    return true;
}

QTreeWidgetItem * RemoteMachineMonitorDialogImpl::addItemToTheView( RemoteMachineItemInfo & itemInfo ) {
    assert( NULL != itemInfo.settings );
    QStringList strings;
    QTreeWidgetItem * widgetItem = new QTreeWidgetItem( machinesTreeWidget );

    widgetItem->setText(0, itemInfo.settings->getName());
    widgetItem->setText(1, itemInfo.settings->getProtocolId());
    widgetItem->setTextAlignment(2, Qt::AlignCenter);
    
    machinesTreeWidget->addTopLevelItem( widgetItem );
    resizeTreeWidget();
    
    return widgetItem;
}

bool RemoteMachineMonitorDialogImpl::hasSameMachineInTheView( RemoteMachineSettings * suspect ) const {
    int sz = machinesItemsByOrder.size();
    int i = 0;
    for( ; i < sz; ++i ) {
        RemoteMachineItemInfo item = machinesItemsByOrder.at( i );
        if( *item.settings == *suspect ) {
            return true;
        }
    }
    return false;
}

QList< RemoteMachineItemInfo > RemoteMachineMonitorDialogImpl::getModel() const {
    return machinesItemsByOrder;
}


void RemoteMachineMonitorDialogImpl::sl_okPushButtonClicked() {
    if (okPushButton->text() == OK_BUTTON_RUN) {
        RemoteMachineSettings* s = getSelectedMachine(); 
        checkCredentials(s);
    }

    accept();
}

void RemoteMachineMonitorDialogImpl::sl_cancelPushButtonClicked() {
    reject();
}


void RemoteMachineMonitorDialogImpl::sl_addPushButtonClicked() {
    RemoteMachineSettingsDialog settingsDlg(this);
    
    QList< ProtocolInfo* > protoInfos = AppContext::getProtocolInfoRegistry()->getProtocolInfos();
    if (protoInfos.size() < 1) {
        QMessageBox::information(this, tr("Add remote macnine"), tr("No protocols for distributed computing are found.\nPlease check your plugin list."));
        return;
    }
    int rc = settingsDlg.exec();
    if( QDialog::Rejected == rc ) {
        return;
    }
    assert( QDialog::Accepted == rc );
    
    RemoteMachineSettings * newMachine = settingsDlg.getMachineSettings();
    if( NULL == newMachine ) {
        return;
    }
    if( !addMachine( newMachine, true ) ) {
        delete newMachine;
    }
}

void RemoteMachineMonitorDialogImpl::sl_modifyPushButtonClicked() {
    assert( 1 == topLevelItemsSelectedNum() );
    int row = getSelectedTopLevelRow();
    assert( 0 <= row && row < machinesItemsByOrder.size() );
        
    RemoteMachineSettingsDialog settingsDlg( this, machinesItemsByOrder.at( row ).settings );
    int rc = settingsDlg.exec();
    if( QDialog::Rejected == rc ) {
        return;
    }
    
    RemoteMachineSettings * newMachine = settingsDlg.getMachineSettings();
    if( NULL == newMachine ) {
        return;
    }
    
    removeDialogItemAt( row );
    addMachine( newMachine, true );
}

void RemoteMachineMonitorDialogImpl::sl_removePushButtonClicked() {
    assert( 1 == topLevelItemsSelectedNum() );
    bool ok = removeDialogItemAt( getSelectedTopLevelRow() );
    if( !ok ) {
        QString msg = tr( "Cannot delete machine that is waiting for response" );
        rsLog.error(msg);
        QMessageBox::critical( this, tr( "Error!" ), msg );
        return;
    }
    
}

bool RemoteMachineMonitorDialogImpl::removeDialogItemAt( int row ) {
    assert( 0 <= row && row < machinesItemsByOrder.size() );
    RemoteMachineItemInfo & itemToRemove = machinesItemsByOrder[row];
    std::auto_ptr<QTreeWidgetItem> treeItemToRemove ( machinesTreeWidget->takeTopLevelItem( row ) );
    rmm->removeMachine(itemToRemove.settings);
    machinesItemsByOrder.removeAt( row );
    
    return true;
}

void RemoteMachineMonitorDialogImpl::sl_selectionChanged() {
    if (machinesTreeWidget->currentItem() != NULL ) {
        currentlySelectedItemIndex = getSelectedTopLevelRow();
    } else {
        currentlySelectedItemIndex = -1;
    }

    updateState();
}

int RemoteMachineMonitorDialogImpl::topLevelItemsSelectedNum() const {
    return machinesTreeWidget->selectedItems().size();
}

int RemoteMachineMonitorDialogImpl::getSelectedTopLevelRow() const {
    assert( 1 == topLevelItemsSelectedNum() ); 
    QList< QTreeWidgetItem* > selection = machinesTreeWidget->selectedItems();
    assert( !selection.isEmpty() );
    return machinesTreeWidget->indexOfTopLevelItem( selection.first() );
}


void RemoteMachineMonitorDialogImpl::enableItem( QTreeWidgetItem * item, bool enable ) {
    assert( NULL != item );
    GUIUtils::setMutedLnF(item, !enable, true);
}

void RemoteMachineMonitorDialogImpl::sl_retrieveInfoTaskStateChanged() {
    RetrieveRemoteMachineInfoTask * retrieveInfoTask = qobject_cast< RetrieveRemoteMachineInfoTask* >( sender() );
    assert( NULL != retrieveInfoTask );
    if( Task::State_Finished != retrieveInfoTask->getState() ) {
        return;
    }
    
    RemoteMachineSettings* machineSettings = retrieveInfoTask->getMachineSettings();
    assert( NULL != machineSettings );
    QTreeWidgetItem * treeItem = pingingItems.value( machineSettings );
    
    pingingItems.remove( machineSettings );
    
    int row = machinesTreeWidget->indexOfTopLevelItem( treeItem );
    if( -1 == row ) {
        return; /* item was deleted from the table */
    }
    
    RemoteMachineItemInfo & itemInfo = machinesItemsByOrder[row];
    
    bool pingOk = retrieveInfoTask->isPingOk();
    bool authOk = !retrieveInfoTask->hasErrors();

    treeItem->setIcon( 2, pingOk ? PING_YES : PING_NO);
    treeItem->setIcon( 3, authOk ? PING_YES : PING_NO);


    if ( !authOk  ) {
        rsLog.error( tr( "Test connection for machine %1 finished with error: '%2'" ).
            arg( itemInfo.settings->getName() ).arg( retrieveInfoTask->getError() ) );
    }
    
    itemInfo.hostname = retrieveInfoTask->getHostName();
    treeItem->setText(1, itemInfo.hostname );
    resizeTreeWidget();

    enableItem(treeItem, authOk);
    
    updateState();

}


void RemoteMachineMonitorDialogImpl::sl_pingPushButtonClicked() {
    assert( 1 == topLevelItemsSelectedNum() );
    int row = getSelectedTopLevelRow();
    pingMachine( machinesItemsByOrder.at( row ).settings, machinesTreeWidget->topLevelItem( row ) );
    updateState();
}

void RemoteMachineMonitorDialogImpl::pingMachine( RemoteMachineSettings * settings, QTreeWidgetItem * item ) {
    assert( NULL != settings && NULL != item );
    
    if (!checkCredentials(settings)) {
        return;
    }
    
    if( pingingItems.values().contains( item ) ) {
        rsLog.info(tr("Ping task is already active for machine: %1" ).arg(item->text(0)));
        return;
    }
    
    pingingItems.insert( settings, item );
    item->setIcon(2, PING_WAIT_FOR_RESPONSE);
    item->setIcon(3, PING_WAIT_FOR_RESPONSE);

    RetrieveRemoteMachineInfoTask * retrieveInfoTask = new RetrieveRemoteMachineInfoTask( settings );
    connect( retrieveInfoTask, SIGNAL( si_stateChanged() ), SLOT( sl_retrieveInfoTaskStateChanged() ) );
    AppContext::getTaskScheduler()->registerTopLevelTask( retrieveInfoTask );
}


void RemoteMachineMonitorDialogImpl::resizeTreeWidget() {
    QHeaderView * header = machinesTreeWidget->header();
    header->resizeSections( QHeaderView::ResizeToContents );
}

void RemoteMachineMonitorDialogImpl::sl_getPublicMachinesButtonClicked() {
    if( NULL != getPublicMachinesTask ) {
        rsLog.details(tr("Public machines request is already sent" ) );
        return;
    }
    
    getPublicMachinesTask = new RetrievePublicMachinesTask();
    connect( getPublicMachinesTask, SIGNAL( si_stateChanged() ), SLOT( sl_getPublicMachinesTaskStateChanged() ) );
    AppContext::getTaskScheduler()->registerTopLevelTask( getPublicMachinesTask );
    getPublicMachinesButton->setEnabled(false);
}

void RemoteMachineMonitorDialogImpl::sl_getPublicMachinesTaskStateChanged() {
    assert( NULL != getPublicMachinesTask && getPublicMachinesTask == sender() );
    if( Task::State_Finished != getPublicMachinesTask->getState() ) {
        return;
    }
        
    QList< RemoteMachineSettings* > newMachines = getPublicMachinesTask->takePublicMachines();

    if( getPublicMachinesTask->hasErrors()) {
        QMessageBox::critical(this, tr("Info"), tr("Error during remote machines request: %1").arg(getPublicMachinesTask->getError()));
    } else if ( newMachines.isEmpty() ) {
        QMessageBox::information(this, tr("Info"), tr("No public machines found"));
    } else {
        foreach( RemoteMachineSettings * machine, newMachines ) {
            if( !addMachine( machine, false ) ) {
                delete machine; /* it already exists in the monitor list */
            }
        }
    }
    getPublicMachinesTask = NULL;
    getPublicMachinesButton->setEnabled(true);

    
}

void RemoteMachineMonitorDialogImpl::sl_machinesTreeMenuRequested(const QPoint& p) {
    QTreeWidgetItem * item = machinesTreeWidget->itemAt(p);
    if( item == NULL || item->parent() != NULL ) {
        return;
    }
    machineActionsMenu->exec(QCursor::pos());
}

void RemoteMachineMonitorDialogImpl::sl_saveMachine() {
    assert(topLevelItemsSelectedNum() == 1);
    RemoteMachineItemInfo & item = machinesItemsByOrder[getSelectedTopLevelRow()];
    QString filename;
    LastOpenDirHelper dirHelper(SAVE_SETTINGS_FILE_DOMAIN);
    dirHelper.url = filename = QFileDialog::getSaveFileName( this, tr("Select a file to save"), dirHelper.dir);
    AppContext::getTaskScheduler()->registerTopLevelTask(new SaveRemoteMachineSettings(item.settings, filename));
}

void RemoteMachineMonitorDialogImpl::updateState()
{
   
    if (okPushButton->text() == OK_BUTTON_RUN) {
        okPushButton->setEnabled(topLevelItemsSelectedNum() > 0);
    }
    
    bool itemSelected = machinesTreeWidget->selectedItems().size() > 0;
    
    bool pinging = false;
    foreach(QTreeWidgetItem* item, pingingItems.values()) {
        if (item->isSelected()) {
            pinging = true;
            break;
        }
    }
    
    bool available = !pinging && itemSelected;

    pingPushButton->setEnabled(available);
    removePushButton->setEnabled(available );
    modifyPushButton->setEnabled(available );
    showTasksButton->setEnabled(available);

}

RemoteMachineSettings* RemoteMachineMonitorDialogImpl::getSelectedMachine() const {
    if ( currentlySelectedItemIndex < 0 || currentlySelectedItemIndex >= machinesItemsByOrder.size() ) {
        return NULL;
    }

    const RemoteMachineItemInfo& info = machinesItemsByOrder.at(currentlySelectedItemIndex);
    return info.settings;
}

void RemoteMachineMonitorDialogImpl::sl_showUserTasksButtonClicked() {
    ProtocolInfoRegistry * pir = AppContext::getProtocolInfoRegistry();
    assert( NULL != pir );
    QList< ProtocolInfo* > protoInfos = pir->getProtocolInfos();
    assert(protoInfos.size() > 0);
    ProtocolInfo* pi = protoInfos.first();

    int row = getSelectedTopLevelRow();
    RemoteMachineSettings* settings = machinesItemsByOrder.at(row).settings;
    if (!checkCredentials(settings)) {
        return;
    }
    if (settings->usesGuestAccount()) {
        QMessageBox::warning(this, tr("User Tasks"), 
            tr("<html><br>Unable to show user task statistics for guest account. \
               <br>Please register on <a href=http://ugene-service.com >ugene-service.com</a></html>"));
        return;
    }

    std::auto_ptr<QDialog> dlg(pi->getProtocolUI()->createUserTasksDialog(settings, this));
    dlg->exec();
}

bool RemoteMachineMonitorDialogImpl::checkCredentials( RemoteMachineSettings* settings ) {
    UserCredentials* credentials = settings->getUserCredentials();
    if (credentials == NULL) {
        AuthDialog dlg(this);
        int rc = dlg.exec();
        if ( QDialog::Rejected == rc ) {
            return false;
        }  
        settings->setupCredentials(dlg.getUserName(), dlg.getPasswd(), dlg.rememberAuthData());
    }
    return true;
}


} // U2
