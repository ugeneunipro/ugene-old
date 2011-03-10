#ifndef _U2_SHUTDOWN_TASK_H_
#define _U2_SHUTDOWN_TASK_H_

#include <QtCore/QCoreApplication>
#include <U2Core/Task.h>

namespace U2 {

class ShutdownTask : public Task {
    Q_OBJECT
public:
    ShutdownTask(QCoreApplication* app);

    void prepare();

    ReportResult report();
protected slots:
    void startShutdown();
protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QCoreApplication* app;
};


}//namespace

#endif
