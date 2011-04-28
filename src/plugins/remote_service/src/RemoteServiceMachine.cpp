/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include <QtCore/QCryptographicHash>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#ifndef QT_NO_OPENSSL
#include <QtNetwork/QSslConfiguration>
#endif

#include <U2Core/AppContext.h>
#include <U2Core/NetworkConfiguration.h>
#include <U2Core/AppSettings.h>
#include <U2Core/Log.h>
#include <U2Core/GUrl.h>

#include <U2Remote/RemoteMachineMonitor.h>
#include <U2Lang/CoreLibConstants.h>

#include "RemoteServiceCommon.h"
#include "RemoteServiceMachine.h"
#include "UctpRequestBuilders.h"

namespace U2 {


const QString RemoteServiceMachineSettings::GUEST_ACCOUNT("guest");

#ifndef QT_NO_OPENSSL

static bool registerMeta() {
    qRegisterMetaType<QSslError>("QSslError");
    qRegisterMetaType< QList<QSslError> >("QList<QSslError>");
    return true;
}

static bool regMetaType = registerMeta();

#endif

// RemoteServiceMachine

RemoteServiceMachine::RemoteServiceMachine(RemoteServiceMachineSettings* s)
: settings(s), protocolHandler(new Uctp()), session(NULL) 
{

    remoteServiceUrl = settings->getUrl();
   
    NetworkConfiguration* nc = AppContext::getAppSettings()->getNetworkConfiguration();
    proxy = nc->getProxyByUrl(remoteServiceUrl);

#ifndef QT_NO_OPENSSL
    sslProtocol = nc->getSslProtocol();
#endif //QT_NO_OPENSSL

    QByteArray sid = settings->getSessionId().toAscii();
    if (!sid.isEmpty() ) {
        session.reset(new UctpSession(sid));    
    }
    rsLog.trace("Started remote service machine instance");
    parentThreadId = (qlonglong) QThread::currentThreadId();
    rsLog.trace(QString("RemoteServiceMachine(): current thread is %1").arg( parentThreadId ));

}

RemoteServiceMachine::~RemoteServiceMachine() {
    UserCredentials* credentials = settings->getUserCredentials();
    if (credentials != NULL) {
        if (!credentials->permanent) {
            settings->flushCredentials();
        }
    }
}


qint64 RemoteServiceMachine::runTask(TaskStateInfo& si, const QString & taskFactoryId, const QVariant & settings) {
    qint64 taskId = -1;

    if (taskFactoryId != Workflow::CoreLibConstants::WORKFLOW_ON_CLOUD_TASK_ID) {
        si.setError(tr("Unsupported task type"));
        return taskId;
    }    

    initSession(si);
    if (si.hasError()) {
        return taskId;
    }
    
    rsLog.trace("Init session successful");

    QVariantMap settingsMap = settings.toMap();
    const QByteArray schema = settingsMap.value(Workflow::CoreLibConstants::WORKFLOW_SCHEMA_ATTR).toByteArray();
    const QStringList inUrls =  settingsMap.value(Workflow::CoreLibConstants::DATA_IN_ATTR).toStringList();
    
    RunRemoteTaskRequest request(session.get(), schema, inUrls);
    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return taskId;
    }
    UctpElementData elData = replyData.value(UctpElements::TASK);
    QString taskIdStr (elData.attributesMap.value(UctpAttributes::TASK_ID));
    bool ok = false;
    taskId = taskIdStr.toLongLong(&ok);
    if (!ok) {
        si.setError(tr("Illegal task id format: %1").arg(taskId));
        return taskId;
    }
    return taskId;
}


void RemoteServiceMachine::cancelTask(TaskStateInfo& si, qint64 taskId) {
    UpdateRemoteTaskRequest request(UctpCommands::CANCEL_TASK, session.get(), taskId);
    sendRequest(si, request);
}

