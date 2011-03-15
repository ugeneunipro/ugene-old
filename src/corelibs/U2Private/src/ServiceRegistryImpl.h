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

#ifndef _U2_SERVICE_REGISTRY_IMPL_
#define _U2_SERVICE_REGISTRY_IMPL_

#include "private.h"
#include <U2Core/ServiceModel.h>
#include <U2Core/Task.h>

#include <QtCore/QMap>

namespace U2 {

class RegisterServiceTask;
class EnableServiceTask;
class UnregisterServiceTask;
class DisableServiceTask;


class U2PRIVATE_EXPORT ServiceRegistryImpl : public ServiceRegistry {
    Q_OBJECT

    friend class RegisterServiceTask;
    friend class EnableServiceTask;
    friend class UnregisterServiceTask;
    friend class DisableServiceTask;

public:
    ServiceRegistryImpl() {timerIsActive = false;}
    ~ServiceRegistryImpl();

    /// Returns list of all registered services
    virtual const QList<Service*>& getServices() const {return services;}

    /// Finds service with the specified ServiceType
    virtual QList<Service*> findServices(ServiceType t) const;

    virtual Task* registerServiceTask(Service* s);

    virtual Task* unregisterServiceTask(Service* s);

    virtual Task* enableServiceTask(Service* s);

    virtual Task* disableServiceTask(Service* s);

    void unregisterPluginServices(Plugin* p);

private:
    void initiateServicesCheckTask();

    Service* findServiceReadyToEnable() const;

    void setServiceState(Service* s, ServiceState state);
    Task* createServiceEnablingTask(Service* s) {return _createServiceEnablingTask(s);}
    Task* createServiceDisablingTask(Service* s) {return _createServiceDisablingTask(s);}

protected:
    void timerEvent(QTimerEvent *event);

private:
    QList<Service*> services;
    bool            timerIsActive;
    QList<Task*> activeServiceTasks;
};

class RegisterServiceTask : public Task {
    Q_OBJECT
public:
    RegisterServiceTask(ServiceRegistryImpl* sr, Service* s);

    virtual void prepare();

private:
    ServiceRegistryImpl* sr;
    Service* s;
};

class EnableServiceTask : public Task {
    Q_OBJECT
public:
    EnableServiceTask(ServiceRegistryImpl* sr, Service* s);

    virtual void prepare();

    virtual ReportResult report();

private:
    ServiceRegistryImpl* sr;
    Service* s;
};

class UnregisterServiceTask : public Task {
    Q_OBJECT
public:
    UnregisterServiceTask(ServiceRegistryImpl* sr, Service* s);

    virtual void prepare();

    virtual ReportResult report();

private:
    ServiceRegistryImpl* sr;
    Service* s;
};


class DisableServiceTask : public Task {
    Q_OBJECT
public:
    DisableServiceTask(ServiceRegistryImpl* sr, Service* s, bool manual);

    virtual void prepare();

    virtual ReportResult report();

private:
    ServiceRegistryImpl* sr;
    Service* s;
    bool manual;
};


}//namespace


#endif


