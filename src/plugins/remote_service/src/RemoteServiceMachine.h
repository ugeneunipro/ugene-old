/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _REMOTE_SERVICE_MACHINE_H_
#define _REMOTE_SERVICE_MACHINE_H_

#include <QtCore/QEventLoop>
#include <QtCore/QPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QAuthenticator>

#ifndef QT_NO_OPENSSL
#include <QtNetwork/QSslError>
#endif 

#include <memory>

#include <U2Remote/RemoteMachine.h>
#include "WebTransportProtocol.h"

namespace U2 {


class RemoteServiceMachineSettings : public RemoteMachineSettings {
public:
    RemoteServiceMachineSettings(const QString &url = QString());
    ~RemoteServiceMachineSettings();

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

typedef QSharedPointer<RemoteServiceMachineSettings> RemoteServiceSettingsPtr;

class RemoteServiceMachine : public QObject, public RemoteMachine {
    Q_OBJECT
public:
    RemoteServiceMachine(const RemoteServiceSettingsPtr& s);
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

    virtual RemoteMachineSettingsPtr getSettings(); 

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

    RemoteServiceSettingsPtr settings;

#ifndef QT_NO_OPENSSL
    QSsl::SslProtocol                   sslProtocol;
#endif    
    QNetworkProxy                       proxy;
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
    void onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*);
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

    virtual RemoteMachine * createInstance( const QString& serializedSettings ) const;
    virtual RemoteMachine * createInstance( const RemoteMachineSettingsPtr& settings ) const;
    virtual RemoteMachineSettingsPtr createSettings( const QString & serializedSettings ) const;
};


} // namespace U2

#endif // _REMOTE_SERVICE_MACHINE_H_
