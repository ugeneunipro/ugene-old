#ifndef _U2_FAIL_TASK_H_
#define _U2_FAIL_TASK_H_

#include <U2Core/Task.h>

namespace U2
{
    class U2CORE_EXPORT FailTask : public Task {
        Q_OBJECT
    public:
        FailTask(const QString& msg) : Task(tr("Failure"), TaskFlag_NoRun) {
            stateInfo.setError(msg);
        }
    };
} //namespace

#endif 
