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

#include <QtCore/QEventLoop>
#include <QtCore/QSet>

#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/MultiTask.h>

#include "utils/ExternalToolSearchTask.h"
#include "utils/ExternalToolValidateTask.h"
#include "ExternalToolSupportSettings.h"
#include "ExternalToolManager.h"

namespace U2 {

ExternalToolManagerImpl::ExternalToolManagerImpl() {
    etRegistry = AppContext::getExternalToolRegistry();
}

ExternalToolManagerImpl::~ExternalToolManagerImpl() {
}

void ExternalToolManagerImpl::start() {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );

    dependencies.clear();
    validateList.clear();
    searchList.clear();
    toolStates.clear();

    // Read settings
    ExternalToolSupportSettings::getExternalTools();

    QList<ExternalTool*> toolsList = etRegistry->getAllEntries();
    QStrStrMap toolPaths;
    foreach (ExternalTool* tool, toolsList) {
        SAFE_POINT(tool, "Tool is NULL", );
        QString toolPath = addTool(tool);
        if (!toolPath.isEmpty()) {
            toolPaths.insert(tool->getName(), toolPath);
        }
    }

    validateTools(toolPaths);
}

void ExternalToolManagerImpl::stop() {
    CHECK(etRegistry, );
    foreach (ExternalTool* tool, etRegistry->getAllEntries()) {
        disconnect(tool, NULL, this, NULL);
    }
}

void ExternalToolManagerImpl::check(const QString& toolName, const QString& toolPath, ExternalToolValidationListener* listener) {
    QStrStrMap toolPaths;
    toolPaths.insert(toolName, toolPath);
    check(QStringList() << toolName, toolPaths, listener);
}

void ExternalToolManagerImpl::check(const QStringList& toolNames, const QStrStrMap& toolPaths, ExternalToolValidationListener* listener) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    SAFE_POINT(listener, "Listener is NULL", );

    QList<Task*> taskList;

    foreach (const QString& toolName, toolNames) {
        QString toolPath = toolPaths.value(toolName);
        if (dependenciesAreOk(toolName) && !toolPath.isEmpty()) {
            ExternalToolValidateTask* task = new ExternalToolJustValidateTask(toolName, toolPath);
            taskList << task;
        } else {
            listener->setToolState(toolName, false);
        }
    }

    if (taskList.isEmpty()) {
        listener->validationFinished();
    } else {
        ExternalToolsValidateTask* validationTask = new ExternalToolsValidateTask(taskList);
        connect(validationTask, SIGNAL(si_stateChanged()), SLOT(sl_checkTaskStateChanged()));
        listeners.insert(validationTask, listener);
        validationTask->setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS);
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        SAFE_POINT(scheduler, "Task scheduler is NULL", );
        scheduler->registerTopLevelTask(validationTask);
    }
}

void ExternalToolManagerImpl::validate(const QString& toolName, ExternalToolValidationListener* listener) {
    validate(QStringList() << toolName, listener);
}

void ExternalToolManagerImpl::validate(const QString& toolName, const QString& path, ExternalToolValidationListener* listener) {
    QStrStrMap toolPaths;
    toolPaths.insert(toolName, path);
    validate(QStringList() << toolName, toolPaths, listener);
}

void ExternalToolManagerImpl::validate(const QStringList& toolNames, ExternalToolValidationListener* listener) {
    validate(toolNames, QStrStrMap(), listener);
}

void ExternalToolManagerImpl::validate(const QStringList& toolNames, const QStrStrMap& toolPaths, ExternalToolValidationListener* listener) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );

    foreach (const QString& toolName, toolNames) {
        if (dependenciesAreOk(toolName)) {
            validateList << toolName;
        } else {
            toolStates.insert(toolName, NotValidByDependency);
            if (toolPaths.contains(toolName)) {
                setToolPath(toolName, toolPaths.value(toolName));
            }
        }
    }

    if (listener && validateList.isEmpty()) {
        listener->validationFinished();
    }

    validateTools(toolPaths, listener);
}

bool ExternalToolManagerImpl::isValid(const QString& toolName) const {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", false);

    return (Valid == toolStates.value(toolName, NotDefined));
}

ExternalToolManager::ExternalToolState ExternalToolManagerImpl::getToolState(const QString& toolName) const {
    return toolStates.value(toolName, NotDefined);
}

