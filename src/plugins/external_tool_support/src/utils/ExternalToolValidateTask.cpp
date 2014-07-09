/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QString>

#include <U2Core/AppContext.h>
#include <U2Core/AppResources.h>
#include <U2Core/ExternalToolRegistry.h>
#include <U2Core/Log.h>
#include <U2Core/ScriptingToolRegistry.h>
#include <U2Core/Timer.h>
#include <U2Core/U2SafePoints.h>

#include <U2Lang/WorkflowUtils.h>

#include "ExternalToolSearchTask.h"
#include "ExternalToolSupportSettings.h"
#include "ExternalToolValidateTask.h"
#include "python/PythonSupport.h"

namespace U2 {

ExternalToolValidateTask::ExternalToolValidateTask(const QString& _toolName, TaskFlags flags) :
    Task(tr("%1 validate task").arg(_toolName), flags),
    toolName(_toolName),
    isValid(false) {
}

ExternalToolJustValidateTask::ExternalToolJustValidateTask(const QString& _toolName, const QString& path) :
    ExternalToolValidateTask(_toolName, TaskFlag_None),
    externalToolProcess(NULL),
    isAlreadyFinished(false),
    startTime(0)
{
    toolPath = path;
    SAFE_POINT_EXT(!toolPath.isEmpty(), setError(tr("Tool's path is empty")), );
}

ExternalToolJustValidateTask::~ExternalToolJustValidateTask() {
    delete externalToolProcess;
    externalToolProcess = NULL;
}

void ExternalToolJustValidateTask::run() {
    startTime = TimeCounter::getCounter();

    ExternalToolRegistry* etRegistry = AppContext::getExternalToolRegistry();
    SAFE_POINT(etRegistry, "An external tool registry is NULL", );
    ExternalTool* tool = etRegistry->getByName(toolName);
    SAFE_POINT(tool, QString("External tool '%1' isn't found in the registry").arg(toolName), );

    QFileInfo info(toolPath);
    CHECK_EXT(info.exists(), setError(tr("Tool's executable isn't exists")), );

    validations.append(tool->getToolAdditionalValidations());
    ExternalToolValidation originalValidation = tool->getToolValidation();
    originalValidation.executableFile = toolPath;

    if (!originalValidation.toolRunnerProgram.isEmpty()) {
        ScriptingToolRegistry* stRegistry = AppContext::getScriptingToolRegistry();
        SAFE_POINT_EXT(stRegistry, setError(tr("Scripting tool registry is NULL")), );
        ScriptingTool* stool = stRegistry->getByName(originalValidation.toolRunnerProgram);
        SAFE_POINT_EXT(stool, setError(tr("Scripting tool '%1' isn't found in the registry")), );

        if(stool->getPath().isEmpty()) {
            stateInfo.setError(QString("The tool %1 that runs %2 is not installed. "
                                       "Please set the path to the executable file of the"
                                       " tool in the External Tools settings. Some of the tools "
                                       "may be located in UGENE/Tools directory")
                               .arg(originalValidation.toolRunnerProgram)
                               .arg(toolName));
        } else {
            originalValidation.arguments.prepend(originalValidation.executableFile);
            originalValidation.executableFile = stool->getPath();
        }
    }

    validations.append(originalValidation);
    coreLog.trace("Creating validation task for: " + toolName);
    checkVersionRegExp = tool->getVersionRegExp();
    version = "unknown";

    algoLog.trace("Program executable: " + toolPath);
    SAFE_POINT(!validations.isEmpty(), "Tools' validations list is empty", );
    algoLog.trace("Program arguments: " + validations.last().arguments.join(" "));

    CHECK(!hasError(), );

    foreach (const ExternalToolValidation& validation, validations) {
        if (externalToolProcess != NULL) {
            delete externalToolProcess;
            externalToolProcess = NULL;
        }

        checkArchitecture(validation.executableFile);
        CHECK_OP(stateInfo, );
        
        externalToolProcess = new QProcess();
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        externalToolProcess->setProcessEnvironment(env);
        connect(externalToolProcess, SIGNAL(finished(int)), SLOT(sl_processFinish()));

        externalToolProcess->start(validation.executableFile, validation.arguments);
        bool started = externalToolProcess->waitForStarted(3000);

        if (!started) {
            errorMsg = validation.possibleErrorsDescr.value(ExternalToolValidation::DEFAULT_DESCR_KEY, "");
            if (!errorMsg.isEmpty()) {
                stateInfo.setError(errorMsg);
            } else {
                stateInfo.setError(tr("Tool does not start.<br>"
                                      "It is possible that the specified executable file "
                                      "<i>%1</i> for %2 tool is invalid. You can change "
                                      "the path to the executable file in the external "
                                      "tool settings in the global preferences.")
                                   .arg(toolPath)
                                   .arg(toolName));
            }
            isValid = false;
            return;
        }


        if (isAlreadyFinished) {
            algoLog.trace("Great! That was the reason of external tools validation hanging");
        }

        while (!externalToolProcess->waitForFinished(1000) && !isAlreadyFinished) {
            if (isCanceled()) {
                cancelProcess();
            }
        }

        if (!parseLog(validation)) {
            return;
        }
        
        if (!isValid) {
            return;
        }
    }
}

Task::ReportResult ExternalToolJustValidateTask::report() {
    qint64 endTime = TimeCounter::getCounter();
    if (U2_UNLIKELY(qgetenv("UGENE_GUI_TEST").toInt() == 1)) {
        coreLog.trace(QString("ExternalToolJustValidateTask: '%1', %2 micro seconds").arg(toolName).arg(endTime - startTime));
    }

    if (!isValid && !stateInfo.hasError() && !toolPath.isEmpty()) {
        if (errorMsg.isEmpty()) {
            stateInfo.setError(tr("Can not find expected message.<br>"
                                  "It is possible that the specified "
                                  "executable file <i>%1</i> for %2 tool "
                                  "is invalid. You can change the path to "
                                  "the executable file in the external tool "
                                  "settings in the global preferences.")
                               .arg(toolPath)
                               .arg(toolName));
        } else {
            stateInfo.setError(errorMsg);
        }
    }

    return ReportResult_Finished;
}

void ExternalToolJustValidateTask::cancelProcess() {
    externalToolProcess->kill();
}

void ExternalToolJustValidateTask::sl_processFinish() {
    isAlreadyFinished = true;
}

bool ExternalToolJustValidateTask::parseLog(const ExternalToolValidation& validation) {
    errorMsg = validation.possibleErrorsDescr.value(ExternalToolValidation::DEFAULT_DESCR_KEY, "");

    QString errLog = QString(externalToolProcess->readAllStandardError());
    if (!errLog.isEmpty()) {
        if (errLog.contains(validation.expectedMsg)) {
            isValid = true;
            checkVersion(errLog);
        } else {
            isValid = false;
            foreach (const QString& errStr, validation.possibleErrorsDescr.keys()) {
                if (errLog.contains(errStr)) {
                    errorMsg = validation.possibleErrorsDescr[errStr];
                    return false;
                }
            }
        }
    }

    QString log = QString(externalToolProcess->readAllStandardOutput());
    if (!log.isEmpty()) {
        if (log.contains(validation.expectedMsg)) {
            isValid = true;
            checkVersion(log);
        } else {
            isValid = false;
            foreach (const QString& errStr, validation.possibleErrorsDescr.keys()) {
                if (log.contains(errStr)) {
                    errorMsg = validation.possibleErrorsDescr[errStr];
                    return false;
                }
            }
        }
    }

    return true;
}

void ExternalToolJustValidateTask::checkVersion(const QString &partOfLog) {
    QStringList lastPartOfLog=partOfLog.split(QRegExp("(\n|\r)"));

    foreach (QString buf, lastPartOfLog) {
        if (buf.contains(checkVersionRegExp)) {
            checkVersionRegExp.indexIn(buf);
            version = checkVersionRegExp.cap(1);
            return;
        }
    }
}

void ExternalToolJustValidateTask::checkArchitecture(const QString &toolPath) {
    Q_UNUSED(toolPath);
#ifdef Q_OS_MAC
    QProcess archProcess;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    archProcess.setProcessEnvironment(env);
    archProcess.start("file", QStringList() << toolPath);

    bool started = archProcess.waitForStarted(3000);
    CHECK(started, );

    int timeout = 10000;
    int period = 1000;
    int timer = 0;
    while (!archProcess.waitForFinished(period)) {
        timer += period;
        CHECK(timer <= timeout, );

        if (isCanceled()) {
            cancelProcess();
        }
    }

    const QString output = archProcess.readAllStandardOutput();
    archProcess.close();

    bool is_ppc = output.contains("Mach-O executable ppc");
    bool is_i386 = output.contains("Mach-O executable i386");
    bool is_x86_64 = output.contains("Mach-O 64-bit executable x86_64");
    if (Q_UNLIKELY(AppResourcePool::isSystem64bit() && is_ppc && !is_i386 && !is_x86_64)) {
        setError("This external tool has unsupported architecture");
    }
#endif
}

ExternalToolSearchAndValidateTask::ExternalToolSearchAndValidateTask(const QString& _toolName) :
    ExternalToolValidateTask(_toolName, TaskFlags(TaskFlag_CancelOnSubtaskCancel | TaskFlag_NoRun)),
    toolIsFound(false),
    searchTask(NULL),
    validateTask(NULL),
    startTime(0)
{
}

void ExternalToolSearchAndValidateTask::prepare() {
    startTime = TimeCounter::getCounter();
    searchTask = new ExternalToolSearchTask(toolName);
    addSubTask(searchTask);
}

QList<Task*> ExternalToolSearchAndValidateTask::onSubTaskFinished(Task *subTask) {
    QList<Task*> subTasks;
    CHECK(!subTask->isCanceled(), subTasks);

    if (searchTask == subTask) {
        CHECK(!searchTask->hasError(), subTasks);
        toolPaths = searchTask->getPaths();
        if (toolPaths.isEmpty()) {
            isValid = false;
            toolPath = "";
            toolIsFound = false;
        } else {
            toolIsFound = true;
            validateTask = new ExternalToolJustValidateTask(toolName, toolPaths.first());
            subTasks << validateTask;
        }
    }

    if (validateTask == subTask) {
        if (validateTask->isValidTool()) {
            isValid = validateTask->isValidTool();
            toolPath = validateTask->getToolPath();
            version = validateTask->getToolVersion();
        } else {
            errorMsg = validateTask->getError();
            toolPath = validateTask->getToolPath();
            SAFE_POINT(!toolPaths.isEmpty(), "Tool path's list is unexpectedly empty", subTasks);
            toolPaths.removeFirst();

            if (!toolPaths.isEmpty()) {
                validateTask = new ExternalToolJustValidateTask(toolName, toolPaths.first());
                subTasks << validateTask;
            }
        }
    }

    return subTasks;
}

Task::ReportResult ExternalToolSearchAndValidateTask::report() {
    qint64 endTime = TimeCounter::getCounter();
    if (U2_UNLIKELY(qgetenv("UGENE_GUI_TEST").toInt() == 1)) {
        taskLog.trace(QString("ExternalToolSearchAndValidateTask: tool '%1', %2 micro seconds").arg(toolName).arg(endTime - startTime));
    }

    ExternalToolRegistry* etRegistry = AppContext::getExternalToolRegistry();
    SAFE_POINT(etRegistry, "An external tool registry is NULL", ReportResult_Finished);
    ExternalTool* tool = etRegistry->getByName(toolName);
    SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(toolName), ReportResult_Finished);

