#include "AssemblyObject.h"

namespace U2 {

GObject* AssemblyObject::clone() const {
    AssemblyObject* cln = new AssemblyObject(dbiRef, getGObjectName(), getGHintsMap());
    return cln;
}


}//namespace


