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

#include "WorkflowRunTask.h"
#include <U2Lang/WorkflowEnv.h>
#include <U2Lang/WorkflowManager.h>
#include <U2Lang/Schema.h>
#include <U2Lang/HRSchemaSerializer.h>
#include <U2Lang/WorkflowUtils.h>
#include <U2Lang/BaseAttributes.h>
#include <U2Lang/WorkflowSettings.h>

#include <U2Core/Counter.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/AppContext.h>
#include <U2Core/Version.h>
#include <U2Core/Settings.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/GUrlUtils.h>
#include <U2Core/BaseDocumentFormats.h>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace U2 {

static const QString OUTPUT_PROGRESS_OPTION("ugene-output-progress-state");
static const QString OUTPUT_ERROR_OPTION("ugene-output-error");
static const int UPDATE_PROGRESS_INTERVAL = 500;
static const int TICK_UPDATE_INTERVAL = 1000;
static const QString OUTPUT_PROGRESS_TAG("task-progress=");
static const QString ERROR_KEYWORD("#%*ugene-finished-with-error#%*");
static const QString STATE_KEYWORD("#%&state#%&");
static const QString MSG_NUM_KEYWORD("#%$msgnum#%$");
static const QString MSG_PASSED_KEYWORD("#%$msgpassed#%$");
static const QString SEP_PROCESS_PREFIX("Workflow separate process: ");

/*******************************************
 * WorkflowRunTask
 *******************************************/
WorkflowRunTask::WorkflowRunTask(const Schema& sh, QList<Iteration> lst, const QMap<ActorId, ActorId>& remap) :
WorkflowAbstractRunner(tr("Execute workflow"), TaskFlags(TaskFlag_NoRun) | TaskFlag_ReportingIsSupported), rmap(remap), flows(sh.getFlows()) {
    GCOUNTER( cvar, tvar, "WorkflowRunTask" );
    assert(!lst.isEmpty());
    foreach(const Iteration& it, lst) {
        WorkflowIterationRunTask* t = new WorkflowIterationRunTask(sh, it);
        connect(t, SIGNAL(si_ticked()), SIGNAL(si_ticked()));
        addSubTask(t);
    }
    setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS_AUTO);
    if(AppContext::getCMDLineRegistry()->hasParameter(OUTPUT_PROGRESS_OPTION)) {
        QTimer * timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), SLOT(sl_outputProgressAndState()));
        timer->start(UPDATE_PROGRESS_INTERVAL);
    }
}

void WorkflowRunTask::sl_outputProgressAndState() {
    coreLog.info(QString("%1%2").arg(OUTPUT_PROGRESS_TAG).arg(getProgress()));
    foreach(const ActorId & id, rmap.values()) {
        QList<WorkerState> ret;
        foreach(Task* t, getSubtasks()) {
            WorkflowIterationRunTask* rt = qobject_cast<WorkflowIterationRunTask*>(t);
            ret << rt->getState(id);
        }
        if(!rmap.key(id).isEmpty()) {
            assert(ret.size() == 1);
            coreLog.info(QString("%1:%2:%3").arg(STATE_KEYWORD).arg(rmap.key(id)).arg((int)ret.first()));
        }
    }
    foreach(Link * l, flows) {
        ActorId srcId = rmap.key(l->source()->owner()->getId());
        ActorId dstId = rmap.key(l->destination()->owner()->getId());
        assert(!srcId.isEmpty() && !dstId.isEmpty());
        coreLog.info(QString("%1:%2:%3:%4").arg(MSG_NUM_KEYWORD).arg(srcId).arg(dstId).arg(getMsgNum(l)));
        coreLog.info(QString("%1:%2:%3:%4").arg(MSG_PASSED_KEYWORD).arg(srcId).arg(dstId).arg(getMsgPassed(l)));
    }
}