static QString getElementValueByNameAttr(const QString& name, const QList<UctpElementData> elements) {
    QString result;
    foreach (const UctpElementData& data, elements) {
        if (data.attributesMap.value(UctpAttributes::NAME) == name) {
            result = data.textData;
            break;
        }    
    }

    return result;
}

 Task::State RemoteServiceMachine::getTaskState(TaskStateInfo& si, qint64 taskId) {
    Task::State state = Task::State_Running;

    QStringList properties;
    properties.append(UctpElements::TASK_STATE);
    properties.append(UctpElements::TASK_ERROR);
    GetRemoteTaskPropertyRequest request(session.get(), taskId, properties);

    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return state;
    }

    QList<UctpElementData> propertyList = replyData.values(UctpElements::PROPERTY);
    QByteArray stateText = getElementValueByNameAttr(UctpElements::TASK_STATE, propertyList).toAscii(); 
    if (stateText == UctpValues::UCTP_STATE_FINISHED) {
        state = Task::State_Finished;
        QByteArray errorText = getElementValueByNameAttr(UctpElements::TASK_ERROR, propertyList).toAscii();
        if (!errorText.isEmpty()) {
            si.setError(errorText);
        }
    } else if (stateText.isEmpty()) {
        si.setError(tr("Empty task state property"));
    }
    return state;
}

int RemoteServiceMachine::getTaskProgress(TaskStateInfo& si, qint64 taskId) {
    int res = 0;
    QStringList properties;
    properties.append(UctpElements::TASK_PROGRESS);
    GetRemoteTaskPropertyRequest request(session.get(), taskId, properties);
    
    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    
    if (si.hasError()) {
        return res;
    }

    QList<UctpElementData> propertyList = replyData.values(UctpElements::PROPERTY);
    QString buf = getElementValueByNameAttr(UctpElements::TASK_PROGRESS, propertyList); 
    bool ok = false;
    res = buf.toInt(&ok);
    if (!ok) {
        si.setError(tr("Failed to parse task progress: %1").arg(buf));
    }
    return res;
}

void RemoteServiceMachine::getTaskResult(TaskStateInfo& si, qint64 taskId, const QStringList& urls, const QString& urlPrefix) {
    try {
        if (urls.isEmpty()) {
            si.setError(tr("No files specified"));
            return;
        }
        
        GetRemoteTaskResultRequst request(session.get(), taskId);

        QMap<QString,UctpElementData> replyData = sendRequest(si, request);
        if (si.hasError()) {
            return;
        }

        QList<UctpElementData> elements = replyData.values(UctpElements::DATA);
        if (!elements.isEmpty()) {
            foreach( const GUrl& url, urls ) {
                QString dataName = urlPrefix + url.fileName();
                const QByteArray data = getElementValueByNameAttr(dataName, elements).toAscii();
                if (data.isEmpty()) {
                    si.setError(tr("Result {%1} is not found.").arg(dataName));
                    return;
                }
                saveResult( QByteArray::fromBase64(data), url.getURLString());
            }
        }

    } catch (UctpError& e) {
        si.setError(e.getErrorMessage());
    }
}


QString RemoteServiceMachine::getTaskErrorMessage(TaskStateInfo& si, qint64 taskId) {
    QString res;
    
    QStringList properties;
    properties.append(UctpElements::TASK_ERROR);
    GetRemoteTaskPropertyRequest request(session.get(), taskId, properties);

    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return res;
    }

    QList<UctpElementData> propertyList = replyData.values(UctpElements::PROPERTY);
    res = getElementValueByNameAttr(UctpElements::TASK_ERROR, propertyList); 
    return res;
}

void RemoteServiceMachine::ping(TaskStateInfo& si) {
    PingRequest request;

    sendRequest(si, request);
}


QString RemoteServiceMachine::getServerName(TaskStateInfo& si) {
    QString res;
    initSession(si);
    if (si.hasError()) {
        return res;
    }
     
    GetGlobalPropertyRequest request(session.get(), BaseGlobalProperties::HOST_NAME);
    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return res;
    }
    UctpElementData elData = replyData.value(UctpElements::PROPERTY);
    if (elData.attributesMap.value(UctpAttributes::NAME) == BaseGlobalProperties::HOST_NAME ) {
        res = elData.textData;
    }
    return res;
}

