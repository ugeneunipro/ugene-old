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

#include "ExternalToolRunTask.h"

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>
#include <U2Core/Log.h>
#include <U2Core/SaveDocumentTask.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/U2SafePoints.h>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>

namespace U2 {

#define WIN_LAUNCH_CMD_COMMAND "cmd /C "
#define START_WAIT_MSEC 3000

ExternalToolRunTask::ExternalToolRunTask(const QString &_toolName, const QStringList &_arguments, ExternalToolLogParser *_logParser, const QString &_workingDirectory, const QStringList &_additionalPaths)
: Task(_toolName + " run task", TaskFlag_None),
  arguments(_arguments),
  logParser(_logParser),
  toolName(_toolName),
  workingDirectory(_workingDirectory),
  additionalPaths(_additionalPaths),
  externalToolProcess(NULL),
  helper(NULL),
  listener(NULL)
{
    coreLog.trace("Creating run task for: " + toolName);
}

ExternalToolRunTask::~ExternalToolRunTask(){
    delete externalToolProcess;
}

void ExternalToolRunTask::run(){
    if (hasError() || isCanceled()) {
        return;
    }

    ProcessRun pRun = ExternalToolSupportUtils::prepareProcess(toolName, arguments, workingDirectory, additionalPaths, stateInfo, listener);
    CHECK_OP(stateInfo, );
    externalToolProcess = pRun.process;

    helper.reset(new ExternalToolRunTaskHelper(this));
    if(NULL != listener) {
        helper->addOutputListener(listener);
    }
    try {
        externalToolProcess->start(pRun.program, pRun.arguments);
        bool started = externalToolProcess->waitForStarted(START_WAIT_MSEC);

        if (!started){
            ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolName);
            if (tool->isValid()){
                stateInfo.setError(tr("Can not run %1 tool.").arg(toolName));
            } else {
                stateInfo.setError(tr("Can not run %1 tool. May be tool path '%2' not valid?")
                                   .arg(toolName)
                                   .arg(AppContext::getExternalToolRegistry()->getByName(toolName)->getPath()));
            }
            return;
        }
        while(!externalToolProcess->waitForFinished(1000)){
            if (isCanceled()) {
                externalToolProcess->kill();
            }
        }
    } catch(...) {
        setError(QString("%1 tool finished unexpectedly.").arg(toolName));
    }

    {
        int exitCode = externalToolProcess->exitCode();
        if(exitCode != EXIT_SUCCESS && !hasError()) {
            setError(tr("%1 tool exited with code %2").arg(toolName).arg(exitCode));
        } else {
            algoLog.details(tr("Tool %1 finished successfully").arg(toolName));
        }
    }
}
void ExternalToolRunTask::addOutputListener(ExternalToolListener* outputListener) {
    if(helper) {
        helper->addOutputListener(outputListener);
    }
    listener = outputListener;
}

////////////////////////////////////////
//ExternalToolSupportTask
void ExternalToolSupportTask::setListenerForTask(ExternalToolRunTask* runTask, int listenerNumber) {
    if(listeners.size() > listenerNumber) {
        runTask->addOutputListener(listeners.at(listenerNumber));
    }
}

void ExternalToolSupportTask::setListenerForHelper(ExternalToolRunTaskHelper* helper, int listenerNumber) {
    if(listeners.size() > listenerNumber) {
        helper->addOutputListener(listeners.at(listenerNumber));
    }
}

////////////////////////////////////////
//ExternalToolRunTaskHelper
ExternalToolRunTaskHelper::ExternalToolRunTaskHelper(ExternalToolRunTask* t)
: process(t->externalToolProcess), logParser(t->logParser), os(t->stateInfo), listener(NULL)
{
    logData.resize(1000);
    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(sl_onReadyToReadLog()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(sl_onReadyToReadErrLog()));
}

ExternalToolRunTaskHelper::ExternalToolRunTaskHelper(QProcess *_process, ExternalToolLogParser *_logParser, U2OpStatus &_os)
: process(_process), logParser(_logParser), os(_os), listener(NULL)
{
    logData.resize(1000);
    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(sl_onReadyToReadLog()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(sl_onReadyToReadErrLog()));
}

