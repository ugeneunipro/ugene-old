#ifndef _REMOTE_TASKS_DIALOG_H_
#define _REMOTE_TASKS_DIALOG_H_

#include <memory>

#include <QtGui/QDialog>
#include "ui/ui_TaskStatisticsDialog.h"

namespace U2 {

class RemoteServiceMachineSettings;
class RemoteServiceMachine;
class GetUserTasksInfoTask;
class FetchRemoteTaskResultTask;
class DeleteRemoteDataTask;
struct RemoteTaskInfo;

class RemoteTasksDialog : public QDialog, public Ui::RemoteTasksDialog
{
    Q_OBJECT
public:
    RemoteTasksDialog(const RemoteServiceMachineSettings* s,QWidget* parent = NULL);

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