QString WorkflowRunTask::generateReport() const {
    QString res;
    res+="<table width='75%'>";
    res+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("Iterations")).arg(tr("Status")).arg(tr("Details"));
    foreach(Task* sub, getSubtasks()) {
        QString name = Qt::escape(sub->getTaskName());
        QString status = sub->hasError() ? tr("Failed") : sub->isCanceled() ? tr("Canceled") : tr("Finished");
        QString error = Qt::escape(sub->getError()).replace("\n", "<br>");
            //AppContext::getTaskScheduler()->getStateName(sub);
        if (sub->hasError()) {
            name = "<font color='red'>"+name+"</font>";
            status = "<font color='red'>"+status+"</font>";
        } else if (sub->isCanceled()) {
            status = "<font color='blue'>"+status+"</font>";
        } else {
            status = "<font color='green'>"+status+"</font>";
        }
        res+=QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(name).arg(status).arg(error);

        QStringList links = (static_cast<WorkflowIterationRunTask*>(sub))->getFiles();
        if(!links.isEmpty() && !sub->hasError()) {
            res += QString("<tr><td><i>%1</i></td></tr>").arg(tr("Output files:"));
        
            foreach(const QString &link, links) {
                res += QString("<tr><td><a href=\"%1\">%2</a></td></tr>").arg(link).arg(link);
            }
        }
        res+="<tr><td></td></tr>";
    }
    res+="</table>";
    return res;
}

QList<WorkerState> WorkflowRunTask::getState( Actor* actor) {
    QList<WorkerState> ret;
    foreach(Task* t, getSubtasks()) {
        WorkflowIterationRunTask* rt = qobject_cast<WorkflowIterationRunTask*>(t);
        ret << rt->getState(actor->getId());
    }
    return ret;
}

int WorkflowRunTask::getMsgNum( Link* l) {
    int ret = 0;
    foreach(Task* t, getSubtasks()) {
        WorkflowIterationRunTask* rt = qobject_cast<WorkflowIterationRunTask*>(t);
        ret += rt->getMsgNum(l);
    }
    return ret;
}

int WorkflowRunTask::getMsgPassed(Link* l) {
    int ret = 0;
    foreach(Task* t, getSubtasks()) {
        ret += qobject_cast<WorkflowIterationRunTask*>(t)->getMsgPassed(l);
    }
    return ret;
}

Task::ReportResult WorkflowRunTask::report() {
    propagateSubtaskError();
    if(hasError() && AppContext::getCMDLineRegistry()->hasParameter(OUTPUT_ERROR_OPTION)) {
        coreLog.info(QString("%1%2%1").arg(ERROR_KEYWORD).arg(getError()));
    }
    return ReportResult_Finished;
}

/*******************************************
* WorkflowIterationRunTask
*******************************************/
WorkflowIterationRunTask::WorkflowIterationRunTask(const Schema& sh, const Iteration& it) : 
Task(QString("%1").arg(it.name), TaskFlags_NR_FOSCOE), schema(new Schema()), scheduler(NULL) {
    rmap = HRSchemaSerializer::deepCopy(sh, schema);
    schema->applyConfiguration(it, rmap);
    
    if(schema->getDomain().isEmpty()) {
        QList<DomainFactory*> factories = WorkflowEnv::getDomainRegistry()->getAllEntries();
        assert(!factories.isEmpty());
        schema->setDomain(factories.isEmpty() ? "" : factories.at(0)->getId());
    }
    DomainFactory* df = WorkflowEnv::getDomainRegistry()->getById(schema->getDomain());
    if (!df) {
        stateInfo.setError(  tr("Unknown domain %1").arg(schema->getDomain()) );
        return;
    }
}

WorkflowIterationRunTask::~WorkflowIterationRunTask() {
    lmap.clear();
    DomainFactory* df = WorkflowEnv::getDomainRegistry()->getById(schema->getDomain());
    if (df) {
        df->destroy(scheduler, schema);
    }
    scheduler = NULL;
    delete schema;
}