QString ExternalToolManagerImpl::addTool(ExternalTool* tool) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", "");
    QString toolPath;

    if (tool->isValid()) {
        toolStates.insert(tool->getName(), Valid);
    } else {
        toolStates.insert(tool->getName(), NotDefined);
    }

    connect(tool,
            SIGNAL(si_toolValidationStatusChanged(bool)),
            SLOT(sl_toolValidationStatusChanged(bool)));

    QStringList toolDependencies = tool->getDependencies();
    if (!toolDependencies.isEmpty()) {
        foreach (const QString& dependency, toolDependencies) {
            dependencies.insertMulti(dependency, tool->getName());
        }

        if (dependenciesAreOk(tool->getName()) && !tool->isValid()) {
            if (tool->isModule()) {
                QString masterName = tool->getDependencies().first();
                ExternalTool* masterTool = etRegistry->getByName(masterName);
                SAFE_POINT(masterTool, QString("An external tool '%1' isn't found in the registry").arg(masterName), "");

                toolPath = masterTool->getPath();
            }
            validateList << tool->getName();
        }
    } else {
        if (!tool->isValid()) {
            validateList << tool->getName();
        }
    }

    if (!validateList.contains(tool->getName()) && !tool->isModule()) {
        searchList << tool->getName();
    }

    return toolPath;
}

void ExternalToolManagerImpl::sl_checkTaskStateChanged() {
    ExternalToolsValidateTask* masterTask = qobject_cast<ExternalToolsValidateTask*>(sender());
    SAFE_POINT(masterTask, "Unexpected task", );

    if (masterTask->isFinished()) {
        ExternalToolValidationListener* listener = listeners.value(masterTask, NULL);
        if (listener) {
            listeners.remove(masterTask);

            QList<Task*> subTasks = masterTask->getSubtasks();
            foreach (Task* subTask, subTasks) {
                ExternalToolValidateTask* task = qobject_cast<ExternalToolValidateTask*>(subTask);
                SAFE_POINT(task, "Unexpected task", );

                listener->setToolState(task->getToolName(), task->isValidTool());
            }
            listener->validationFinished();
        }
    }
}

void ExternalToolManagerImpl::sl_validationTaskStateChanged() {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    ExternalToolValidateTask* task = qobject_cast<ExternalToolValidateTask*>(sender());
    SAFE_POINT(task, "Unexpected task", );

    if (task->isFinished()) {
        if (task->isValidTool()) {
            toolStates.insert(task->getToolName(), Valid);
        } else {
            toolStates.insert(task->getToolName(), NotValid);
        }

        ExternalTool* tool = etRegistry->getByName(task->getToolName());
        SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(task->getToolName()), );
        if (tool->isModule()) {
            QStringList toolDependencies = tool->getDependencies();
            SAFE_POINT(!toolDependencies.isEmpty(), QString("Tool's dependencies list is unexpectedly empty: "
                                                            "a master tool for the module '%1' is not defined").arg(tool->getName()), );
            QString masterName = toolDependencies.first();
            ExternalTool* masterTool = etRegistry->getByName(masterName);
            SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(masterName), );
            SAFE_POINT(masterTool->getPath() == task->getToolPath(), "Module tool should have the same path as it's master tool", );
        }

        tool->setVersion(task->getToolVersion());
        tool->setPath(task->getToolPath());
        tool->setValid(task->isValidTool());

        searchTools();
    }
}

void ExternalToolManagerImpl::sl_searchTaskStateChanged() {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    ExternalToolSearchTask* task = qobject_cast<ExternalToolSearchTask*>(sender());
    SAFE_POINT(task, "Unexpected task", );

    if (task->isFinished()) {
        QStringList toolPaths = task->getPaths();
        if (!task->getPaths().isEmpty()) {
            setToolPath(task->getToolName(), toolPaths.first());
        }
    }
}

