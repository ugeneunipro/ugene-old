#include "AppContextImpl.h"
#include <U2Core/AppGlobalObject.h>

namespace U2 {

static AppContextImpl appContext;

AppContextImpl::~AppContextImpl() {
    for (int i=appGlobalObjects.size();  --i>=0; ) { //remove objects in reverse mode, so the last added deleted first
        AppGlobalObject* obj = appGlobalObjects[i];
        delete obj;
    }
}

AppContextImpl* AppContextImpl::getApplicationContext() { 
    return &appContext; 
}


void AppContextImpl::_registerGlobalObject(AppGlobalObject* go) {
    const QString& id = go->getId();
    assert(_getGlobalObjectById(id) == NULL);
    Q_UNUSED(id);
    appGlobalObjects.append(go);
}

void AppContextImpl::_unregisterGlobalObject(const QString& id) {
    for (int i=0, n = appGlobalObjects.size(); i < n; i++) {
        AppGlobalObject* obj = appGlobalObjects.at(i);
        if (obj->getId() == id) {
            appGlobalObjects.removeAt(i);
            break;
        }
    }
}

AppGlobalObject* AppContextImpl::_getGlobalObjectById(const QString& id) const {
    foreach(AppGlobalObject* obj, appGlobalObjects) {
        if (obj->getId() == id) {
            return obj;
        }
    }
    return NULL;
}




}//namespace

