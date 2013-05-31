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

#include "ExternalToolValidateTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/Log.h>

#include <U2Lang/WorkflowUtils.h>

#include <QtCore/QString>

namespace U2 {

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName) :
        Task(_toolName + " validate task", TaskFlag_None), toolName(_toolName), errorMsg("")
{
    ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    assert(tool);
    if (tool){
        program=tool->getPath();
    }else{
        program = "";
    }
}

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName, const QString& path) :
        Task(_toolName + " validate task", TaskFlag_None), toolName(_toolName), errorMsg("")
{
    program=path;
}
ExternalToolValidateTask::~ExternalToolValidateTask(){
    delete externalToolProcess;
    externalToolProcess=NULL;
}
void ExternalToolValidateTask::prepare(){
    
}
void ExternalToolValidateTask::run(){
    ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    assert(tool);
    if (tool){
        assert(program!="");
        validations.append(tool->getToolAdditionalValidations());
        ExternalToolValidation origianlValidation = tool->getToolValidation();
        origianlValidation.executableFile = program;
        if (!origianlValidation.toolRunnerProgram.isEmpty()){
            ScriptingToolRegistry* stregister = AppContext::getScriptingToolRegistry();
            SAFE_POINT_EXT(stregister != NULL, setError("No scripting tool registry"), );
            ScriptingTool* stool = stregister->getByName(origianlValidation.toolRunnerProgram);
            if(!stool || stool->getPath().isEmpty()){
                stateInfo.setError(QString("The tool %1 that runs %2 is not installed. Please set the path of the tool in the External Tools settings").arg(origianlValidation.toolRunnerProgram).arg(toolName));
            }else{
                origianlValidation.arguments.prepend(origianlValidation.executableFile);
                origianlValidation.executableFile = stool->getPath();
            }
        }
        validations.append(origianlValidation);
        coreLog.trace("Creating validation task for: " + toolName);
        externalToolProcess=NULL;
        isValid=false;
        checkVersionRegExp=tool->getVersionRegExp();
        version="unknown";

    }
    algoLog.trace("Program executable: "+program);
    assert(!validations.isEmpty());
    algoLog.trace("Program arguments: "+validations.last().arguments.join(" "));
    if (stateInfo.hasError()){
        return;
    }
    foreach(const ExternalToolValidation& validation, validations){
        if(externalToolProcess != NULL){
            delete externalToolProcess;
            externalToolProcess=NULL;
        }
        
        externalToolProcess=new QProcess();
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        externalToolProcess->setProcessEnvironment(env);

        bool started = WorkflowUtils::startExternalProcess(externalToolProcess, validation.executableFile, validation.arguments);

        if(!started){
            errorMsg = validation.possibleErrorsDescr.value(ExternalToolValidation::DEFAULT_DESCR_KEY, "");
            if(!errorMsg.isEmpty()){
                stateInfo.setError(errorMsg);
            }else{
                stateInfo.setError(tr("Tool does not start.<br>It is possible that the specified executable file <i>%1</i> for %2 tool is invalid. You can change the path to the executable file in the external tool settings in the global preferences.").arg(program).arg(toolName));
            }
            isValid=false;
            return;
        }
        while(!externalToolProcess->waitForFinished(1000)){
            if (isCanceled()) {
                cancelProcess();
            }
        }
        if(!parseLog(validation)){
            return;
        }
        
        if (!isValid){
            return;
        }
    }
}
Task::ReportResult ExternalToolValidateTask::report(){
    if(!isValid && !stateInfo.hasError()){
        if (errorMsg.isEmpty()){
            stateInfo.setError(tr("Can not find expected message.<br>It is possible that the specified executable file <i>%1</i> for %2 tool is invalid. You can change the path to the executable file in the external tool settings in the global preferences.").arg(program).arg(toolName));
        }else{
            stateInfo.setError(errorMsg);
        }
    }
    return ReportResult_Finished;
}
void ExternalToolValidateTask::cancelProcess(){
    externalToolProcess->kill();
}

bool ExternalToolValidateTask::parseLog(const ExternalToolValidation& validation){
    errorMsg = validation.possibleErrorsDescr.value(ExternalToolValidation::DEFAULT_DESCR_KEY, "");

    QString errLog=QString(externalToolProcess->readAllStandardError());
    if(!errLog.isEmpty()){
        if(errLog.contains(validation.expectedMsg)){
            isValid=true;
            checkVersion(errLog);
        }else{
            isValid=false;
            foreach(const QString& errStr, validation.possibleErrorsDescr.keys()){
                if(errLog.contains(errStr)){
                    errorMsg = validation.possibleErrorsDescr[errStr];
                    return false;
                }
            }
        }
    }
    QString log=QString(externalToolProcess->readAllStandardOutput());
    if(!log.isEmpty()){
        if(log.contains(validation.expectedMsg)){
            isValid=true;
            checkVersion(log);
        }else{
            isValid=false;
            foreach(const QString& errStr, validation.possibleErrorsDescr.keys()){
                if(log.contains(errStr)){
                    errorMsg = validation.possibleErrorsDescr[errStr];
                    return false;
                }
            }
        }
    }
    return true;
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
