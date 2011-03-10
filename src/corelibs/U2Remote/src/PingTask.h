#ifndef _U2_PING_TASK_H_
#define _U2_PING_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class RemoteMachine;

class U2REMOTE_EXPORT PingTask : public Task {
     Q_OBJECT
public:
    PingTask(RemoteMachine* m);
    virtual void run();

private:
    RemoteMachine* machine;     
 }; // PingTask

} // U2

#endif // _U2_PING_TASK_H_