void WorkflowIterationRunTask::prepare() {
    if( hasError() || isCanceled() ) {
        return;
    }
    
    DomainFactory* df = WorkflowEnv::getDomainRegistry()->getById(schema->getDomain());
    assert( df != NULL ); // checked in constructor
    foreach(Actor* a, schema->getProcesses()) {
        Worker* w = df->createWorker(a);
        if (!w) {
            stateInfo.setError( tr("Failed to create worker %1 %2 in domain %3")\
                .arg(a->getProto()->getId()).arg(a->getId()).arg(schema->getDomain()) );
            return;
        }
    }
    foreach(Link* l, schema->getFlows()) {
        CommunicationChannel* cc = df->createConnection(l);
        if (!cc) {
            stateInfo.setError(  tr("Failed to create connection %1 %2 in domain %3") ); //fixme
            return;
        }
        QStringList lst;
        lst << rmap.key(l->source()->owner()->getId());
        lst << (l->source()->getId());
        lst << rmap.key(l->destination()->owner()->getId());
        lst << (l->destination()->getId());
        QString key = lst.join("|");
        lmap.insert(key, cc);
    }
    scheduler = df->createScheduler(schema);
    if (!scheduler) {
        stateInfo.setError(  tr("Failed to create scheduler in domain %1").arg(df->getDisplayName()) );
        return;
    }
    scheduler->init();
    while(scheduler->isReady() && !isCanceled()) {
        Task* t = scheduler->tick();
        if (t) {
            addSubTask(t);
            break;
        }
    }
}

QList<Task*> WorkflowIterationRunTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> tasks;
    if (subTask->hasError()) {
        emit si_ticked();
        propagateSubtaskError();
        return tasks;
    }
    while(scheduler->isReady() && !isCanceled()) {
        Task* t = scheduler->tick();
        if (t) {
            tasks << t;
            break;
        }
    }
    emit si_ticked();
    return tasks;
}

DocumentFormat *getDocumentFormatByProtoId(QString protoId) {
    DocumentFormatId formatId;
    if (CoreLibConstants::WRITE_TEXT_PROTO_ID == protoId) {
        formatId = BaseDocumentFormats::PLAIN_TEXT;
    } 
    else if (CoreLibConstants::WRITE_FASTA_PROTO_ID == protoId) {
        formatId = BaseDocumentFormats::PLAIN_FASTA;
    }
    else if (CoreLibConstants::WRITE_GENBANK_PROTO_ID == protoId) {
        formatId = BaseDocumentFormats::PLAIN_GENBANK;
    }
    else if (CoreLibConstants::WRITE_CLUSTAL_PROTO_ID == protoId) {
        formatId = BaseDocumentFormats::CLUSTAL_ALN;
    }
    else if (CoreLibConstants::WRITE_STOCKHOLM_PROTO_ID == protoId) {
        formatId = BaseDocumentFormats::STOCKHOLM;
    }
    else if (CoreLibConstants::WRITE_FASTQ_PROTO_ID == protoId ) {
        formatId = BaseDocumentFormats::FASTQ;
    } else {
        return NULL;
    }

    return AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
}

static QStringList getOutputFiles(const QList<Actor*> & procs) {
    QStringList res;
    foreach(Actor *a, procs) {
        // commented because of UGENE-537
        //Attribute * formatAttr = a->getParameter(BaseAttributes::DOCUMENT_FORMAT_ATTRIBUTE().getId());
        foreach(Attribute *attr, a->getProto()->getAttributes()) {
            if(attr->getId() == BaseAttributes::URL_OUT_ATTRIBUTE().getId()) {
                QString str = a->getParameter(BaseAttributes::URL_OUT_ATTRIBUTE().getId())->getAttributeValue<QString>();
                // commented because of UGENE-537
                /*DocumentFormat *format = NULL;
                if (NULL != formatAttr) {
                    QString formatId = formatAttr->getAttributeValue<QString>();
                    format = AppContext::getDocumentFormatRegistry()->getFormatById(formatId);
                } else {
                    format = getDocumentFormatByProtoId(a->getProto()->getId());
                }
                if (NULL != format) {
                    str = GUrlUtils::ensureFileExt(GUrl(str), format->getSupportedDocumentFileExtensions()).getURLString();
                }*/
                QUrl url(str);
                if(url.isValid()) {
                    res << url.toString();
                }
            }
        }
    }
    return res;
}

