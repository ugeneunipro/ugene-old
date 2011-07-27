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

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include <U2Core/AppContext.h>
#include <U2Misc/DialogUtils.h>

#include "RemoteServiceMachine.h"
#include "RemoteServiceUtilTasks.h"
#include "RemoteTasksDialog.h"


namespace U2 {

RemoteTasksDialog::RemoteTasksDialog(const RemoteServiceSettingsPtr& settings,QWidget* parent ) 
: QDialog(parent), getInfoTask(NULL), fetchResultTask(NULL), deleteRemoteDataTask(NULL), machine(NULL) 
{
    setupUi(this);
    
    
    machine.reset(new RemoteServiceMachine(settings));
    urlLabel->setText(settings->getName());
    
    connect(this, SIGNAL(finished(int)), SLOT(sl_onDialogClosed()));
    connect(tasksTreeWidget, SIGNAL(itemSelectionChanged()),SLOT(sl_onSelectionChanged()) );
    connect(refreshPushButton, SIGNAL(clicked()), SLOT(sl_onRefreshButtonClicked()));
    connect(fetchPushButton, SIGNAL(clicked()), SLOT(sl_onFetchButtonClicked()));
    connect(removePushButton, SIGNAL(clicked()), SLOT(sl_onRemoveButtonClicked()));

    refresh();
    
    updateState();
}

void RemoteTasksDialog::updateState() {
    bool enable = getInfoTask == NULL && machine.get() != NULL && fetchResultTask == NULL;
    bool itemSelected = tasksTreeWidget->selectedItems().size() > 0;    

    refreshPushButton->setEnabled(enable);
    removePushButton->setEnabled(enable && itemSelected);
    fetchPushButton->setEnabled(enable && itemSelected);

}

void RemoteTasksDialog::refresh() {
    tasksTreeWidget->clear();
    getInfoTask = new GetUserTasksInfoTask(machine.get());
    getInfoTask->setErrorNotificationSuppression(true);
    connect(getInfoTask, SIGNAL(si_stateChanged()), SLOT(sl_onRefreshFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(getInfoTask);

}

void RemoteTasksDialog::sl_onRefreshFinished() {

    if (getInfoTask->getState() != Task::State_Finished) {
        return;
    }
    
    QList<RemoteTaskInfo> infoItems = getInfoTask->getUserTasksInfo();
    
    foreach(const RemoteTaskInfo& info, infoItems) {
        addItemToView(info);
    }

    QHeaderView * header = tasksTreeWidget->header();
    header->resizeSections( QHeaderView::ResizeToContents );

    getInfoTask = NULL;

    updateState();
}

void RemoteTasksDialog::addItemToView( const RemoteTaskInfo& info ) {
    QStringList strings;
    strings.append(info.taskId);
    strings.append(info.date);
    strings.append(info.taskState);
    strings.append(info.result);
    tasksTreeWidget->addTopLevelItem(new QTreeWidgetItem(strings));
}

void RemoteTasksDialog::sl_onRefreshButtonClicked() {
    refresh();
    updateState();
}

#define STATE_FINISHED "FINISHED"

void RemoteTasksDialog::sl_onFetchButtonClicked() {
   
    QTreeWidgetItem* current = tasksTreeWidget->currentItem();

    QString state = current->text(2);
    if (state != STATE_FINISHED) {
        QMessageBox::warning(this, tr("Fetch data error"), tr("The task is not finished yet."));
        return;
    }

    
    QString result = current->text(3);
    if (result.isEmpty()) {
        QMessageBox::warning(this, tr("Fetch data error"), tr("No result available for task."));
        return;
    }
        

    bool ok = false;
    qint64 taskId = current->text(0).toLongLong(&ok);
    if (!ok) {
        QMessageBox::warning(this, tr("Fetch data error"), tr("Failed to parse task id."));
        return;
    }

    LastOpenDirHelper h;
    QString folder = QFileDialog::getExistingDirectory(this, tr("Select directory to save results: "), h.dir);
    
    if (folder.isEmpty()) {
        return;
    }

    QList<QString> urls = result.split(";");
    for (int i = 0; i < urls.size(); ++i) {
        QString& str = urls[i];
        str.prepend(folder + "/");
    }

    fetchResultTask = new FetchRemoteTaskResultTask(machine.get(), urls, taskId);
    fetchResultTask->setErrorNotificationSuppression(true);
    connect(fetchResultTask, SIGNAL(si_stateChanged()), SLOT(sl_onFetchFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(fetchResultTask);
}

void RemoteTasksDialog::sl_onFetchFinished() {
    if (fetchResultTask->getState() != Task::State_Finished) {
        return;
    }
    
    if (!fetchResultTask->hasError()) {
        QMessageBox::information(this, tr("Fetch data"), tr("Download finished successfully."));
    } else {
        QMessageBox::critical(this, tr("Fetch data error"), tr("Failed to download task result. %1").arg(fetchResultTask->getError()));
    }

    fetchResultTask = NULL;
    updateState();
}

void RemoteTasksDialog::sl_onSelectionChanged() {
    updateState();
}

void RemoteTasksDialog::sl_onRemoveButtonClicked() {
    QTreeWidgetItem* current = tasksTreeWidget->currentItem();

    bool ok = false;
    qint64 taskId = current->text(0).toLongLong(&ok);
    if (!ok) {
        QMessageBox::critical(this, "Error!", "Failed to parse task id.");
        return;
    }
       
    deleteRemoteDataTask = new DeleteRemoteDataTask(machine.get(), taskId);
    deleteRemoteDataTask->setErrorNotificationSuppression(true);
    connect(deleteRemoteDataTask, SIGNAL(si_stateChanged()), SLOT(sl_onRemoveTaskFinished()));
    AppContext::getTaskScheduler()->registerTopLevelTask(deleteRemoteDataTask);
}

void RemoteTasksDialog::sl_onRemoveTaskFinished() {
    if (deleteRemoteDataTask->getState() != Task::State_Finished) {
        return;
    }

    if (deleteRemoteDataTask->hasError()) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to download task result. %1").arg(deleteRemoteDataTask->getError()));
    }

    deleteRemoteDataTask = NULL;
    refresh();
    updateState();   
}

void RemoteTasksDialog::sl_onDialogClosed() {
    //cleanup resources

    if (fetchResultTask != NULL) {
      fetchResultTask->disconnect(this);
    }
    
    if (getInfoTask != NULL) {
        getInfoTask->disconnect(this);
    }
    if (deleteRemoteDataTask != NULL) {    
        deleteRemoteDataTask->disconnect(this);
    }
    



}



}