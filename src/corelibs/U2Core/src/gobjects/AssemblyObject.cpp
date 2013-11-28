/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2AttributeUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatus.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2SafePoints.h>

#include "AssemblyObject.h"

namespace U2 {

AssemblyObject::AssemblyObject(const U2EntityRef& ref, const QString& objectName, const QVariantMap& hints) 
: GObject(GObjectTypes::ASSEMBLY, objectName, hints)
{
    this->entityRef = ref;
}

GObject* AssemblyObject::clone(const U2DbiRef &dstDbiRef, U2OpStatus &os) const {
    U2EntityRef dstEntityRef = AssemblyObject::dbi2dbiClone(this, dstDbiRef, os);
    CHECK_OP(os, NULL);
    AssemblyObject *dstObj = new AssemblyObject(dstEntityRef, this->getGObjectName(), this->getGHintsMap());

    return dstObj;
}

U2EntityRef AssemblyObject::dbi2dbiClone(const AssemblyObject *const srcObj, const U2DbiRef &dstDbiRef, U2OpStatus &os) {
    U2DbiRef srcDbiRef = srcObj->getEntityRef().dbiRef;
    U2DataId srcObjId = srcObj->getEntityRef().entityId;
    DbiConnection dstCon(dstDbiRef, true, os);
    CHECK_OP(os, U2EntityRef());
    DbiConnection srcCon(srcDbiRef, os);
    CHECK_OP(os, U2EntityRef());

    U2ObjectDbi *dstObjectDbi = dstCon.dbi->getObjectDbi();
    U2ObjectDbi *srcObjectDbi = srcCon.dbi->getObjectDbi();
    SAFE_POINT_EXT(NULL != dstObjectDbi, os.setError("NULL destination object dbi"), U2EntityRef());
    SAFE_POINT_EXT(NULL != srcObjectDbi, os.setError("NULL source object dbi"), U2EntityRef());

    U2AssemblyDbi *dstAssemblyDbi = dstCon.dbi->getAssemblyDbi();
    U2AssemblyDbi *srcAssemblyDbi = srcCon.dbi->getAssemblyDbi();
    SAFE_POINT_EXT(NULL != dstAssemblyDbi, os.setError("NULL destination assembly dbi"), U2EntityRef());
    SAFE_POINT_EXT(NULL != srcAssemblyDbi, os.setError("NULL source assembly dbi"), U2EntityRef());

    // copy object folders
    QStringList dstFolders = dstObjectDbi->getFolders(os);
    CHECK_OP(os, U2EntityRef());
    QStringList srcFolders = srcObjectDbi->getObjectFolders(srcObjId, os);
    CHECK_OP(os, U2EntityRef());
    foreach (const QString &folder, srcFolders) {
        if (!dstFolders.contains(folder)) {
            dstObjectDbi->createFolder(folder, os);
            CHECK_OP(os, U2EntityRef());
        }
    }

    // copy object
    U2Assembly assembly;
    assembly.visualName = srcObj->getGObjectName();
    U2AssemblyReadsImportInfo info;
    dstAssemblyDbi->createAssemblyObject(assembly, srcFolders.first(), NULL, info, os);
    CHECK_OP(os, U2EntityRef());
    QList<U2DataId> objList;
    objList << assembly.id;
    foreach (const QString &folder, srcFolders) {
        dstObjectDbi->addObjectsToFolder(objList, folder, os);
        CHECK_OP(os, U2EntityRef());
    }

    // copy reads
    qint64 assemblyLength = srcAssemblyDbi->getMaxEndPos(srcObjId, os) + 1;
    CHECK_OP(os, U2EntityRef());
    U2DbiIterator<U2AssemblyRead> *iter = srcAssemblyDbi->getReads(srcObjId, U2Region(0, assemblyLength), os);
    QScopedPointer< U2DbiIterator<U2AssemblyRead> > iterPtr(iter);
    CHECK_OP(os, U2EntityRef());
    Q_UNUSED(iterPtr);

    dstAssemblyDbi->addReads(assembly.id, iter, os);
    CHECK_OP(os, U2EntityRef());

    // copy attributes
    U2AttributeDbi *srcAttributeDbi = srcCon.dbi->getAttributeDbi();
    U2AttributeDbi *dstAttributeDbi = dstCon.dbi->getAttributeDbi();
    U2AttributeUtils::copyObjectAttributes(srcObjId, assembly.id, srcAttributeDbi, dstAttributeDbi, os);
    CHECK_OP(os, U2EntityRef());

    U2EntityRef dstEntityRef(dstDbiRef, assembly.id);
    return dstEntityRef;
}


}//namespace