Task::ReportResult WorkflowIterationRunTask::report() {
    if (scheduler) {
        scheduler->cleanup();
        if (!scheduler->isDone()) {
            if(!hasError() && !isCanceled()) {
                setError(tr("No workers are ready, while not all workers are done. Schema is broken?"));
            }
        }
    }
    fileLinks = getOutputFiles(schema->getProcesses());
    return ReportResult_Finished;
}

WorkerState WorkflowIterationRunTask::getState(const ActorId& id)
{
    if (scheduler) {
        return scheduler->getWorkerState(rmap.value(id));
    }
    return WorkerWaiting;
}

static QString getKey(Link * l) {
    QStringList lst;
    lst << (l->source()->owner()->getId());
    lst << (l->source()->getId());
    lst << (l->destination()->owner()->getId());
    lst << (l->destination()->getId());
    return lst.join("|");
}

int WorkflowIterationRunTask::getMsgNum( Link* l)
{
    CommunicationChannel* cc = lmap.value(getKey(l));
    if (cc) {
        return cc->hasMessage();
    }
    return 0;
}

int WorkflowIterationRunTask::getMsgPassed(Link* l) {
    CommunicationChannel * cc = lmap.value(getKey(l));
    if(cc != NULL) {
        return cc->takenMessages();
    }
    return 0;
}

QStringList WorkflowIterationRunTask::getFiles() const {
    return fileLinks;
}

/*******************************************
 * WorkflowRunInProcessTask
 *******************************************/
WorkflowRunInProcessTask::WorkflowRunInProcessTask(const Schema & sc, const QList<Iteration> & its) :
WorkflowAbstractRunner(tr("Execute workflow in separate process"), TaskFlags(TaskFlag_NoRun) | TaskFlag_ReportingIsSupported) {
    GCOUNTER(cvar, tvar, "WorkflowRunInProcessTask");
    assert(!its.isEmpty());
    foreach(const Iteration& it, its) {
        WorkflowIterationRunInProcessTask * t = new WorkflowIterationRunInProcessTask(sc, it);
        addSubTask(t);
    }
    setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS_AUTO);
    
    QTimer * timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SIGNAL(si_ticked()));
    timer->start(TICK_UPDATE_INTERVAL);
}

Task::ReportResult WorkflowRunInProcessTask::report() {
    propagateSubtaskError();
    return ReportResult_Finished;
}

QList<WorkerState> WorkflowRunInProcessTask::getState(Actor * a) {
    QList<WorkerState> ret;
    foreach(Task * t, getSubtasks()) {
        WorkflowIterationRunInProcessTask * it = qobject_cast<WorkflowIterationRunInProcessTask*>(t);
        ret << it->getState(a);
    }
    return ret;
}

int WorkflowRunInProcessTask::getMsgNum(Link * l) {
    int ret = 0;
    foreach(Task* t, getSubtasks()) {
        WorkflowIterationRunInProcessTask* rt = qobject_cast<WorkflowIterationRunInProcessTask*>(t);
        ret += rt->getMsgNum(l);
    }
    return ret;
}

int WorkflowRunInProcessTask::getMsgPassed(Link * l) {
    int ret = 0;
    foreach(Task* t, getSubtasks()) {
        WorkflowIterationRunInProcessTask* rt = qobject_cast<WorkflowIterationRunInProcessTask*>(t);
        ret += rt->getMsgPassed(l);
    }
    return ret;
}

