#ifndef _TASK_STATUS_BAR_H_
#define _TASK_STATUS_BAR_H_

#include <U2Core/Task.h>
#include <U2Gui/Notification.h>
#include <U2Core/Log.h>

#include <QtGui/QWidget>
#include <QtGui/QLabel>
#include <QtGui/QProgressBar>
#include <QtGui/QPixmap>
#include <QtGui/QHBoxLayout>

namespace U2 {

class MainWindowImpl;

class TaskStatusBar : public QWidget {
    Q_OBJECT

public:
    TaskStatusBar();

private slots:
    void sl_taskStateChanged(Task* t);
    void sl_taskStateChanged();
    void sl_taskProgressChanged();
    void sl_taskDescChanged();
    void sl_reportsCountChanged();
    void sl_newReport(Task*);

    void sl_showReport();
    void sl_notificationChanged();

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void drawProgress(QLabel* l);
    
private:
    void updateState();
    void setTaskToTrack(Task* t);

    Task*           taskToTrack;
    QHBoxLayout*    l;
    QLabel*         taskInfoLabel;
    QProgressBar*   taskProgressBar;
    QLabel*         taskCountLabel;
    QLabel*         lampLabel;
    QLabel*         notificationLabel;
    QPixmap         iconOn;
    QPixmap         iconOff;
    int             nReports;
    bool            tvConnected;
    NotificationStack*   nStack;  
    QPixmap         notificationEmpty;
    QPixmap         notificationReport;
    QPixmap         notificationError;
};


}//namespace

#endif
