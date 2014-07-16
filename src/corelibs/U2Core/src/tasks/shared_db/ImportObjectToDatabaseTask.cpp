/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include <QtCore/QCoreApplication>

#include <U2Core/DocumentModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>

#include "ImportObjectToDatabaseTask.h"

namespace U2 {

ImportObjectToDatabaseTask::ImportObjectToDatabaseTask(GObject* object, const U2DbiRef& dstDbiRef, const QString& dstFolder) :
    Task(tr("Import object %1 to database").arg(NULL != object ? object->getGObjectName() : ""), TaskFlag_None),
    object(object),
    dstDbiRef(dstDbiRef),
    dstFolder(dstFolder),
    dstObject(NULL)
{
    CHECK_EXT(NULL != object, setError(tr("Invalid object to import")), );
    CHECK_EXT(dstDbiRef.isValid(), setError(tr("Invalid database reference")), );
    tpm = Progress_Manual;
}

void ImportObjectToDatabaseTask::run() {
    DbiOperationsBlock opBlock(dstDbiRef, stateInfo);
    Q_UNUSED(opBlock);
    CHECK_OP(stateInfo, );

    DbiConnection con(dstDbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    SAFE_POINT_EXT(NULL != con.dbi, setError(QObject::tr("Error! No DBI")), );
    U2ObjectDbi *oDbi = con.dbi->getObjectDbi();
    SAFE_POINT_EXT(NULL != oDbi, setError(QObject::tr("Error! No object DBI")), );

    CHECK_EXT(!object.isNull(), setError(tr("The object has been removed")), );
    dstObject = object->clone(dstDbiRef, stateInfo);
    CHECK_OP(stateInfo, );
    dstObject->moveToThread(QCoreApplication::instance()->thread());

    // TODO: clone to the destination folder
    QStringList folders = oDbi->getFolders(stateInfo);
    CHECK_OP(stateInfo, );

    if (!folders.contains(dstFolder)) {
        oDbi->createFolder(dstFolder, stateInfo);
        CHECK_OP(stateInfo, );
    }

    oDbi->moveObjects(QList<U2DataId>() << dstObject->getEntityRef().entityId, U2ObjectDbi::ROOT_FOLDER, dstFolder, stateInfo);
    CHECK_OP(stateInfo, );
}

GObject * ImportObjectToDatabaseTask::takeResult() {
    GObject *result = dstObject;
    dstObject = NULL;
    return result;
}

GObject * ImportObjectToDatabaseTask::getSourceObject() const {
    return object.data();
}

GObject *ImportObjectToDatabaseTask::getDestinationObject() const {
    return dstObject;
}

const QString & ImportObjectToDatabaseTask::getFolder() const {
    return dstFolder;
}

const U2DbiRef & ImportObjectToDatabaseTask::getDbiRef() const {
    return dstDbiRef;
}

}   // namespace U2