void ExternalToolRunTaskHelper::sl_onReadyToReadLog(){
    QMutexLocker locker(&logMutex);

    CHECK(NULL != process, );
    if (process->readChannel() == QProcess::StandardError) {
        process->setReadChannel(QProcess::StandardOutput);
    }
    int numberReadChars = process->read(logData.data(), logData.size());
    while(numberReadChars > 0){
        //call log parser
        QString line = QString::fromLocal8Bit(logData.constData(), numberReadChars);
        logParser->parseOutput(line);
        if(NULL != listener) {
            listener->addNewLogMessage(line, OUTPUT_LOG);
        }
        numberReadChars = process->read(logData.data(), logData.size());
    }
    os.setProgress(logParser->getProgress());
}

void ExternalToolRunTaskHelper::sl_onReadyToReadErrLog(){
    QMutexLocker locker(&logMutex);

    CHECK(NULL != process, );
    if (process->readChannel() == QProcess::StandardOutput) {
        process->setReadChannel(QProcess::StandardError);
    }
    int numberReadChars = process->read(logData.data(), logData.size());
    while(numberReadChars > 0){
        //call log parser
        QString line = QString::fromLocal8Bit(logData.constData(), numberReadChars);
        logParser->parseErrOutput(line);
        if(NULL != listener) {
            listener->addNewLogMessage(line, ERROR_LOG);
        }
        numberReadChars = process->read(logData.data(), logData.size());
    }
    QString lastErr = logParser->getLastError();
    if (!lastErr.isEmpty()) {
        os.setError(lastErr);
    }
    os.setProgress(logParser->getProgress());
}

void ExternalToolRunTaskHelper::addOutputListener(ExternalToolListener* _listener) {
    listener = _listener;
}

////////////////////////////////////////
//ExternalToolLogParser
ExternalToolLogParser::ExternalToolLogParser() {
    progress=-1;
    lastLine="";
    lastErrLine="";
    lastError="";
}
void ExternalToolLogParser::parseOutput(const QString& partOfLog){
    lastPartOfLog=partOfLog.split(QChar('\n'));
    lastPartOfLog.first()=lastLine+lastPartOfLog.first();
    lastLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        
if(buf.contains("error",Qt::CaseInsensitive)){
            setLastError(buf);
        }else{
            ioLog.trace(buf);
        }
    }
}

void ExternalToolLogParser::parseErrOutput(const QString& partOfLog){
    lastPartOfLog=partOfLog.split(QChar('\n'));
    lastPartOfLog.first()=lastErrLine+lastPartOfLog.first();
    lastErrLine=lastPartOfLog.takeLast();
    foreach(const QString& buf, lastPartOfLog){
        if(buf.contains("error",Qt::CaseInsensitive)){
            setLastError(buf);
        }else{
            ioLog.trace(buf);
        }
    }
}

void ExternalToolLogParser::setLastError(const QString &value) {
    if (!value.isEmpty()) {
        ioLog.error(value);
    }
    lastError = value;
}

////////////////////////////////////////
//ExternalToolSupportUtils
void ExternalToolSupportUtils::removeTmpDir( const QString& tmpDirUrl, U2OpStatus& os) {
    if (tmpDirUrl.isEmpty()) {
        os.setError(tr("Can not remove temporary directory: path is empty."));
        return;
    }
    QDir tmpDir(tmpDirUrl);
    foreach(const QString& file, tmpDir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries)){
        if (!tmpDir.remove(file)) {
            os.setError(tr("Can not remove files from temporary directory."));
            return;
        }
    }
    if (!tmpDir.rmdir(tmpDir.absolutePath())){
        os.setError(tr("Can not remove directory for temporary files."));
    }
}

QString ExternalToolSupportUtils::createTmpDir(const QString &prePath, const QString &domain, U2OpStatus &os) {
    int i = 0;
    while (true) {
        QString tmpDirName = QString("d_%1").arg(i);
        QString tmpDirPath = prePath + "/" + domain + "/" + tmpDirName;
        QDir tmpDir(tmpDirPath);

        if (!tmpDir.exists()) {
            if (!QDir().mkpath(tmpDirPath)) {
                os.setError(tr("Can not create directory for temporary files: %1").arg(tmpDirPath));
            } 
            return tmpDir.absolutePath();
        }
        i++;
    }
}

