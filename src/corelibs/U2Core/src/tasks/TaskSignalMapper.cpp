#include "TaskSignalMapper.h"

namespace U2 {

void TaskSignalMapper::sl_taskStateChanged() {
    QObject* o = sender();
    if (o) {
        Task* t = qobject_cast<Task*>(o);
        if (t) {
            switch (t->getState()) {
            case Task::State_Prepared:
                emit si_taskPrepared(t);
                break;
            case Task::State_Running:
                emit si_taskRunning(t);
                break;
            case Task::State_Finished:
                emit si_taskFinished(t);
                if (t->hasErrors() || t->hasSubtasksWithErrors()) {
                    emit si_taskFailed(t);
                } else {
                    emit si_taskSucceeded(t);
                }
                break;
            case Task::State_New:
                break;
            }
        }
    }
}

} //namespace
