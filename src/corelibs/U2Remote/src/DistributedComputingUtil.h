#ifndef _U2_DISTRIBUTED_COMPUTING_UTIL_H
#define _U2_DISTRIBUTED_COMPUTING_UTIL_H

#include <QtCore/QObject>

#include <U2Remote/ProtocolInfo.h>
#include <U2Remote/RemoteMachineMonitor.h>

namespace U2 {

class PingTask;

class U2REMOTE_EXPORT DistributedComputingUtil : public QObject {
    Q_OBJECT
public:
    DistributedComputingUtil();
    ~DistributedComputingUtil();
        
    /* remote machines has some services that are only for internal usage. this function filters them */
    static QStringList filterRemoteMachineServices( const QStringList & services );
    
private slots:
    void sl_showRemoteMachinesMonitor();
    
private:
    /* pointers here to manage object creation order */
    ProtocolInfoRegistry *        pir;
    RemoteMachineMonitor *        rmm;
    
}; // DistributedComputingUtil


/* Checks for active tasks and adds them to TaskScheduler 
 */
class UpdateActiveTasks : public Task {
    Q_OBJECT

public:
    UpdateActiveTasks( RemoteMachineSettings* s);
    ~UpdateActiveTasks();
    virtual void run();
    RemoteMachineSettings* getSettings() { return settings; }
   
private:
    RemoteMachine* machine;
    RemoteMachineSettings* settings;
    void addTaskToScheduler(qint64 taskid);

}; // UpdateActiveTasks

} // U2

#endif // _U2_DISTRIBUTED_COMPUTING_UTIL_H