void RemoteServiceMachineReplyHandler::sl_onReplyFinished(QNetworkReply* reply ) {
    assert(reply != NULL);
    assert(eventLoop != NULL);
    si->setDescription("");
    if (reply->error() == QNetworkReply::NoError) {
        bool parseOk = protocolHandler->parseReply(reply, command, *replyData);
        if (!parseOk) {
            si->setError(QString(tr("Failed to parse server response. %1")).arg(protocolHandler->getErrorText()));
        }
    } else {
        if(!si->hasError()) {
            si->setError(reply->errorString());
        }
    }
    
    eventLoop->exit();
}

void RemoteServiceMachineReplyHandler::sl_onUploadProgress(qint64 bytesSent, qint64 bytesTotal) {
    if(bytesTotal != -1) {
        si->setDescription(RemoteServiceMachineReplyHandler::tr("Uploading %1%").arg((int)(bytesSent / (float)bytesTotal * 100)));
    }
    inactiveCount = 0;
}

void RemoteServiceMachineReplyHandler::sl_onDownloadProgress(qint64 bytesSent, qint64 bytesTotal) {
    if(bytesTotal != -1) {
        si->setDescription(RemoteServiceMachineReplyHandler::tr("Downloading %1%").arg((int)(bytesSent / (float)bytesTotal * 100)));
    }
    inactiveCount = 0;
}

void RemoteServiceMachineReplyHandler::sl_onTimer() {
    if(si->cancelFlag) {
        reply->abort();
    }
    if(static_cast<QTimer*>(sender())->interval() * inactiveCount > requestTimeout ) {
        si->setError(RemoteServiceMachineReplyHandler::tr("Request inactivity time is up"));
        reply->abort();
    }
    inactiveCount++;
}


