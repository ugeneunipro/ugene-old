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
    SAFE_POINT(NULL != etRegistry, "External tool registry is NULL", );

    dependencies.clear();
    validateList.clear();
    toolStates.clear();

    // Read settings
    ExternalToolSupportSettings::getExternalTools();

    QList<ExternalTool*> toolsList = etRegistry->getAllEntries();
    QStrStrMap toolPaths;
    foreach (ExternalTool* tool, toolsList) {
        QString toolPath = addTool(tool);
        toolPaths.insert(tool->getName(), toolPath);
    }

    validateTools(toolPaths);
}

void ExternalToolManagerImpl::stop() {
    CHECK(NULL != etRegistry, );
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
    SAFE_POINT(NULL != etRegistry, "External tool registry is NULL", );

    QList<Task*> taskList;

    foreach (const QString& toolName, toolNames) {
        QString toolPath = toolPaths.value(toolName);
        if (dependenciesIsOk(toolName) && !toolPath.isEmpty()) {
            ExternalToolSearchAndValidateTask* task = new ExternalToolSearchAndValidateTask(toolName, toolPath);
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
        AppContext::getTaskScheduler()->registerTopLevelTask(validationTask);
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
    SAFE_POINT(NULL != etRegistry, "External tool registry is NULL", );

    foreach (const QString& toolName, toolNames) {
        if (dependenciesIsOk(toolName)) {
            validateList << toolName;
        } else {
            toolStates.insert(toolName, NotValidByDependency);
            if (toolPaths.contains(toolName)) {
                setToolPath(toolName, toolPaths.value(toolName));
            }
        }
    }

    if (NULL != listener && validateList.isEmpty()) {
        listener->validationFinished();
    }

    validateTools(toolPaths, listener);
}

bool ExternalToolManagerImpl::isValid(const QString& toolName) const {
    SAFE_POINT(NULL != etRegistry, "External tool registry is NULL", false);

    return (Valid == toolStates.value(toolName, NotDefined));
}

ExternalToolManager::ExternalToolState ExternalToolManagerImpl::getToolState(const QString& toolName) const {
    return toolStates.value(toolName, NotDefined);
}

QString ExternalToolManagerImpl::addTool(ExternalTool* tool) {
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
        foreach (QString dependency, toolDependencies) {
            dependencies.insertMulti(dependency, tool->getName());
        }

        if (dependenciesIsOk(tool->getName()) && !tool->isValid()) {
            if (tool->isModule()) {
                QString masterName = tool->getDependencies().first();
                ExternalTool* masterTool = etRegistry->getByName(masterName);
                SAFE_POINT(NULL != masterTool, QString("Couldn't find the external tool \'%1\' in the registry").arg(masterName), QString());

                toolPath = masterTool->getPath();
            }
            validateList << tool->getName();
        }
    } else {
        if (!tool->isValid()) {
            validateList << tool->getName();
        }
    }

    return toolPath;
}

void ExternalToolManagerImpl::sl_checkTaskStateChanged() {
    ExternalToolsValidateTask* masterTask = qobject_cast<ExternalToolsValidateTask*>(sender());
    SAFE_POINT(NULL != masterTask, "Unexpected task", );

    if (masterTask->isFinished()) {
        ExternalToolValidationListener* listener = listeners.value(masterTask, NULL);
        CHECK(NULL != listener, );
        listeners.remove(masterTask);

        QList<Task*> subTasks = masterTask->getSubtasks();
        foreach (Task* subTask, subTasks) {
            ExternalToolSearchAndValidateTask* task = qobject_cast<ExternalToolSearchAndValidateTask*>(subTask);
            SAFE_POINT(NULL != task, "Unexpected task", );

            listener->setToolState(task->getToolName(), task->isValidTool());
        }
        listener->validationFinished();
    }
}

void ExternalToolManagerImpl::sl_validationTaskStateChanged() {
    ExternalToolSearchAndValidateTask* task = qobject_cast<ExternalToolSearchAndValidateTask*>(sender());
    SAFE_POINT(NULL != task, "Unexpected task", );

    if (task->isFinished()) {
        if (task->isValidTool()) {
            toolStates.insert(task->getToolName(), Valid);
        } else {
            toolStates.insert(task->getToolName(), NotValid);
        }

        ExternalTool* tool = etRegistry->getByName(task->getToolName());
        SAFE_POINT(NULL != tool, QString("Couldn't find the external tool \'%1\' in the registry").arg(task->getToolName()), );
        if (tool->isModule()) {
            QString masterName = tool->getDependencies().first();
            ExternalTool* masterTool = etRegistry->getByName(masterName);
            SAFE_POINT(NULL != tool, QString("Couldn't find the external tool \'%1\' in the registry").arg(masterName), );
            SAFE_POINT(masterTool->getPath() == task->getToolPath(), "Module tool should have the same path as it's master tool", );
        }

        tool->setVersion(task->getToolVersion());
        tool->setPath(task->getToolPath());
        tool->setValid(task->isValidTool());
    }
}

void ExternalToolManagerImpl::sl_toolValidationStatusChanged(bool isValid) {
    ExternalTool* tool = qobject_cast<ExternalTool*>(sender());
    SAFE_POINT(NULL != tool, "Unexpected external tool", );

    if (isValid) {
        toolStates.insert(tool->getName(), Valid);
    } else {
        toolStates.insert(tool->getName(), NotValid);
    }

    QStrStrMap toolPaths;
    foreach (QString vassalName, dependencies.values(tool->getName())) {
        ExternalTool* vassalTool = etRegistry->getByName(vassalName);
        SAFE_POINT(NULL != vassalTool, QString("Couldn't find the external tool \'%1\' in the registry").arg(vassalName), );

        if (vassalTool->isModule()) {
            toolPaths.insert(vassalName, tool->getPath());
            setToolPath(vassalName, tool->getPath());
        }

        if (true == isValid &&
                dependenciesIsOk(vassalName) &&
                ValidationIsInProcess != toolStates.value(vassalName, NotDefined)) {
            validateList << vassalName;
        } else {
            toolStates.insert(vassalName, NotValidByDependency);
            vassalTool->setValid(false);
        }
    }

    validateTools(toolPaths);
}

bool ExternalToolManagerImpl::dependenciesIsOk(const QString& toolName) {
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

        QString toolPath = toolPaths.value(toolName, QString::null);

        ExternalToolSearchAndValidateTask* task = new ExternalToolSearchAndValidateTask(toolName, toolPath);
        connect(task,
                SIGNAL(si_stateChanged()),
                SLOT(sl_validationTaskStateChanged()));
        taskList << task;
    }

    if (!taskList.isEmpty()) {
        ExternalToolsValidateTask* validationTask = new ExternalToolsValidateTask(taskList);
        validationTask->setMaxParallelSubtasks(MAX_PARALLEL_SUBTASKS);
        if (NULL != listener) {
            connect(validationTask, SIGNAL(si_stateChanged()), SLOT(sl_checkTaskStateChanged()));
            listeners.insert(validationTask, listener);
        }
        AppContext::getTaskScheduler()->registerTopLevelTask(validationTask);
    }
}

void ExternalToolManagerImpl::setToolPath(const QString& toolName, const QString& toolPath) {
    AppContext::getExternalToolRegistry()->getByName(toolName)->setPath(toolPath);
}

}   // namespace
