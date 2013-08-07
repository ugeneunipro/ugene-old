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
#include "ExternalToolSupportSettings.h"
#include "python/PythonSupport.h"

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

    externalToolProcess=NULL;
    isValid=false;
}

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName, const QString& path) :
        Task(_toolName + " validate task", TaskFlag_None), toolName(_toolName), errorMsg("")
{
    program=path;

    externalToolProcess=NULL;
    isValid=false;
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
                stateInfo.setError(QString("The tool %1 that runs %2 is not installed. Please set the path to the executable file of the"
                    " tool in the External Tools settings. Some of the tools may be located in UGENE/Tools directory").arg(origianlValidation.toolRunnerProgram).arg(toolName));
            }else{
                origianlValidation.arguments.prepend(origianlValidation.executableFile);
                origianlValidation.executableFile = stool->getPath();
            }
        }
        validations.append(origianlValidation);
        coreLog.trace("Creating validation task for: " + toolName);
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

        externalToolProcess->start(validation.executableFile, validation.arguments);
        bool started = externalToolProcess->waitForStarted(3000);

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

const QString ExternalToolSearchAndValidateTask::TOOLS = "tools";

ExternalToolSearchAndValidateTask::ExternalToolSearchAndValidateTask(const QString& _toolName, const QString& _path) :
    Task(_toolName + " search and validate task", TaskFlags(TaskFlag_CancelOnSubtaskCancel | TaskFlag_NoRun)),
    toolName(_toolName),
    isValid(false),
    toolIsFound(true),
    path(_path),
    validateTask(NULL) {
}

void ExternalToolSearchAndValidateTask::prepare() {
    ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    if (!tool) {
        setError(QString("Tool \'%1\' wasn't found in registry").arg(toolName));
        return;
    }

    if (!path.isEmpty()) {
        // Try to validate tool on the predefined path
        toolPaths << path;
    } else {
        // Search and validate

        // Check the path from the settings
        if (!tool->getPath().isEmpty()) {
            toolPaths << tool->getPath();
        }

        // Search for the tool in an application dir
        QDir appDir(QCoreApplication::applicationDirPath());
        QStringList entryList = appDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QString toolsDir;
        foreach(const QString& dirName, entryList) {
            if (dirName.contains(TOOLS)) {
                toolsDir = appDir.absolutePath()+ "/" + dirName;
                break;
            }
        }

        if (!toolsDir.isEmpty()) {
            QString exeName = tool->getExecutableFileName();
            bool fileNotFound = true;
            LimitedDirIterator it (toolsDir);
            while (it.hasNext() && fileNotFound) {
                it.next();
                QString toolPath(it.filePath() + "/" + exeName);
                QFileInfo info(toolPath);
                if (info.exists() && info.isFile()) {
                    toolPaths << QDir::toNativeSeparators(toolPath);
                    fileNotFound = false;
                }
            }
        }

        // Search for the tool in the PATH variable
        QStringList envList = QProcess::systemEnvironment();
        if (envList.indexOf(QRegExp("PATH=.*",Qt::CaseInsensitive)) >= 0) {
            QString pathEnv = envList.at(envList.indexOf(QRegExp("PATH=.*",Qt::CaseInsensitive)));
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            QStringList paths = pathEnv.split("=").at(1).split(":");
#else
#ifdef Q_OS_WIN
            QStringList paths = pathEnv.split("=").at(1).split(";");
#else
            QStringList paths;
#endif
#endif
            // UGENE-1781: Remove python external tool search in PATH
            // It should be fixed without crutches.
            if (tool->getName() != PYTHON_TOOL_NAME) {
                foreach (const QString& curPath, paths) {
                    QString exePath = curPath + "/" + tool->getExecutableFileName();
                    QFileInfo fileExe(exePath);
                    if (fileExe.exists()) {
                        toolPaths << exePath;
                    }
                }
            }
        }

    }

    if (!toolPaths.isEmpty()) {
        validateTask = new ExternalToolValidateTask(toolName, toolPaths.first());
        addSubTask(validateTask);
    } else {
        toolIsFound = false;
    }
}

QList<Task*> ExternalToolSearchAndValidateTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> subTasks;

    if (subTask->isCanceled()) {
        return subTasks;
    }

    if (validateTask == subTask) {
        if (validateTask->isValidTool()) {
            isValid = validateTask->isValidTool();
            program = validateTask->getToolPath();
            version = validateTask->getToolVersion();
        } else {
            errorMsg = validateTask->getError();
            program = validateTask->getToolPath();
            toolPaths.removeAll(validateTask->getToolPath());
            if (!toolPaths.isEmpty()) {
                validateTask = new ExternalToolValidateTask(toolName, toolPaths.first());
                subTasks << validateTask;
            }
        }
    }

    return subTasks;
}

Task::ReportResult ExternalToolSearchAndValidateTask::report() {
    ExternalTool* tool = AppContext::getExternalToolRegistry()->getByName(toolName);
    SAFE_POINT(NULL != tool, QString("Tool \'%1\' wasn't found in registry").arg(toolName), ReportResult_Finished);
    if (!isValid && toolIsFound) {
        if (errorMsg.isEmpty()) {
            stateInfo.setError(tr("Can not find expected message."
                                  "<br>It is possible that the specified executable file "
                                  "<i>%1</i> for %2 tool is invalid. "
                                  "You can change the path to the executable file "
                                  "in the external tool settings in the global "
                                  "preferences.").arg(program).arg(toolName));
        } else {
            stateInfo.setError(errorMsg);
        }
    }
    return ReportResult_Finished;
}

bool ExternalToolSearchAndValidateTask::isValidTool() {
    return isValid;
}
QString ExternalToolSearchAndValidateTask::getToolName() {
    return toolName;
}
QString ExternalToolSearchAndValidateTask::getToolPath() {
    return program;
}
QString ExternalToolSearchAndValidateTask::getToolVersion() {
    return version;
}

ExternalToolsValidateTask::ExternalToolsValidateTask(const QList<Task*> &_tasks) :
    SequentialMultiTask(tr("Checking external tools for the first time"), _tasks, TaskFlags(TaskFlag_NoRun | TaskFlag_CancelOnSubtaskCancel)) {
}

QList<Task*> ExternalToolsValidateTask::onSubTaskFinished(Task* subTask) {
    if (subTask->hasError()) {
        taskLog.error(subTask->getTaskName() + tr(" failed: ") + subTask->getError());
    }
    return SequentialMultiTask::onSubTaskFinished(subTask);
}

}//namespace