QString ExternalToolSupportUtils::createTmpDir(const QString& domain, U2OpStatus& os) {
    QString tmpDirPath = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath();
    return createTmpDir(tmpDirPath, domain, os);
}

Document * ExternalToolSupportUtils::createAnnotationsDocument(const QString &filePath,
                             const DocumentFormatId &format,
                             const QList<SharedAnnotationData> &anns,
                             U2OpStatus &os) {
     Document *doc = NULL;
     { // create document
         DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(format);
         IOAdapterFactory *iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
         doc = f->createNewLoadedDocument(iof, filePath, os);
         CHECK_OP(os, NULL);
         doc->setDocumentOwnsDbiResources(false);
     }

     { // add annotations object
         AnnotationTableObject *aobj = new AnnotationTableObject("anns");
         foreach(const SharedAnnotationData& ann, anns) {
             aobj->addAnnotation(new Annotation(ann));
         }
         doc->addObject(aobj);
     }
     return doc;
}

void ExternalToolSupportUtils::appendExistingFile(const QString &path, QStringList &files) {
    GUrl url(path);
    if (QFile::exists(url.getURLString())) {
        files << url.getURLString();
    }
}

bool ExternalToolSupportUtils::startExternalProcess(QProcess *process, const QString &program, const QStringList &arguments) {
    process->start(program, arguments);
    bool started = process->waitForStarted(START_WAIT_MSEC);

#ifdef Q_OS_WIN32
    if(!started) {
        QString execStr = WIN_LAUNCH_CMD_COMMAND + program;
        foreach (const QString arg, arguments) {
            execStr += " " + arg;
        }
        process->start(execStr);
        coreLog.trace(tr("Can't run an executable file \"%1\" as it is. Try to run it as a cmd line command: \"%2\"")
            .arg(program).arg(execStr));
        started = process->waitForStarted(START_WAIT_MSEC);
    }
#endif

    return started;
}

ProcessRun ExternalToolSupportUtils::prepareProcess(const QString &toolName, const QStringList &arguments, const QString &workingDirectory, const QStringList &additionalPaths, U2OpStatus &os, ExternalToolListener* listener) {
    ProcessRun result;
    result.process = NULL;
    result.arguments = arguments;

    ExternalTool *tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    if (NULL == tool) {
        os.setError(tr("Undefined tool: '%1'").arg(toolName));
        return result;
    }
    if (tool->getPath().isEmpty()) {
        os.setError(tr("Path for '%1' tool not set").arg(toolName));
        return result;
    }
    result.program = tool->getPath();
    QString toolRunnerProgram = tool->getToolRunnerProgram();

    if (!toolRunnerProgram.isEmpty()){
        ScriptingToolRegistry *stregister = AppContext::getScriptingToolRegistry();
        SAFE_POINT_EXT(NULL != stregister, os.setError("No scripting tool registry"), result);
        ScriptingTool *stool = stregister->getByName(toolRunnerProgram);
        if (NULL == stool || stool->getPath().isEmpty()){
            os.setError(QString("The tool %1 that runs %2 is not installed. Please set the path of the tool in the External Tools settings").arg(toolRunnerProgram).arg(toolName));
            return result;
        }
        result.arguments.prepend(result.program);
        result.program = stool->getPath();
    }

    QProcessEnvironment processEnvironment = QProcessEnvironment::systemEnvironment();
    foreach (QString path, additionalPaths) {
        path = path + ":" + processEnvironment.value("PATH");
        processEnvironment.insert("PATH", path);
    }

    result.process = new QProcess();
    result.process->setProcessEnvironment(processEnvironment);
    if (!workingDirectory.isEmpty()){
        result.process->setWorkingDirectory(workingDirectory);
        algoLog.details(tr("Working directory is \"%1\"").arg(result.process->workingDirectory()));
    }
    algoLog.details(tr("Launching %1 tool: %2 %3").arg(toolName).arg(result.program).arg(arguments.join(" ")));
    if(NULL != listener) {
        listener->setToolName(toolName);
        listener->addNewLogMessage(result.program, PROGRAM_PATH);
        listener->addNewLogMessage(arguments.join("\n"), ARGUMENTS);
    }
    return result;
}

}//namespace
