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

#include "ServiceRegistryImpl.h"

#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Core/PluginModel.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Core/CMDLineCoreOptions.h>

#include <QtCore/QTimerEvent>

namespace U2 {

/* TRANSLATOR U2::ServiceRegistryImpl */

ServiceRegistryImpl::~ServiceRegistryImpl() {
    foreach(Service* s, services) {
        assert(s->isDisabled());
        delete s;
    }
    assert(activeServiceTasks.isEmpty());
}

QList<Service*> ServiceRegistryImpl::findServices(ServiceType t ) const {
    QList<Service*> res;
    foreach(Service* s, services) {
        if (s->getType() == t) {
            res.append(s);
        }
    }
    return res;
}


/// Returns 'true' if a service with the specified ServiceType is registered and enabled
Task* ServiceRegistryImpl::registerServiceTask(Service* s) {
    return new RegisterServiceTask(this, s);
}

Task* ServiceRegistryImpl::unregisterServiceTask(Service* s) {
    return new UnregisterServiceTask(this, s);
}



Task* ServiceRegistryImpl::enableServiceTask(Service* s) {
    return new EnableServiceTask(this, s);
}

Task* ServiceRegistryImpl::disableServiceTask(Service* s) {
    return new DisableServiceTask(this, s, true);
}


void ServiceRegistryImpl::setServiceState(Service* s, ServiceState state) {
    if (s->getState() == state) {
        return;
    }
    _setServiceState(s, state);

    initiateServicesCheckTask();
}

void ServiceRegistryImpl::initiateServicesCheckTask() {
    if (!timerIsActive) {
        timerIsActive = true;
        startTimer(100);
    }
}

void ServiceRegistryImpl::timerEvent(QTimerEvent *event) {
    if (!activeServiceTasks.empty()) {
        return; //wait until no active service tasks left
    }
    killTimer(event->timerId());
    timerIsActive = false;
    Service* s = findServiceReadyToEnable();
    if (s!=NULL) {
        AppContext::getTaskScheduler()->registerTopLevelTask(new EnableServiceTask(this, s));
    }
}

Service* ServiceRegistryImpl::findServiceReadyToEnable() const {
    //TODO: recheck circular tasks too

    // look for new + parent_disabled services and check if a service can be run
    foreach(Service* s, services) {
        if (s->getState() == ServiceState_Disabled_New || s->getState() == ServiceState_Disabled_ParentDisabled) {
            bool allParentsEnabled = true;
            QList<ServiceType> parentTypes = s->getParentServiceTypes();
            foreach(ServiceType t, parentTypes) {
                QList<Service*> parentServices = findServices(t);
                bool parentIsEnabled = false;
                foreach(Service* ps, parentServices) {
                    if (ps->isEnabled()) {
                        parentIsEnabled = true;
                        break;
                    }
                }
                if (!parentIsEnabled) {
                    allParentsEnabled = false;
                    break;
                }
            }
            if (allParentsEnabled) {
                return s;
            }
        }
    }
    return NULL;
}

//////////////////////////////////////////////////////////////////////////
// Tasks

/// RegisterServiceTask

RegisterServiceTask::RegisterServiceTask(ServiceRegistryImpl* _sr, Service* _s) 
: Task(tr("Register '%1' service").arg(_s->getName()), TaskFlag_NoRun), sr(_sr), s(_s)
{
    assert(s->getState() == ServiceState_Disabled_New);
    assert(!sr->services.contains(s));
}

void RegisterServiceTask::prepare() {
    if (sr->services.contains(s)) {
        stateInfo.setError(  tr("Service is already registered '%1'").arg(s->getName()) );
        return;
    }
    if (s->getState() != ServiceState_Disabled_New) {
        stateInfo.setError(  tr("Illegal service state: %1").arg(s->getName()) );
        return;
    }

    foreach(Service* rs, sr->services) {
        if (rs->getType() == s->getType() && (s->getFlags().testFlag(ServiceFlag_Singleton) || s->getFlags().testFlag(ServiceFlag_Singleton))) {
            stateInfo.setError(tr("Only one service of specified type is allowed: %1").arg(s->getType().id));
            return;
        }
    }

    sr->services.append(s);
    emit sr->si_serviceRegistered(s);
    addSubTask(new EnableServiceTask(sr, s));
}

///EnableServiceTask

EnableServiceTask::EnableServiceTask(ServiceRegistryImpl* _sr, Service* _s) 
: Task(tr("Enable '%1' service").arg(_s->getName()), TaskFlag_NoRun), sr(_sr), s(_s)
{
    assert(sr && s && s->isDisabled());
}

static bool findCircular(ServiceRegistryImpl* sr, Service* s, int currentDepth =0);
static bool checkAllParentsEnabled(ServiceRegistryImpl* sr, Service* s);

void EnableServiceTask::prepare() {
//TODO: improve messaging. The service name is already mentioned in task name!

    sr->activeServiceTasks.push_back(this);
    if (s->isEnabled()) {
        stateInfo.setError(tr("Service is already enabled: %1").arg(s->getName()));
        return;
    }
    bool circular = findCircular(sr, s);
    if (circular) {
        sr->setServiceState(s, ServiceState_Disabled_CircularDependency);
        stateInfo.setError(tr("Circular service dependency: %1").arg(s->getName()));
        return;
    }
    bool noparent = !checkAllParentsEnabled(sr, s);
    if (noparent) {
        sr->setServiceState(s, ServiceState_Disabled_ParentDisabled);
        stateInfo.setError(tr("Required service is not enabled: %1").arg(s->getName()));
        return;
    }

    Task* t = sr->createServiceEnablingTask(s);
    if (t != NULL) {
        addSubTask(t);
    }

    sr->initiateServicesCheckTask();
}

Task::ReportResult EnableServiceTask::report() {
    sr->activeServiceTasks.removeAll(this);
    if (stateInfo.hasError() || s->isEnabled()) {
        return ReportResult_Finished;
    }
    bool success = !propagateSubtaskError();
    sr->setServiceState(s, success ? ServiceState_Enabled : ServiceState_Disabled_FailedToStart);
    return ReportResult_Finished;
}


static bool findCircular(ServiceRegistryImpl* sr, Service* s, int currentDepth) {
    currentDepth++;
    if (currentDepth > sr->getServices().size()) {
        return true;
    }
    foreach(ServiceType st, s->getParentServiceTypes()) {
        QList<Service*> parents = sr->findServices(st);
        foreach(Service* p, parents) {
            bool circular = findCircular(sr, p, currentDepth);
            if (circular) {
                return true;
            }
        }
    }
    return false;
}

static bool checkAllParentsEnabled(ServiceRegistryImpl* sr, Service* s) {
    foreach(ServiceType st, s->getParentServiceTypes()) {
        QList<Service*> parents = sr->findServices(st);
        if (parents.isEmpty()) {
            return false;
        }
        foreach(Service* p, parents) {
            if (p->isDisabled()) {
                return false;
            }
        }
    }
    return true;
}

/// UnregisterServiceTask
UnregisterServiceTask::UnregisterServiceTask(ServiceRegistryImpl* _sr, Service* _s) 
: Task(tr("Unregister '%1' service").arg(_s->getName()), TaskFlag_NoRun), sr(_sr), s(_s)
{
    assert(sr->services.contains(s));
}

void UnregisterServiceTask::prepare() {
    if (!sr->services.contains(s)) {
        stateInfo.setError(  tr("Service is not registered: %1").arg(s->getName()) );
        return;
    }
    if (s->isEnabled()) {
        addSubTask(new DisableServiceTask(sr, s, false));
    }
}

Task::ReportResult UnregisterServiceTask::report() {
    if (stateInfo.hasError()) {
        return ReportResult_Finished;
    }
    if (s->isDisabled()) {
        assert(sr->services.count(s) == 1);
        sr->services.removeAll(s);
        emit sr->si_serviceUnregistered(s);
        delete s;//TODO: redesign real-time service registration/unregistration-> synchronize with plugin list
    }
    return ReportResult_Finished;
}


/// DisableServiceTask
DisableServiceTask::DisableServiceTask(ServiceRegistryImpl* _sr, Service* _s, bool _manual) 
: Task(tr("Disable '%1' service").arg(_s->getName()), TaskFlags_NR_FOSCOE), sr(_sr), s(_s), manual(_manual)
{
    assert(sr->services.contains(s) && s->isEnabled());
}

static QList<Service*> getDirectChilds(ServiceRegistryImpl* sr, ServiceType st);

bool DisableServiceTask::isGUITesting() const {
    CMDLineRegistry *cmdLine = AppContext::getCMDLineRegistry();
    return cmdLine && cmdLine->hasParameter(CMDLineCoreOptions::LAUNCH_GUI_TEST);
}

void DisableServiceTask::prepare() {
    sr->activeServiceTasks.push_back(this);
    const QList<Task*>& activeTopTasks = AppContext::getTaskScheduler()->getTopLevelTasks();

    int nTopLevelTasks = activeTopTasks.count();
    if (nTopLevelTasks > 1) { // [parent] of DisableServiceTask
        foreach(Task* t, activeTopTasks) {
            coreLog.details(tr("Active top-level task name: %1").arg(t->getTaskName()));
        }
        stateInfo.setError(  tr("Active task was found") );
        return;
    }
    if (!sr->services.contains(s)) {
        stateInfo.setError(  tr("Service is not registered: %1").arg(s->getName()) );
        return;
    }
    if (!s->isEnabled()) {
        stateInfo.setError(  tr("Service is not enabled: %1").arg(s->getName()) );
        return;
    }
    bool stopChilds = sr->findServices(s->getType()).size() == 1;
    if (stopChilds) {
        QList<Service*> childsToDisable = getDirectChilds(sr, s->getType());
        foreach(Service* c, childsToDisable) {
            if (c->isEnabled()) {
                addSubTask(new DisableServiceTask(sr, c, false));
            }
        }
    }
    Task* disablingTask = sr->createServiceDisablingTask(s);
    if (disablingTask!=NULL) {
        addSubTask(disablingTask);
    }
}

Task::ReportResult DisableServiceTask::report() {
    sr->activeServiceTasks.removeAll(this);
    if (stateInfo.hasError() || s->isDisabled()) {
        return ReportResult_Finished;
    }
    if (propagateSubtaskError()) {
        return ReportResult_Finished;
    }
    ServiceState newState = manual ? ServiceState_Disabled_Manually : ServiceState_Disabled_ParentDisabled;
    sr->setServiceState(s, newState);
    return ReportResult_Finished;
}


static QList<Service*> getDirectChilds(ServiceRegistryImpl* sr, ServiceType st) {
    QList<Service*> res;
    foreach(Service* s, sr->getServices()) {
        if (s->getParentServiceTypes().contains(st)) {
            res.append(s);
        }
    }
    return res;
}


}//namespace
