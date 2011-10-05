/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2ObjectDbi.h>

#include <QtCore/QFile>

namespace U2 {

void U2DbiUtils::logNotSupported(U2DbiFeature f, U2Dbi* dbi, U2OpStatus& os) {
    QString msg = tr("Feature is not supported: %1, dbi: %2").arg(int(f)).arg(dbi == NULL ? QString("<unknown>") : dbi->getDbiId());
    coreLog.error(msg);
    if (!os.hasError()) {
        os.setError(msg);
    }

#ifdef _DEBUG
    if (dbi->getFeatures().contains(f)) {
        coreLog.error("Calling not-supported method while features is listed in supported features list!");
    }
    assert(0);
#endif
}

U2DbiRef U2DbiUtils::toRef(U2Dbi* dbi) {
    if (dbi == NULL) {
        return U2DbiRef();
    }
    return U2DbiRef(dbi->getFactoryId(), dbi->getDbiId());
}

//////////////////////////////////////////////////////////////////////////
// DbiConnection
DbiConnection::DbiConnection(const U2DbiRef& ref,  U2OpStatus& _os) : dbi(NULL), os(_os) {
    open(ref, os);    
}

DbiConnection::DbiConnection(const U2DbiRef& ref,  bool create, U2OpStatus& _os) : dbi(NULL), os(_os) {
    open(ref, create, os);
}

DbiConnection::DbiConnection(const DbiConnection& dbiConnection) : dbi(dbiConnection.dbi), os(dbiConnection.os) {
    if (dbiConnection.dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->addRef(dbi, os);
    }
}

DbiConnection::~DbiConnection() {
    close();
}

void DbiConnection::open(const U2DbiRef& ref,  U2OpStatus& _os)  {
    SAFE_POINT_EXT(!isOpen(), _os.setError(QString("Connection is already opened! %1").arg(dbi->getDbiId())), );
    os = _os;
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(ref, false, os);
}

void DbiConnection::open(const U2DbiRef& ref,  bool create, U2OpStatus& _os)  {
    SAFE_POINT_EXT(!isOpen(), _os.setError(QString("Connection is already opened! %1").arg(dbi->getDbiId())), );
    os = _os;
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(ref, create, os);
}

void DbiConnection::close() {
    if (dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->releaseDbi(dbi, os);
        dbi = NULL;
    }
}

static U2OpStatus& getStubOpStatus() {
    static U2OpStatusImpl stubOs;
    return stubOs;
}

DbiConnection::DbiConnection() : dbi(NULL), os(getStubOpStatus()) {
}

//////////////////////////////////////////////////////////////////////////
// TmpDbiHandle

TmpDbiHandle::TmpDbiHandle(const QString& alias, U2OpStatus& os)  {
    deallocate = false;
    dbiRef = AppContext::getDbiRegistry()->allocateTmpDbi(alias, os);
    CHECK_OP(os, );
    deallocate = true;
}

TmpDbiHandle::TmpDbiHandle(const U2DbiRef& _dbiRef, bool _deallocate) 
: dbiRef(_dbiRef), deallocate(_deallocate)
{
}

TmpDbiHandle::~TmpDbiHandle () {
    if (deallocate) {
        U2OpStatus2Log os;
        AppContext::getDbiRegistry()->deallocateTmpDbi(dbiRef, os);
    }
}

//////////////////////////////////////////////////////////////////////////
// TmpDbiObjects
TmpDbiObjects::~TmpDbiObjects() {
    if (os.isCoR()) {
        foreach(const U2DataId& id, objects) {
            if (!id.isEmpty()) {
                U2OpStatus2Log os2log;
                DbiConnection con(dbiRef, os2log);
                con.dbi->getObjectDbi()->removeObject(id, os2log);
            }
        }
    }
}


} //namespace
