#ifndef _U2_ASSEMBLY_OBJECT_H_
#define _U2_ASSEMBLY_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/U2Type.h>

namespace U2 {

class U2CORE_EXPORT AssemblyObject : public GObject {
    Q_OBJECT
public:
    AssemblyObject(const U2DataRef& ref, const QString& objectName, const QVariantMap& hints) 
        : GObject(GObjectTypes::ASSEMBLY, objectName, hints), dbiRef(ref){};

    virtual GObject* clone() const;
    const U2DataRef& getDbiRef() const {return dbiRef;}
protected:
    U2DataRef dbiRef;
};

}//namespace


#endif
