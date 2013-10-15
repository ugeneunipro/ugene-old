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


#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtCore/QTimer>

#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/Counter.h>

#include <U2Remote/SerializeUtils.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/CoreLibConstants.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/BaseTypes.h>
#include <U2Lang/BaseAttributes.h>
#include <time.h>

#include "RemoteWorkflowRunTask.h"


namespace U2 {

#define TASK_OUTPUT_DIR "out/"
#define TASK_INPUT_DIR "in/"

/***************************************
* RemoteWorkflowRunTask
***************************************/

RemoteWorkflowRunTask::RemoteWorkflowRunTask( const RemoteMachineSettingsPtr& m, const Schema & sc)
    : Task( tr( "Workflow run task on the cloud" ), TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled ), machineSettings( m ), 
      machine( NULL ), schema( sc ), taskId(0), eventLoop(NULL),taskIsActive(false) 
{
    GCOUNTER(cvar, tvar, "WorkflowOnTheCloud");
    if( NULL == machineSettings ) {
        setError( tr("Bad remote machine settings"));
        return;
    }
    tpm = Progress_Manual;
}

RemoteWorkflowRunTask::RemoteWorkflowRunTask( const RemoteMachineSettingsPtr& m, qint64 remoteTaskId )
: Task( tr( "Workflow run task on the cloud" ), TaskFlags_FOSCOE | TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled), machineSettings( m ), 
machine( NULL ), taskId(remoteTaskId), eventLoop(NULL), taskIsActive(true)
{
    GCOUNTER(cvar, tvar, "WorkflowOnTheCloud");
    if( NULL == machineSettings ) {
        setError( tr("Bad remote machine settings"));
        return;
    }
    tpm = Progress_Manual;    
}

void RemoteWorkflowRunTask::preprocessSchema()
{
    foreach( Actor * actor, schema.getProcesses() ) {
        assert( NULL != actor );
        if( actor->getParameter( BaseAttributes::URL_IN_ATTRIBUTE().getId() ) != NULL &&
            actor->getParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId()) == NULL ) {

                actor->addParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId(), 
                    new Attribute( BaseAttributes::URL_LOCATION_ATTRIBUTE(), BaseTypes::BOOL_TYPE(), false, true ) );
        }
    }
}

void RemoteWorkflowRunTask::dumpSchema(const QString& fileName, const QByteArray& schema) {
    QFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(schema);
    file.close();
}


void RemoteWorkflowRunTask::prepare()
{
    if( hasError() || isCanceled() ) {
        return;
    }

    rsLog.trace(tr("Started remote workflow task"));
    machine = AppContext::getProtocolInfoRegistry()->getProtocolInfo( machineSettings->getProtocolId() )
        ->getRemoteMachineFactory()->createInstance( machineSettings );
    if( NULL == machine ) {
        setError( tr( "Cannot create remote machine from remote machine settings: %1" ).arg( machineSettings->getName()) );
        return;
    }

    stateInfo.progress = 0;

    if (taskIsActive) {
        return;
    }

    preprocessSchema();

    QStringList inputUrls;

    foreach( Actor * actor, schema.getProcesses() ) {
        assert( NULL != actor );
        ActorId actorId = actor->getId();

        Attribute * urlInAttr = actor->getParameter( BaseAttributes::URL_IN_ATTRIBUTE().getId() );
        if( NULL != urlInAttr ) {
            Attribute * urlLocAttr = actor->getParameter(BaseAttributes::URL_LOCATION_ATTRIBUTE().getId());
            assert(NULL != urlLocAttr);
            if (urlLocAttr->getAttributePureValue().toBool()) { // file located on this computer
                QString urlpath = urlInAttr->getAttributePureValue().toString();
                // multiple urls are in the same string
                // TODO: folder contents handling
                QStringList urls = urlpath.split(';', QString::SkipEmptyParts); 
                QStringList newPathes;
                foreach (const GUrl& filePath, urls) {
                    QString path = TASK_INPUT_DIR + filePath.fileName();
                    inputUrls.append(filePath.getURLString());
                    newPathes.append(path);
                }
                // skip first semicolon
                QString newPath = newPathes.join(";");
                urlInAttr->setAttributeValue(newPath);
            }
        }

        Attribute * urlOutAttr = actor->getParameter( BaseAttributes::URL_OUT_ATTRIBUTE().getId() );
        if( NULL != urlOutAttr ) {
            assert( NULL == actor->getParameter( BaseAttributes::URL_LOCATION_ATTRIBUTE().getId() ) );
            GUrl filePath = urlOutAttr->getAttributePureValue().toString();
            QString newPath = TASK_OUTPUT_DIR + filePath.fileName();
            outputUrls.append(filePath.getURLString());
            urlOutAttr->setAttributeValue(newPath);
        }
    }

    QByteArray rawData = HRSchemaSerializer::schema2String(schema, NULL).toUtf8();
    taskSettings.insert(CoreLibConstants::WORKFLOW_SCHEMA_ATTR, rawData);
    taskSettings.insert(CoreLibConstants::DATA_IN_ATTR, inputUrls);
    taskSettings.insert(CoreLibConstants::DATA_OUT_ATTR, outputUrls);

    rsLog.trace("Workflow is preprocessed for sending to remote service");

#ifdef _DEBUG
    assert(!WorkflowUtils::WD_FILE_EXTENSIONS.isEmpty());
    dumpSchema("dump." + WorkflowUtils::WD_FILE_EXTENSIONS.first(), rawData);
#endif
}

