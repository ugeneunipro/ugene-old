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

#include "ExternalToolValidateTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <QtCore/QString>

namespace U2 {

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName) :
        Task(_toolName + " validate task", TaskFlag_None), toolName(_toolName)
{
    program=AppContext::getExternalToolRegistry()->getByName(toolName)->getPath();
    assert(program!="");
    arguments=AppContext::getExternalToolRegistry()->getByName(toolName)->getValidationArguments();
    expectedMessage=AppContext::getExternalToolRegistry()->getByName(toolName)->getValidMessage();
    if(expectedMessage==""){
        assert(NULL);
    }
    coreLog.trace("Creating validation task for: " + toolName);
    externalToolProcess=NULL;
    isValid=false;
    checkVersionRegExp=AppContext::getExternalToolRegistry()->getByName(toolName)->getVersionRegExp();
    version="unknown";
}

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName, const QString& path) :
        Task(_toolName + " validate task", TaskFlag_None), toolName(_toolName)
{
    program=path;
    assert(program!="");
    arguments=AppContext::getExternalToolRegistry()->getByName(toolName)->getValidationArguments();
    expectedMessage=AppContext::getExternalToolRegistry()->getByName(toolName)->getValidMessage();
    if(expectedMessage==""){
        assert(NULL);
    }
    coreLog.trace("Creating validation task for: " + toolName);
    externalToolProcess=NULL;
    isValid=false;
    checkVersionRegExp=AppContext::getExternalToolRegistry()->getByName(toolName)->getVersionRegExp();
    version="unknown";
}
ExternalToolValidateTask::~ExternalToolValidateTask(){
    delete externalToolProcess;
    externalToolProcess=NULL;
}
void ExternalToolValidateTask::prepare(){
    algoLog.trace("Program executable: "+program);
    algoLog.trace("Program arguments: "+arguments.join(" "));
}
void ExternalToolValidateTask::run(){
    externalToolProcess=new QProcess();
    externalToolProcess->start(program, arguments);
    if(!externalToolProcess->waitForStarted(3000)){
        stateInfo.setError(tr("It is possible that the specified executable file for  %1 tool is invalid. Tool does not start.").arg(toolName));
        isValid=false;
        return;
    }
    while(!externalToolProcess->waitForFinished(1000)){
        if (isCanceled()) {
            cancelProcess();
        }
    }
    parseLog();
}
Task::ReportResult ExternalToolValidateTask::report(){
    if(!isValid && !stateInfo.hasError()){
        stateInfo.setError(tr("It is possible that the specified executable file for  %1 tool is invalid. Can not find expected message.").arg(toolName));
    }
    return ReportResult_Finished;
}
void ExternalToolValidateTask::cancelProcess(){
    externalToolProcess->kill();
}

void ExternalToolValidateTask::parseLog(){
    QString log=QString(externalToolProcess->readAllStandardOutput());
    if(!log.isEmpty()){
        if(log.contains(expectedMessage)){
            isValid=true;
            checkVersion(log);
        }
    }
    QString errLog=QString(externalToolProcess->readAllStandardError());
    if(!errLog.isEmpty()){
        if(errLog.contains(expectedMessage)){
            isValid=true;
            checkVersion(errLog);
        }
    }
}

void ExternalToolValidateTask::checkVersion(const QString &partOfLog){
    QStringList lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));
    foreach(QString buf, lastPartOfLog){
        if(buf.contains(checkVersionRegExp)){
            assert(checkVersionRegExp.indexIn(buf)>-1);
            checkVersionRegExp.indexIn(buf);
            version=checkVersionRegExp.cap(1);
            return;
        }
    }
}
}//namespace
