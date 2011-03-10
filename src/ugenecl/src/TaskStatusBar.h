#ifndef _TASK_STATUS_BAR_CON_H_
#define _TASK_STATUS_BAR_CON_H_

#include <U2Core/Task.h>
#define TSB_SETTINGS_ROOT QString("task_status_bar_settings/")
namespace U2 {


class TaskStatusBarCon : public QObject {
    Q_OBJECT
public:
    static const QString NO_TASK_STATUS_BAR_CMD_OPTION;
    
private:
    static bool helpRegistered;
    static void setTSBCmdlineHelp();
    
public:
    TaskStatusBarCon();
    ~TaskStatusBarCon();

private slots:
    void sl_taskStateChanged(Task* t);
    void sl_taskStateChanged();
    void sl_update();

private:
    void setTSBSettings();
    void updateState();
    void setTaskToTrack(Task* t);

    Task*           taskToTrack;
    QString         emptyLine;
};


}//namespace

#endif