QString WorkflowRunInProcessTask::generateReport() const {
    QString res;
    res+="<table width='75%'>";
    res+=QString("<tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("Iterations")).arg(tr("Status")).arg(tr("Details"));
    foreach(Task* sub, getSubtasks()) {
        QString name = Qt::escape(sub->getTaskName());
        QString status = sub->hasError() ? tr("Failed") : sub->isCanceled() ? tr("Canceled") : tr("Finished");
        QString error = Qt::escape(sub->getError()).replace("\n", "<br>");
        if (sub->hasError()) {
            name = "<font color='red'>"+name+"</font>";
            status = "<font color='red'>"+status+"</font>";
        } else if (sub->isCanceled()) {
            status = "<font color='blue'>"+status+"</font>";
        } else {
            status = "<font color='green'>"+status+"</font>";
        }
        res+=QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(name).arg(status).arg(error);
        QStringList links = (qobject_cast<WorkflowIterationRunInProcessTask*>(sub))->getFiles();
        if(!links.isEmpty() && !sub->hasError()) {
            res += QString("<tr><td><i>%1</i></td></tr>").arg(tr("Output files:"));
            foreach(const QString &link, links) {
                res += QString("<tr><td><a href=\"%1\">%2</a></td></tr>").arg(link).arg(link);
            }
        }
        res+="<tr><td></td></tr>";
    }
    res+="</table>";
    return res;
}

/*******************************************
 * WorkflowIterationRunInProcessTask
 *******************************************/
WorkflowIterationRunInProcessTask::WorkflowIterationRunInProcessTask(const Schema & sc, const Iteration & it) :
Task(QString("Execute iteration '%1'").arg(it.name), TaskFlags_NR_FOSCOE), schema(new Schema()), saveSchemaTask(NULL), monitor(NULL) {
    tempFile.setFileTemplate(QString("%1/XXXXXX.uwl").arg(QDir::tempPath()));
    if(!tempFile.open()) {
        setError(tr("Cannot create temporary file for saving schema!"));
        return;
    }
    Metadata meta; 
    meta.url = tempFile.fileName();
    tempFile.close();
    
    rmap = HRSchemaSerializer::deepCopy(sc, schema);
    schema->applyConfiguration(it, rmap);
    schema->getIterations().clear();
    saveSchemaTask = new SaveWorkflowTask(schema, meta, true);
    saveSchemaTask->setSubtaskProgressWeight(0);
    addSubTask(saveSchemaTask);
}

WorkflowIterationRunInProcessTask::~WorkflowIterationRunInProcessTask() {
    delete schema;
}

QList<Task*> WorkflowIterationRunInProcessTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    propagateSubtaskError();
    if(hasError() || isCanceled()) {
        return res;
    }
    if(saveSchemaTask == subTask) {
        monitor = new WorkflowRunInProcessMonitorTask(tempFile.fileName());
        monitor->setSubtaskProgressWeight(1);
        res << monitor;
    } else if(monitor == subTask) {
        monitor = NULL;
    } else {
        assert(false);
    }
    return res;
}

Task::ReportResult WorkflowIterationRunInProcessTask::report() {
    return ReportResult_Finished;
}

WorkerState WorkflowIterationRunInProcessTask::getState(Actor* a) {
    return monitor != NULL ? monitor->getState(rmap.value(a->getId())) : WorkerWaiting;
}

int WorkflowIterationRunInProcessTask::getMsgNum(Link * l) {
    if(monitor != NULL) {
        ActorId srcId = rmap.value(l->source()->owner()->getId());
        ActorId dstId = rmap.value(l->destination()->owner()->getId());
        assert(!srcId.isEmpty() && !dstId.isEmpty());
        return monitor->getMsgNum(QString("%1:%2").arg(srcId).arg(dstId));
    } else {
        return 0;
    }
}

int WorkflowIterationRunInProcessTask::getMsgPassed(Link * l) {
    if(monitor != NULL) {
        ActorId srcId = rmap.value(l->source()->owner()->getId());
        ActorId dstId = rmap.value(l->destination()->owner()->getId());
        assert(!srcId.isEmpty() && !dstId.isEmpty());
        return monitor->getMsgPassed(QString("%1:%2").arg(srcId).arg(dstId));
    } else {
        return 0;
    }
}

QStringList WorkflowIterationRunInProcessTask::getFiles() {
    return getOutputFiles(schema->getProcesses());
}

/***********************************
 * WorkflowRunInProcessMonitorTask
 ***********************************/
