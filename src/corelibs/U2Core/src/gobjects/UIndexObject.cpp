
#include "UIndexObject.h"

namespace U2 {

const QString UIndexObject::OBJ_NAME = "Index";

UIndexObject::UIndexObject( const UIndex& a_ind, const QString& name ) : GObject( GObjectTypes::UINDEX, name ),
                                                                         ind( a_ind ){}

UIndex UIndexObject::getIndex() const {
    return ind;
}

GObject* UIndexObject::clone() const {
    UIndexObject* cln = new UIndexObject( ind, getGObjectName() );
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

} // U2
