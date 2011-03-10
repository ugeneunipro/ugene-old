#include "UnloadedObject.h"

#include <U2Core/GHints.h>

namespace U2 {

UnloadedObject::UnloadedObject(const QString& objectName, const GObjectType& lot, const QVariantMap& hintsMap)
: GObject(GObjectTypes::UNLOADED, objectName, hintsMap)
{
    setLoadedObjectType(lot);
}

UnloadedObject::UnloadedObject(const UnloadedObjectInfo& info) 
: GObject(GObjectTypes::UNLOADED, info.name, info.hints) 
{
    setLoadedObjectType(info.type);
}

GObject* UnloadedObject::clone() const {
    UnloadedObject* cln = new UnloadedObject(getGObjectName(), getLoadedObjectType(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

void UnloadedObject::setLoadedObjectType(const GObjectType& lot) {
    assert(lot!=GObjectTypes::UNLOADED);
    loadedObjectType = lot;
}

UnloadedObjectInfo::UnloadedObjectInfo(GObject* obj) {
    if (obj == NULL) {
        return;
    }
    name = obj->getGObjectName();
    hints = obj->getGHintsMap();
    if (obj->isUnloaded()) {
        UnloadedObject* uo = qobject_cast<UnloadedObject*>(obj);
        type = uo->getLoadedObjectType();
    } else {
        type = obj->getGObjectType();
    }
}

}//namespace