WorkflowRunInProcessMonitorTask::WorkflowRunInProcessMonitorTask(const QString & path) : 
Task(tr("Monitoring execution of workflow schema"), TaskFlag_NoRun), schemaPath(path), proc(new QProcess(this)){
    QStringList args;
    // FIXME: use defined constants!
    args << QString("--task=%1").arg(schemaPath);
    args << "--log-no-task-progress";
    args << "--log-level-info";
    args << QString("--%1").arg(OUTPUT_PROGRESS_OPTION);
    args << "--lang=en";
    args << QString("--%1").arg(OUTPUT_ERROR_OPTION);
    args << QString("--ini-file='%1'").arg(AppContext::getSettings()->fileName());
    connect(proc, SIGNAL(error(QProcess::ProcessError)), SLOT(sl_onError(QProcess::ProcessError)));
    connect(proc, SIGNAL(readyReadStandardOutput()), SLOT(sl_onReadStandardOutput()));
    QString cmdlineUgenePath(WorkflowSettings::getCmdlineUgenePath());
    assert(!cmdlineUgenePath.isEmpty());
    QString line = cmdlineUgenePath;
    foreach(const QString& arg, args ) {
        line+=" " + arg;
    }
    coreLog.details("Starting UGENE workflow in separate process: " + line);
    proc->start(cmdlineUgenePath, args);
    bool startedSuccessfully = proc->waitForStarted();
    if(!startedSuccessfully) {
        setError(tr("Cannot start process '%1'").arg(cmdlineUgenePath));
        return;
    }
}

WorkflowRunInProcessMonitorTask::~WorkflowRunInProcessMonitorTask() {
}

void WorkflowRunInProcessMonitorTask::sl_onError(QProcess::ProcessError err) {
    QString msg;
    switch(err) {
    case QProcess::FailedToStart:
        msg = tr("The process '%1' failed to start. Either the invoked program is missing, "
            "or you may have insufficient permissions to invoke the program").arg(WorkflowSettings::getCmdlineUgenePath());
        break;
    case QProcess::Crashed:
        msg = tr("The process '%1' crashed some time after starting successfully").arg(WorkflowSettings::getCmdlineUgenePath());
        break;
    case QProcess::WriteError:
    case QProcess::ReadError:
        msg = tr("Error occurred while reading from or writing to channel");
        break;
    default:
        msg = tr("Unknown error occurred");
    }
    setError(msg);
}

void WorkflowRunInProcessMonitorTask::writeLog(QString message) {
    QStringList lines = message.split(QChar('\n'));

    foreach(QString line, lines) {
        line = line.simplified();
        if ("" == line) {
            continue;
        }
        QRegExp rx("\\[.+\\]\\[INFO\\]");
        if (0 == rx.indexIn(line)) {
            QString logLine = line.right(line.length() - rx.matchedLength());
            logLine = logLine.simplified();
            if (!logLine.startsWith(OUTPUT_PROGRESS_TAG)
             && !logLine.startsWith(ERROR_KEYWORD)
             && !logLine.startsWith(STATE_KEYWORD)
             && !logLine.startsWith(MSG_NUM_KEYWORD)
             && !logLine.startsWith(MSG_PASSED_KEYWORD)) {
                logLine.prepend(SEP_PROCESS_PREFIX);
                taskLog.trace(logLine);
            }
        }
    }
}

