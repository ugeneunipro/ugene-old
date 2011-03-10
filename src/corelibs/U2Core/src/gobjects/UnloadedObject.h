#ifndef _U2_UNLOADED_OBJECT_H_
#define _U2_UNLOADED_OBJECT_H_

#include <U2Core/GObject.h>
#include "GObjectTypes.h"

namespace U2 {

class U2CORE_EXPORT UnloadedObjectInfo {
public:
    UnloadedObjectInfo(GObject* obj = NULL);
    QString         name;
    GObjectType     type;
    QVariantMap     hints;
    
    bool isValid() const {return !name.isEmpty() && !type.isEmpty() && type != GObjectTypes::UNLOADED;}
};

class U2CORE_EXPORT UnloadedObject: public GObject {
    Q_OBJECT
public:
    UnloadedObject(const QString& objectName, const GObjectType& loadedObjectType, const QVariantMap& hintsMap = QVariantMap());
    UnloadedObject(const UnloadedObjectInfo& info);

    virtual GObject* clone() const;
    
    GObjectType getLoadedObjectType() const {return loadedObjectType;}
    void setLoadedObjectType(const GObjectType& lot);

protected:
    GObjectType loadedObjectType;
};

}//namespace

#endif
