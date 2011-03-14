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
        : GObject(GObjectTypes::ASSEMBLY, objectName, hints), objectView(0), dbiRef(ref){};

    virtual GObject* clone() const;
    const U2DataRef& getDbiRef() const {return dbiRef;}
    inline void setView(QObject * view) {objectView = view;}
    inline QObject * getView() const {return objectView;}
protected:
    QObject * objectView;
    U2DataRef dbiRef;
};

}//namespace


#endif