    if (!isValid && toolIsFound && !toolPath.isEmpty()) {
        if (errorMsg.isEmpty()) {
            setError(tr("Can not find expected message."
                        "<br>It is possible that the specified executable file "
                        "<i>%1</i> for %2 tool is invalid. "
                        "You can change the path to the executable file "
                        "in the external tool settings in the global "
                        "preferences.").arg(toolPath).arg(toolName));
        } else {
            setError(errorMsg);
        }
    }

    return ReportResult_Finished;
}

ExternalToolsValidateTask::ExternalToolsValidateTask(const QList<Task*> &_tasks) :
    SequentialMultiTask(tr("Checking external tools"), _tasks, TaskFlags(TaskFlag_NoRun | TaskFlag_CancelOnSubtaskCancel)) {
}

QList<Task*> ExternalToolsValidateTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> subTasks;
    ExternalToolValidateTask* validateTask = qobject_cast<ExternalToolValidateTask*>(subTask);
    bool muted = false;

    if (validateTask) {
        ExternalToolRegistry* etRegistry = AppContext::getExternalToolRegistry();
        SAFE_POINT(etRegistry, "An external tool registry is NULL", subTasks);
        ExternalTool* tool = etRegistry->getByName(validateTask->getToolName());
        SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(validateTask->getToolName()), subTasks);
        muted = tool->isMuted();
    }

    if (subTask->hasError()) {
        if (muted) {
            taskLog.details(subTask->getTaskName() + tr(" failed: ") + subTask->getError());
        } else {
            taskLog.error(subTask->getTaskName() + tr(" failed: ") + subTask->getError());
        }
    }

    subTasks = SequentialMultiTask::onSubTaskFinished(subTask);
    return subTasks;
}

ExternalToolsInstallTask::ExternalToolsInstallTask(const QList<Task*> &_tasks) :
    SequentialMultiTask(tr("Installing external tools"), _tasks, TaskFlags(TaskFlag_NoRun | TaskFlag_CancelOnSubtaskCancel)) {
}

QList<Task*> ExternalToolsInstallTask::onSubTaskFinished(Task* subTask) {
    if (subTask->hasError()) {
        taskLog.error(subTask->getTaskName() + tr(" failed: ") + subTask->getError());
    }
    return SequentialMultiTask::onSubTaskFinished(subTask);
}

}//namespace