void RemoteWorkflowRunTask::run() {
    if (!taskIsActive) {
        taskId = machine->runTask(stateInfo, CoreLibConstants::WORKFLOW_ON_CLOUD_TASK_ID, taskSettings);
        if (hasError()) {
            return;
        }
    }

    eventLoop = new QEventLoop(this);

    QTimer::singleShot(RemoteWorkflowRunTask::TIMER_UPDATE_TIME, this, SLOT(sl_remoteTaskTimerUpdate()));
    eventLoop->exec(QEventLoop::ExcludeUserInputEvents);

    delete eventLoop;
    eventLoop = NULL;
}


void RemoteWorkflowRunTask::sl_remoteTaskTimerUpdate()  {
    assert( eventLoop != NULL );
    if (isCanceled()) {
        machine->cancelTask(stateInfo, taskId);
        eventLoop->exit();
        return;
    }  

    State state = State_Running;
    state = machine->getTaskState(stateInfo, taskId);
    if (hasError()) {
        eventLoop->exit();
        return;
    }

    if (state == State_Finished) {
        rsLog.trace("Workflow task finished on remote host.");
        if (!outputUrls.isEmpty()) {
            machine->getTaskResult(stateInfo, taskId, outputUrls, TASK_OUTPUT_DIR);
            if (hasError()) {
                eventLoop->exit();
                return;
            }
            rsLog.trace("Retrieved result data from remote host.");
        }
        eventLoop->exit();
        return;

    }

    int progress = machine->getTaskProgress(stateInfo, taskId);
    if (hasError()) {
        eventLoop->exit();
        return;
    }

    stateInfo.progress = progress;
    QTimer::singleShot( RemoteWorkflowRunTask::TIMER_UPDATE_TIME, this, SLOT( sl_remoteTaskTimerUpdate() ) );
}

QString RemoteWorkflowRunTask::generateReport() const {
    QString res;
    res+="<table width='75%'>";
    res+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("Task")).arg(tr("Status")).arg(tr("Details"));
    QString name = Qt::escape(getTaskName());
    QString status = hasError() ? tr("Failed") : isCanceled() ? tr("Canceled") : tr("Finished");
    QString error = Qt::escape(getError()).replace("\n", "<br>");
    if (hasError()) {
        name = "<font color='red'>"+name+"</font>";
        status = "<font color='red'>"+status+"</font>";
    } else if (isCanceled()) {
        status = "<font color='blue'>"+status+"</font>";
    } else {
        status = "<font color='green'>"+status+"</font>";
    }
    res+=QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(name).arg(status).arg(error);
    res += QString("<tr><td><i>%1</i></td></tr>").arg(tr("Output files:"));
    foreach(QString url, outputUrls) {
        if(QFile::exists(url)) {
            res += QString("<tr><td><a href=\"%1\">%2</a></td></tr>").arg(url).arg(url);
        }
    }
    res+="<tr><td></td></tr>";
    res+="</table>";
    return res;
}


Task::ReportResult RemoteWorkflowRunTask::report() {
    if (!hasError() && !isCanceled()) {
        rsLog.details("Remote task finished successfully");
    }
    return ReportResult_Finished;
}

RemoteWorkflowRunTask::~RemoteWorkflowRunTask() {
    delete machine;
}


} // ~ U2


