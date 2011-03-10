#ifndef _U2_TASK_STARTER_H_
#define _U2_TASK_STARTER_H_

#include <U2Core/Task.h>

namespace U2
{
/**
* Intended for deferred task run, triggered by a signal.
*/
class U2CORE_EXPORT TaskStarter : public QObject {
    Q_OBJECT
public:
    TaskStarter(Task* t) : QObject(t), t(t) {}
public slots:
    void registerTask();
private:
    Task* t;
};

} //namespace

#endif 