void WorkflowRunInProcessMonitorTask::sl_onReadStandardOutput() {
    QString data(proc->readAllStandardOutput());
    writeLog(data);

    int errInd = data.indexOf(ERROR_KEYWORD);
    if(errInd >= 0) {
        int errIndEnd = data.indexOf(ERROR_KEYWORD, errInd + 1);
        assert(errIndEnd > errInd);
        if(errIndEnd > errInd) {
            setError(data.mid(errInd + ERROR_KEYWORD.size(), errIndEnd - errInd - ERROR_KEYWORD.size()));
        } else {
            assert(false);
            setError(data.mid(errInd + ERROR_KEYWORD.size() + 1));
        }
        return;
    }
    
    QStringList words = data.split(QRegExp("\\s+"));
    int sz = words.size();
    for(int i = 0; i < sz; ++i) {
        QString word = words.at(i);
        if(word.startsWith(OUTPUT_PROGRESS_TAG)) {
            QString numStr = words.at(i).mid(OUTPUT_PROGRESS_TAG.size());
            bool ok = false;
            int num = numStr.toInt(&ok);
            if(ok && num >= 0) {
                stateInfo.progress = qMin(num, 100);
            }
        } else if(word.startsWith(STATE_KEYWORD)) {
            QStringList stWords = word.split(":");
            if(stWords.size() == 3) {
                bool ok = false;
                int num = stWords.at(2).toInt(&ok);
                if(ok && num >= 0) {
                    WorkerState st = (WorkerState)num;
                    states[stWords.at(1)] = st;
                }
            }
        } else if(word.startsWith(MSG_NUM_KEYWORD)) {
            QStringList msgNumWords = word.split(":");
            if(msgNumWords.size() == 4) {
                bool ok = false;
                int num = msgNumWords.at(3).toInt(&ok);
                if(ok && num >= 0) {
                    msgNums[QString("%1:%2").arg(msgNumWords.at(1)).arg(msgNumWords.at(2))] = num;
                }
            }
        } else if(word.startsWith(MSG_PASSED_KEYWORD)) {
            QStringList msgPassedWords = word.split(":");
            if(msgPassedWords.size() == 4) {
                bool ok = false;
                int num = msgPassedWords.at(3).toInt(&ok);
                if(ok && num >= 0) {
                    msgPassed[QString("%1:%2").arg(msgPassedWords.at(1)).arg(msgPassedWords.at(2))] = num;
                }
            }
        }
    }
}

Task::ReportResult WorkflowRunInProcessMonitorTask::report() {
    assert(proc != NULL);
    if(hasError()) {
        return ReportResult_Finished;
    }
    if(isCanceled()) {
        proc->kill();
        return ReportResult_Finished;
    }
    QProcess::ProcessState st = proc->state();
    if(st == QProcess::Running) {
        return ReportResult_CallMeAgain;
    }
    return ReportResult_Finished;
}

WorkerState WorkflowRunInProcessMonitorTask::getState(const ActorId& id) {
    return states.value(id, WorkerWaiting);
}

int WorkflowRunInProcessMonitorTask::getMsgNum(const QString & ids) {
    return msgNums.value(ids, 0);
}

int WorkflowRunInProcessMonitorTask::getMsgPassed(const QString & ids) {
    return msgPassed.value(ids, 0);
}


/**************************
 * CheckCmdlineUgeneUtils
 **************************/
static QStringList generateCandidatesWithExt(const QString & path) {
    QStringList res;
    res << path;
    res << path + ".exe";
    return res;
}

static QStringList generateCandidates(const QString & prefix) {
    QStringList res;
    res << generateCandidatesWithExt(prefix + "/" + "ugene");
    res << generateCandidatesWithExt(prefix + "/" + "ugened");
    res << generateCandidatesWithExt(prefix + "/" + "ugenecl");
    res << generateCandidatesWithExt(prefix + "/" + "ugenecld");
    return res;
}

static QString getCmdlineUgenePath() {
    QString executableDir = QCoreApplication::applicationDirPath();
    QStringList candidates(generateCandidates(executableDir));
    foreach(const QString & candidate, candidates) {
        if(QFile::exists(candidate)) {
            return candidate;
        }
    }
    coreLog.info(WorkflowRunTask::tr("Command line UGENE path not found, a possibility to run in separate process will be disabled"));
    return QString();
}

void CheckCmdlineUgeneUtils::setCmdlineUgenePath() {
    QString path = getCmdlineUgenePath();
    WorkflowSettings::setCmdlineUgenePath(path);
    if(!WorkflowSettings::hasRunInSeparateProcess()) {
        WorkflowSettings::setRunInSeparateProcess(!path.isEmpty());
    }
}

}//namespace
