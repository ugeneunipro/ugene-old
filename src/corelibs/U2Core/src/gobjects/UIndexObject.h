
#ifndef _U2_UINDEX_OBJECT_H_
#define _U2_UINDEX_OBJECT_H_

#include <U2Core/GObject.h>
#include <U2Core/UIndex.h>

#include "GObjectTypes.h"

namespace U2 {

class U2CORE_EXPORT UIndexObject : public GObject {
    Q_OBJECT
public:
    static const QString OBJ_NAME;
    
    UIndexObject( const UIndex& a_ind, const QString& name );
    UIndex getIndex() const;
    
    virtual GObject* clone() const;
    
private:
    UIndex ind;
    
}; // UIndexObject

} // U2

#endif // _U2_UINDEX_OBJECT_H_
