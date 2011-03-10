
#ifndef _U2_REMOTE_MACHINE_H_
#define _U2_REMOTE_MACHINE_H_

#include "ProtocolInfo.h"

#include "Serializable.h"

#include <U2Core/Task.h>

namespace U2 {

class RemoteMachineSettings;

/* 
 * Base class for RemoteMachines. Should be implemented for each transport protocol.
 * Stores all required remote machine info, performs remote requests.
 */
class U2REMOTE_EXPORT RemoteMachine {
public:
    virtual ~RemoteMachine();

    // Runs task on remote machine. Returns remote taskId
    virtual qint64 runTask( TaskStateInfo& si, const QString & taskFactoryId, const QVariant & settings) = 0;
    
    // Cancels remote task
    virtual void cancelTask(TaskStateInfo& si, qint64 taskId ) = 0;

    // Retrieves remote task state
    virtual Task::State getTaskState(TaskStateInfo& si, qint64 taskId) = 0;

    // Retrieves remote task progress [0..100%]
    virtual int getTaskProgress(TaskStateInfo& si, qint64 taskId) = 0;

    // Retrieves remote task result
    virtual void getTaskResult(TaskStateInfo& si, qint64 taskId, const QStringList& resultUrls, const QString& urlPrefix ) = 0;
    
    // Retrieves remote task error message 
    virtual QString getTaskErrorMessage(TaskStateInfo& si, qint64 taskId) = 0;

    // Returns remote machine server name. TODO: refactor
    virtual QString getServerName(TaskStateInfo& si) = 0;

    // Lists active tasks ids on remote machine
    virtual QList<qint64> getActiveTasks(TaskStateInfo& si) = 0;

    // Pings remote machine
    virtual void ping(TaskStateInfo& si) = 0;

    virtual const RemoteMachineSettings* getSettings() const = 0;

}; // RemoteMachine


/*
* The type determines how to treat remote machine.
* In case of remote service the remote tasks are launched via UGENE remote service protocol.
*/

enum RemoteMachineType {
    RemoteMachineType_SimpleMachine,
    RemoteMachineType_RemoteService
};

/*
* Authentication information
*/
struct U2REMOTE_EXPORT UserCredentials {
    QString name;
    QString passwd;
    bool permanent;
};

/*
 * Base class for remote machine settings
 */
class U2REMOTE_EXPORT RemoteMachineSettings : public HumanSerializable {
public:
    RemoteMachineSettings( ProtocolInfo * proto,  RemoteMachineType type );
    virtual ~RemoteMachineSettings();

    RemoteMachineType getMachineType() const { return machineType; }
    const QString& getProtocolId() const {return protoId;}
    void flushCredentials();
    void setupCredentials(const QString& userName, const QString& passwd, bool permanent );
    UserCredentials* getUserCredentials() const { return credentials; }
    
    virtual QString serialize() const = 0;
    virtual QString getName() const = 0;
    virtual bool usesGuestAccount() const = 0;

    
    virtual bool operator==( const RemoteMachineSettings& ) const = 0;
    
protected:
    UserCredentials* credentials;
    QString protoId;
    RemoteMachineType machineType;
    
}; // RemoteMachineSettings

/*
 * Base class for RemoteMachinesFactories
 */
class U2REMOTE_EXPORT RemoteMachineFactory {
public:
    virtual ~RemoteMachineFactory();
    
    virtual RemoteMachine * createInstance( const QString & serializedSettings ) const = 0;
    virtual RemoteMachine * createInstance( RemoteMachineSettings * settings ) const = 0;
    virtual RemoteMachineSettings * createSettings( const QString & serializedSettings ) const = 0;
    
}; // RemoteMachineFactory

} // U2

#endif // U2_REMOTE_MACHINE_H_
