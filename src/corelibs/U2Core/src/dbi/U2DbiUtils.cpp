#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/AppContext.h>

namespace U2 {

//////////////////////////////////////////////////////////////////////////
U2AbstractDbi::U2AbstractDbi() {
    state = U2DbiState_Void;
}


//////////////////////////////////////////////////////////////////////////
// U2DbiHandle
DbiHandle::DbiHandle(U2DbiFactoryId id, const QString& url,  U2OpStatus& _os) : dbi(NULL), os(_os) {
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(id, url, false, os);
}

DbiHandle::DbiHandle(U2DbiFactoryId id, const QString& url,  bool create, U2OpStatus& _os) : dbi(NULL), os(_os) {
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(id, url, false, os);
}

DbiHandle::~DbiHandle() {
    if (dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->releaseDbi(dbi, os);
    }
}


} //namespace