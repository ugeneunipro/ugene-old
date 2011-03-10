#ifndef _REMOTE_SERVICE_PING_TASK_H_
#define _REMOTE_SERVICE_PING_TASK_H_

#include <U2Core/Task.h>
#include <memory>

namespace U2 {

 class RemoteServiceMachine;
 class RemoteServiceMachineFactory;

class RemoteServicePingTask : public Task {
public:
    RemoteServicePingTask(const QString& url);
    virtual void prepare();
    virtual void run();
private:
    QString machinePath;
    std::auto_ptr<RemoteServiceMachine> machine;
    std::auto_ptr<RemoteServiceMachineFactory> machineFactory;
};
  

} // namespace U2

#endif // _REMOTE_SERVICE_PING_TASK_H_