void ExternalToolManagerImpl::sl_toolValidationStatusChanged(bool isValid) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    ExternalTool* tool = qobject_cast<ExternalTool*>(sender());
    SAFE_POINT(tool, "Unexpected message sender", );

    if (isValid) {
        toolStates.insert(tool->getName(), Valid);
    } else {
        toolStates.insert(tool->getName(), NotValid);
    }

    QStrStrMap toolPaths;
    foreach (const QString& vassalName, dependencies.values(tool->getName())) {
        ExternalTool* vassalTool = etRegistry->getByName(vassalName);
        SAFE_POINT(vassalTool, QString("An external tool '%1' isn't found in the registry").arg(vassalName), );

        if (vassalTool->isModule()) {
            toolPaths.insert(vassalName, tool->getPath());
            setToolPath(vassalName, tool->getPath());
        }

        if (isValid &&
                dependenciesAreOk(vassalName) &&
                ValidationIsInProcess != toolStates.value(vassalName, NotDefined)) {
            validateList << vassalName;
            searchList.removeAll(vassalName);
        } else if (ValidationIsInProcess != toolStates.value(vassalName, NotDefined)) {
            vassalTool->setValid(false);
            toolStates.insert(vassalName, NotValidByDependency);
        }
    }

    validateTools(toolPaths);
}

bool ExternalToolManagerImpl::dependenciesAreOk(const QString& toolName) {
    bool result = true;
    foreach (const QString& masterName, dependencies.keys(toolName)) {
        result &= (Valid == toolStates.value(masterName, NotDefined));
    }
    return result;
}

void ExternalToolManagerImpl::validateTools(const QStrStrMap& toolPaths, ExternalToolValidationListener* listener) {
    QList<Task*> taskList;

    foreach (QString toolName, validateList) {
        validateList.removeAll(toolName);
        toolStates.insert(toolName, ValidationIsInProcess);

        QString toolPath;
        bool pathSpecified = toolPaths.contains(toolName);
        if (pathSpecified) {
            toolPath = toolPaths.value(toolName);
            if (toolPath.isEmpty()) {
                toolStates.insert(toolName, NotValid);
                setToolPath(toolName, toolPath);
                if (listener) {
                    listener->setToolState(toolName, false);
                }
                setToolValid(toolName, false);
                continue;
            }
        }

        ExternalToolValidateTask* task;
        if (pathSpecified) {
            task = new ExternalToolJustValidateTask(toolName, toolPath);
        } else {
            task = new ExternalToolSearchAndValidateTask(toolName);
        }
        connect(task,
                SIGNAL(si_stateChanged()),
                SLOT(sl_validationTaskStateChanged()));
        taskList << task;
    }

    if (!taskList.isEmpty()) {
        ExternalToolsValidateTask* validationTask = new ExternalToolsValidateTask(taskList);
        validationTask->setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS);
        if (listener) {
            connect(validationTask, SIGNAL(si_stateChanged()), SLOT(sl_checkTaskStateChanged()));
            listeners.insert(validationTask, listener);
        }
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        SAFE_POINT(scheduler, "Task scheduler is NULL", );
        scheduler->registerTopLevelTask(validationTask);
    } else if (listener) {
        listener->validationFinished();
    }
}

void ExternalToolManagerImpl::searchTools() {
    QList<Task*> taskList;

    foreach (const QString& toolName, searchList) {
        searchList.removeAll(toolName);
        ExternalToolSearchTask* task = new ExternalToolSearchTask(toolName);
        connect(task,
                SIGNAL(si_stateChanged()),
                SLOT(sl_searchTaskStateChanged()));
        taskList << task;
    }

    if (!taskList.isEmpty()) {
        ExternalToolsSearchTask* searchTask = new ExternalToolsSearchTask(taskList);
        TaskScheduler* scheduler = AppContext::getTaskScheduler();
        SAFE_POINT(scheduler, "Task scheduler is NULL", );
        scheduler->registerTopLevelTask(searchTask);
    }
}

void ExternalToolManagerImpl::setToolPath(const QString& toolName, const QString& toolPath) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    ExternalTool* tool = etRegistry->getByName(toolName);
    SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(toolName), );
    tool->setPath(toolPath);
}

void ExternalToolManagerImpl::setToolValid(const QString& toolName, bool isValid) {
    SAFE_POINT(etRegistry, "The external tool registry is NULL", );
    ExternalTool* tool = etRegistry->getByName(toolName);
    SAFE_POINT(tool, QString("An external tool '%1' isn't found in the registry").arg(toolName), );
    tool->setValid(isValid);
}

}   // namespace