QMap<QString,UctpElementData> RemoteServiceMachine::sendRequest(TaskStateInfo& si,  UctpRequestBuilder& requestBuilder ) {
    
    QByteArray command = requestBuilder.getCommand();
    rsLog.trace(QString("SendRequest(%1): current thread is %2").arg(command.constData()).arg(( qlonglong) QThread::currentThreadId() ));

    QMap<QString,UctpElementData> replyData;

    QIODevice* dataSource = requestBuilder.getDataSource();
    
    if (dataSource == NULL) {
        si.setError(tr("Send request failed: empty message data"));
        return replyData;
    }
    
    qint64 dataLength = dataSource->size();
    QEventLoop eventLoop;
    QNetworkAccessManager networkManager;

    QNetworkRequest request (remoteServiceUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");
        
#ifndef QT_NO_OPENSSL
    QSslConfiguration sslConf = QSslConfiguration::defaultConfiguration();

    sslConf.setProtocol(sslProtocol);
    request.setSslConfiguration(sslConf);
#endif

    QNetworkReply *reply = networkManager.post(request, dataSource );
    QTimer timer;
    RemoteServiceMachineReplyHandler handler(protocolHandler.get(), &eventLoop, &replyData, command, &si, reply, AppContext::getAppSettings()->getNetworkConfiguration()->remoteRequestTimeout() * 1000);

    connect(&networkManager, SIGNAL(finished(QNetworkReply*)), &handler, SLOT(sl_onReplyFinished(QNetworkReply*)));
    connect(reply, SIGNAL(uploadProgress(qint64, qint64)), &handler, SLOT(sl_onUploadProgress(qint64, qint64)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), &handler, SLOT(sl_onDownloadProgress(qint64, qint64)));
    connect(&timer, SIGNAL(timeout()), &handler, SLOT(sl_onTimer()));

#ifndef QT_NO_OPENSSL
    // QNetworkAccessManager bug workaround:
    // If the network reply and the manager are created in different threads 
    // blocked connection is required for ssl errors signal
    qlonglong threadId = (qlonglong) QThread::currentThreadId();
    Qt::ConnectionType connectionType = threadId == parentThreadId ? Qt::AutoConnection : Qt::BlockingQueuedConnection; 
    connect(&networkManager, SIGNAL(sslErrors( QNetworkReply *, const QList<QSslError> & )), SLOT(sl_onSslErrors( QNetworkReply*, const QList<QSslError>& )), connectionType );
#endif

    timer.start(1000);

    eventLoop.exec();

    timer.stop();

    return replyData;
}

void RemoteServiceMachine::initSession(TaskStateInfo& si) {
    if (settings->getUserCredentials() == NULL) {
        si.setError(tr("User auth info is not available"));
        return;
    }
    
    if (session.get() != NULL) {
        return;
    }
    QByteArray userName = settings->getUserName().toAscii();
    QByteArray pass = settings->getPasswd().toAscii();

    if (userName == RemoteServiceMachineSettings::GUEST_ACCOUNT) {
        // in this case we set the password
        pass = RemoteServiceMachineSettings::GUEST_ACCOUNT.toAscii();
    }

    InitSessionRequest request(userName, pass);
    
    QMap<QString,UctpElementData> replyData =  sendRequest(si, request);
    if (si.hasError()) {
        return;
    }
    UctpElementData elData = replyData.value(UctpElements::SESSION);
    QByteArray id = elData.attributesMap.value(UctpAttributes::SESSION_ID);
    session.reset(new UctpSession(id));
    updateGlobalSettings();
}


void RemoteServiceMachine::saveResult( const QByteArray& result, const QString& path ) {
    QFile resultFile(path);
    if ( !resultFile.open(QIODevice::WriteOnly) ) {
        throw UctpError("Can not open file to write results");
    }

    qint64 bytesWrittern = resultFile.write(result);
    if (bytesWrittern != result.size()) {
        throw UctpError("Error writing results");
    }
}

void RemoteServiceMachine::updateGlobalSettings() {
    settings->setSessionId(session->getUid());
}

#ifndef QT_NO_OPENSSL
void RemoteServiceMachine::sl_onSslErrors(QNetworkReply * reply, const QList<QSslError> & errors ) {
    QSslError error;
    foreach(const QSslError& e, errors) {
        rsLog.trace(tr("SSL connection errors: %1").arg(e.errorString()));
    } 
    reply->ignoreSslErrors();
}
#endif //QT_NO_OPENSSL



void RemoteServiceMachine::getTaskProperties( TaskStateInfo& si, qint64 taskId, QMap<QString,QString>& properties) {
    GetRemoteTaskPropertyRequest request( session.get(), taskId, properties.keys());

    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return;
    }
    QList<UctpElementData> propertyList = replyData.values(UctpElements::PROPERTY);
    foreach (const QString& propertyName, properties.keys()) {
        QString propertyValue = getElementValueByNameAttr(propertyName, propertyList); 
        properties.insert(propertyName, propertyValue);
    }
}


QList<qint64> RemoteServiceMachine::getActiveTasks(TaskStateInfo& si) {
    return getTasksList(si, BaseGlobalProperties::TASKS_LIST_ACTIVE);
}

QList<qint64> RemoteServiceMachine::getFinishedTasks(TaskStateInfo& si) {
    return getTasksList(si, BaseGlobalProperties::TASKS_LIST_FINISHED);
}


QList<qint64> RemoteServiceMachine::getTasksList( TaskStateInfo& si, const QByteArray& taskState) {
    QList<qint64> res;
    initSession(si);
    if (si.hasError()) {
        return res;
    }   

    GetGlobalPropertyRequest request( session.get(), taskState);

    QMap<QString,UctpElementData> replyData = sendRequest(si, request);
    if (si.hasError()) {
        return res;
    }
    QList<UctpElementData> propertyList = replyData.values(UctpElements::PROPERTY);
    QStringList ids = getElementValueByNameAttr(taskState, propertyList).split(",", QString::SkipEmptyParts);
    foreach(const QString& idStr, ids ) {
        bool ok = false; 
        qint64 taskId = idStr.toLongLong(&ok);
        if (ok) {
            res.append(taskId);
        }
    } 
    return res;
}

void RemoteServiceMachine::deleteRemoteTask(TaskStateInfo& si,  qint64 taskId) {
    UpdateRemoteTaskRequest request(UctpCommands::DELETE_TASK, session.get(), taskId);
    sendRequest( si, request );
}


// RemoteServiceMachineSettings

