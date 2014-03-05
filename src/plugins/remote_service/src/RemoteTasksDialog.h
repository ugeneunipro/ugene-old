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

#ifndef _REMOTE_TASKS_DIALOG_H_
#define _REMOTE_TASKS_DIALOG_H_

#include <memory>

#include <qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QDialog>
#else
#include <QtWidgets/QDialog>
#endif
#include "ui/ui_TaskStatisticsDialog.h"
#include "RemoteServiceMachine.h"

namespace U2 {

class GetUserTasksInfoTask;
class FetchRemoteTaskResultTask;
class DeleteRemoteDataTask;
struct RemoteTaskInfo;

class RemoteTasksDialog : public QDialog, public Ui::RemoteTasksDialog
{
    Q_OBJECT
public:
    RemoteTasksDialog(const RemoteServiceSettingsPtr& s,QWidget* parent = NULL);

private slots:
    void sl_onRefreshFinished();
    void sl_onRefreshButtonClicked();
    void sl_onFetchButtonClicked();
    void sl_onRemoveButtonClicked();
    void sl_onRemoveTaskFinished();
    void sl_onFetchFinished();
    void sl_onSelectionChanged();
    void sl_onDialogClosed();


private:
    void addItemToView(const RemoteTaskInfo& info);
    void refresh();
    void updateState();
    void clearTreeWidget();
    GetUserTasksInfoTask* getInfoTask;
    FetchRemoteTaskResultTask* fetchResultTask;
    DeleteRemoteDataTask* deleteRemoteDataTask;
    std::auto_ptr<RemoteServiceMachine> machine;

};

} // namespace U2

#endif // _REMOTE_TASKS_DIALOG_H_
