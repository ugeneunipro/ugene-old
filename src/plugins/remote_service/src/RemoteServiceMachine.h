#ifndef _REMOTE_SERVICE_MACHINE_H_
#define _REMOTE_SERVICE_MACHINE_H_

#include <QtCore/QEventLoop>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>

#ifndef QT_NO_OPENSSL
#include <QtNetwork/QSslError>
#endif 

#include <memory>

#include <U2Remote/RemoteMachine.h>
#include "WebTransportProtocol.h"

namespace U2 {


class RemoteServiceMachineSettings : public RemoteMachineSettings {
public:
    RemoteServiceMachineSettings();
    RemoteServiceMachineSettings(const QString &url);

    virtual bool operator==( const RemoteMachineSettings& machine ) const;

    QString getUrl()const {return url; }
    QString getUserName() const; 
    QString getPasswd() const; 
    QString getSessionId() const { return sessionId; }
    void setSessionId(const QString& id) { sessionId = id; }
    bool usesGuestAccount() const;

    virtual QString getName() const { return url; }
    virtual QString serialize() const;

    bool deserialize( const QString & data );
    static const QString GUEST_ACCOUNT;
private:
    QString url;
    QString sessionId;
};

class UctpSession;

class RemoteServiceMachine : public QObject, public RemoteMachine {
    Q_OBJECT
public:
    RemoteServiceMachine(RemoteServiceMachineSettings* s);
    virtual ~RemoteServiceMachine();
    
    // Runs task on remote machine. Returns remote taskId
    virtual qint64 runTask( TaskStateInfo& si, const QString & taskFactoryId, const QVariant & settings);

    // Cancels remote task
    virtual void cancelTask(TaskStateInfo& si, qint64 taskId );

    // Retrieves remote task state
    virtual Task::State getTaskState(TaskStateInfo& si, qint64 taskId) ;

    // Retrieves remote task progress [0..100%]
    virtual int getTaskProgress(TaskStateInfo& si, qint64 taskId);

    // Retrieves remote task result
    virtual void getTaskResult(TaskStateInfo& si, qint64 taskId, const QStringList& resultUrls, const QString& urlPrefix );

    // Retrieves remote task error message 
    virtual QString getTaskErrorMessage(TaskStateInfo& si, qint64 taskId);

    // Returns remote machine server name. TODO: refactor
    virtual QString getServerName(TaskStateInfo& si);

    // Lists active tasks ids on remote machine
    virtual QList<qint64> getActiveTasks(TaskStateInfo& si);

    // Pings remote machine
    virtual void ping(TaskStateInfo& si);

    virtual const RemoteMachineSettings* getSettings() const {return settings;}

    //non-virtual methods -> TODO: refactor or add to the base interface
    void initSession(TaskStateInfo& si);
    
    // for the given task property names fills values
    void getTaskProperties(TaskStateInfo& si, qint64 taskId, QMap<QString,QString>& properties);

    // returns list of finished tasks
    QList<qint64> getFinishedTasks(TaskStateInfo& si);

    // removes remote task and all its data
    void deleteRemoteTask(TaskStateInfo& si, qint64 taskId);

#ifndef QT_NO_OPENSSL
private slots:
    void sl_onSslErrors( QNetworkReply* reply, const QList<QSslError> & errors );
#endif 

private:
    QMap<QString,UctpElementData> sendRequest(TaskStateInfo& si, UctpRequestBuilder& requestBuilder);
    void saveResult(const QByteArray& result, const QString& path); 
    void updateGlobalSettings();
    bool replyContainsData(const QString& dataName);

    QList<qint64> getTasksList(TaskStateInfo& si, const QByteArray& taskState);


    RemoteServiceMachineSettings*       settings;
    QNetworkProxy                       proxy;

#ifndef QT_NO_OPENSSL
    QSsl::SslProtocol                   sslProtocol;
#endif    

    std::auto_ptr<Uctp>                 protocolHandler;
    std::auto_ptr<UctpSession>          session;
    QString                             remoteServiceUrl;
    qlonglong                           parentThreadId;
};

// helper class to handle request/reply io
class RemoteServiceMachineReplyHandler : public QObject {
    Q_OBJECT
public:
    RemoteServiceMachineReplyHandler(Uctp* _ph, QEventLoop* _el, QMap<QString,UctpElementData>* _rd, 
                                    const QByteArray& _ac, TaskStateInfo* _si, QNetworkReply* _reply, const int _requestTimeout)
    : requestTimeout(_requestTimeout), protocolHandler(_ph), eventLoop(_el), replyData(_rd), command(_ac), si(_si), reply(_reply),
    inactiveCount(0) {}
private slots:
    void sl_onReplyFinished(QNetworkReply* reply);
    void sl_onUploadProgress(qint64 bytesSent, qint64 bytesTotal);
    void sl_onDownloadProgress(qint64 bytesSent, qint64 bytesTotal);
    void sl_onTimer();
private:
    const int                       requestTimeout;
    Uctp*                           protocolHandler;
    QEventLoop*                     eventLoop;
    QMap<QString,UctpElementData>*  replyData;
    QByteArray                      command;
    TaskStateInfo*                  si;
    QNetworkReply*                  reply;
    int                             inactiveCount;
};

class RemoteServiceMachineFactory : public RemoteMachineFactory {
public:
    RemoteServiceMachineFactory();
    virtual ~RemoteServiceMachineFactory();

    virtual RemoteMachine * createInstance( const QString & serializedSettings ) const;
    virtual RemoteMachine * createInstance( RemoteMachineSettings * settings ) const;
    virtual RemoteMachineSettings * createSettings( const QString & serializedSettings ) const;
    
};


} // namespace U2

#endif // _REMOTE_SERVICE_MACHINE_H_