RemoteServiceMachineSettings::RemoteServiceMachineSettings():
RemoteMachineSettings( AppContext::getProtocolInfoRegistry()->getProtocolInfo( RemoteServiceCommon::WEB_TRANSPORT_PROTOCOL_ID), RemoteMachineType_RemoteService)

{
}

RemoteServiceMachineSettings::RemoteServiceMachineSettings(const QString &host):
RemoteMachineSettings(AppContext::getProtocolInfoRegistry()->getProtocolInfo( RemoteServiceCommon::WEB_TRANSPORT_PROTOCOL_ID ), RemoteMachineType_RemoteService ),
url(host)
{
}

#define URL_ATTR "url"
#define NAME_ATTR "login"
#define PASSWD_ATTR "passwd"



bool RemoteServiceMachineSettings::operator ==( const RemoteMachineSettings & m ) const {
    const RemoteMachineSettings * machine = &m;
    const RemoteServiceMachineSettings* cfg = dynamic_cast< const RemoteServiceMachineSettings* >( machine );
    if( NULL == cfg ) {
        return false;
    }
    
    return url == cfg->getUrl();
}

QString RemoteServiceMachineSettings::serialize() const {
    QStringList cfg;
    cfg.append(QString("%1=%2").arg(URL_ATTR).arg(url));
    if (credentials != NULL) {
        cfg.append(QString("%1=%2").arg(NAME_ATTR).arg(credentials->name));
        cfg.append(QString("%1=%2").arg(PASSWD_ATTR).arg(credentials->passwd));
    }

    return cfg.join("\n");
}

bool RemoteServiceMachineSettings::deserialize( const QString & data ) {
    QStringList lines = data.split("\n", QString::SkipEmptyParts);
    
    QString name, passwd;
    foreach (const QString cfgLine, lines) {
        if (cfgLine.startsWith("#")) {
            continue;
        } else if (cfgLine.startsWith(URL_ATTR)) {
             url = cfgLine.split("=").at(1).trimmed();
        } else if (cfgLine.startsWith(NAME_ATTR)) {
            name =cfgLine.split("=").at(1).trimmed();
        } else if (cfgLine.startsWith(PASSWD_ATTR)) {
            passwd = cfgLine.split("=").at(1).trimmed();
        }
    }
    
    if (url.isEmpty()) {
        return false;
    }

    if (!name.isEmpty()) {
        if (name == GUEST_ACCOUNT || !passwd.isEmpty()){
            setupCredentials(name, passwd, true);
        }
    }

    return true;
}


QString RemoteServiceMachineSettings::getUserName() const {
    assert(credentials != NULL);
    return credentials->name;
}

QString RemoteServiceMachineSettings::getPasswd() const {
    assert(credentials != NULL);
    return credentials->passwd;
}

bool RemoteServiceMachineSettings::usesGuestAccount() const {
    if (credentials == NULL) {
        return false;
    }

    if (credentials->name == GUEST_ACCOUNT) {
        return true;
    } else {
        return false;
    }
}

// RemoteServiceMachineFactory

RemoteServiceMachineFactory::RemoteServiceMachineFactory() {
}

RemoteServiceMachineFactory::~RemoteServiceMachineFactory() {
}

RemoteMachine * RemoteServiceMachineFactory::createInstance( const QString & serializedSettings ) const {
    RemoteServiceMachineSettings settings;
    if(settings.deserialize(serializedSettings))
    {
        return createInstance(&settings);
    }
    return NULL;
}

RemoteMachine * RemoteServiceMachineFactory::createInstance( RemoteMachineSettings * settings ) const {
    RemoteServiceMachineSettings *castedSettings = dynamic_cast<RemoteServiceMachineSettings *>(settings);
    if(NULL != castedSettings) {
        return new RemoteServiceMachine(castedSettings);
    }
    return NULL;
}

RemoteMachineSettings * RemoteServiceMachineFactory::createSettings( const QString & serializedSettings ) const {
    RemoteServiceMachineSettings * ret = new RemoteServiceMachineSettings();
    if( ret->deserialize( serializedSettings ) ) {
        return ret;
    }
    delete ret;
    return NULL;
}

} // namespace U2
