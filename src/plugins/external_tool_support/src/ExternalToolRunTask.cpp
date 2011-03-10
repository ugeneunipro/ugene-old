#include "ExternalToolRunTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <QtCore/QString>

namespace U2 {

ExternalToolRunTask::ExternalToolRunTask(const QString& _toolName, const QStringList& _arguments, ExternalToolLogParser*  _logParser)
: Task(_toolName + " run task", TaskFlag_None), arguments(_arguments), 
  logParser(_logParser), toolName(_toolName), logData(NULL), externalToolProcess(NULL)
{
    ExternalTool * tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    if(tool == NULL) {
        setError(tr("Undefined tool: '%1'").arg(toolName));
        return;
    }
    if(tool->getPath().isEmpty()) {
        setError(tr("Path for '%1' tool not set").arg(toolName));
        return;
    }
    if(!tool->isValid()) {
//        setError(tr("'%1' tool located in '%2' not exists or not valid").arg(toolName).arg(tool->getPath()));
//        return;
    }
    program=tool->getPath();
    coreLog.trace("Creating run task for: " + toolName);
    logData=(char*)malloc(1000*sizeof(char));
}

ExternalToolRunTask::~ExternalToolRunTask(){
    free(logData);
    delete externalToolProcess;
}

void ExternalToolRunTask::prepare(){
    if(hasErrors() || isCanceled()) {
        return;
    }
    algoLog.trace("Program executable: "+program);
    algoLog.trace("Program arguments: "+arguments.join(" "));
}
void ExternalToolRunTask::run(){
    if(hasErrors() || isCanceled()) {
        return;
    }
    externalToolProcess=new QProcess();//???
    connect(externalToolProcess,SIGNAL(readyReadStandardOutput()),SLOT(sl_onReadyToReadLog()));
    connect(externalToolProcess,SIGNAL(readyReadStandardError()),SLOT(sl_onReadyToReadErrLog()));
    externalToolProcess->start(program, arguments);
    //externalToolProcess->state()
    if(!externalToolProcess->waitForStarted(3000)){
        if(AppContext::getExternalToolRegistry()->getByName(toolName)->isValid()){
            stateInfo.setError(tr("Can not run %1 tool.").arg(toolName));
        }else{
            stateInfo.setError(tr("Can not run %1 tool. May be tool path '%2' not valid?")
                               .arg(toolName)
                               .arg(AppContext::getExternalToolRegistry()->getByName(toolName)->getPath()));
        }
        return;
    }
    while(!externalToolProcess->waitForFinished(1000)){
        if (isCanceled()) {
            cancelProcess();
        }
    }
}
Task::ReportResult ExternalToolRunTask::report(){
    return ReportResult_Finished;
}
void ExternalToolRunTask::cancelProcess(){
    externalToolProcess->kill();
}

void ExternalToolRunTask::sl_onReadyToReadLog(){
    if(externalToolProcess->readChannel() == QProcess::StandardError)
        externalToolProcess->setReadChannel(QProcess::StandardOutput);
    int numberReadChars=externalToolProcess->read(logData,1000);
    while(numberReadChars > 0){
        //call log parser
        logParser->parseOutput(QString(logData).left(numberReadChars));
        numberReadChars=externalToolProcess->read(logData,1000);
    }
    stateInfo.progress=logParser->getProgress();
    emit si_progressChanged();
}

void ExternalToolRunTask::sl_onReadyToReadErrLog(){
    if(externalToolProcess->readChannel() == QProcess::StandardOutput)
        externalToolProcess->setReadChannel(QProcess::StandardError);
    int numberReadChars=externalToolProcess->read(logData,1000);
    while(numberReadChars > 0){
        //call log parser
        logParser->parseErrOutput(QString(logData).left(numberReadChars));
        numberReadChars=externalToolProcess->read(logData,1000);
    }
    stateInfo.setError(logParser->getLastError());
    stateInfo.progress=logParser->getProgress();
    emit si_progressChanged();
}
////////////////////////////////////////
//ExternalToolLogParser
ExternalToolLogParser::ExternalToolLogParser() {
    progress=-1;
    lastLine="";
    lastErrLine="";
}
void ExternalToolLogParser::parseOutput(const QString& partOfLog){
    lastPartOfLog=partOfLog.split(QChar('\n'));
    lastPartOfLog.first()=lastLine+lastPartOfLog.first();
    lastLine=lastPartOfLog.takeLast();
    foreach(QString buf, lastPartOfLog){
        if(buf.contains("error",Qt::CaseInsensitive)){
            ioLog.error(buf);
            lastErrLine = buf;
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
            ioLog.error(buf);
            lastErrLine = buf;
        }else{
            ioLog.trace(buf);
        }
    }
}
}//namespace
