#ifndef _U2_TASK_MAPPER_H_
#define _U2_TASK_MAPPER_H_

#include <U2Core/Task.h>

namespace U2
{

class U2CORE_EXPORT TaskSignalMapper : public QObject {
    Q_OBJECT
public:
    TaskSignalMapper(QObject* parent = NULL) : QObject(parent) {}
    TaskSignalMapper(Task* t) : QObject(t) {
        connect(t, SIGNAL(si_stateChanged()), SLOT(sl_taskStateChanged()));
    }
public slots:
    void sl_taskStateChanged();
signals:
    void si_taskPrepared(Task*);
    void si_taskRunning(Task*);
    void si_taskFinished(Task*);
    void si_taskSucceeded(Task*);
    void si_taskFailed(Task*);
};

} //namespace

#endif 
