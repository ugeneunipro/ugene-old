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
#include <U2Core/U2OpStatus.h>
#include <U2Core/AppContext.h>

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

//////////////////////////////////////////////////////////////////////////
// U2DbiHandle
DbiHandle::DbiHandle(U2DbiFactoryId id, const QString& url,  U2OpStatus& _os) : dbi(NULL), os(_os) {
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(id, url, false, os);
}

DbiHandle::DbiHandle(U2DbiFactoryId id, const QString& url,  bool create, U2OpStatus& _os) : dbi(NULL), os(_os) {
    dbi = AppContext::getDbiRegistry()->getGlobalDbiPool()->openDbi(id, url, create, os);
}

DbiHandle::DbiHandle(const DbiHandle & dbiHandle_) : dbi(dbiHandle_.dbi), os(dbiHandle_.os) {
    if (dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->addRef(dbi, os);
    }
}

DbiHandle::~DbiHandle() {
    if (dbi != NULL) {
        AppContext::getDbiRegistry()->getGlobalDbiPool()->releaseDbi(dbi, os);
    }
}


} //namespace
