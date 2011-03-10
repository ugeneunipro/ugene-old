#include "ResourceTracker.h"

#include <U2Core/Log.h>
#include "Task.h"

namespace U2 {

ResourceTracker::~ResourceTracker() {
    assert(resMap.isEmpty());
}


void ResourceTracker::registerResourceUser(const QString& resourceName, Task* t) {
    TList list = resMap.value(resourceName);
    assert(!list.contains(t));
    list.append(t);
    resMap[resourceName] = list;
    coreLog.details(tr("resource '%1' is used by '%2'").arg(resourceName).arg(t->getTaskName()));
    emit si_resourceUserRegistered(resourceName, t);
}
    


void ResourceTracker::unregisterResourceUser(const QString& resourceName, Task* t) {
    assert(resMap.contains(resourceName));
    TList list = resMap.value(resourceName);
    assert(list.contains(t));
    list.removeOne(t);
    if (list.isEmpty()) {
        resMap.remove(resourceName);
    } else {
        resMap[resourceName] = list;
    }
    coreLog.details(tr("resource '%1' is released by '%2'").arg(resourceName).arg(t->getTaskName()));
    emit si_resourceUserUnregistered(resourceName, t);
}


QList<Task*> ResourceTracker::getResourceUsers(const QString& resourceName) {
    QList<Task*> res = resMap.value(resourceName);
    return res;
}




} //namespace
