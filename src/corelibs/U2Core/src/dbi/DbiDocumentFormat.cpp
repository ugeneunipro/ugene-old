/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2016 UniPro <ugene@unipro.ru>
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

#include <U2Core/AppContext.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceDbi.h>

#include "DbiDocumentFormat.h"

namespace U2 {

DbiDocumentFormat::DbiDocumentFormat(const U2DbiFactoryId& _id, const DocumentFormatId& _formatId,
                                     const QString& _formatName, const QStringList& exts, DocumentFormatFlags flags, QObject* p)
: DocumentFormat(p, flags, exts)
{
    id = _id;
    formatId = _formatId;
    formatName = _formatName;
    formatDescription = tr("ugenedb is a internal UGENE database file format");
    supportedObjectTypes+=GObjectTypes::ASSEMBLY;
    formatFlags|=DocumentFormatFlag_NoPack;
    formatFlags|=DocumentFormatFlag_NoFullMemoryLoad;
    formatFlags|=DocumentFormatFlag_DirectWriteOperations;
}



static void renameObjectsIfNamesEqual(QList<GObject*> & objs) {
    for(int i = 0; i < objs.size(); ++i) {
        int howManyEquals = 0;
        for(int j = i + 1; j < objs.size(); ++j) {
            if(objs[i]->getGObjectName() == objs[j]->getGObjectName()) {
                objs[j]->setGObjectName(QString("%1 %2").arg(objs[j]->getGObjectName()).arg(++howManyEquals));
            }
        }
    }
}

Document* DbiDocumentFormat::loadDocument(IOAdapter* io, const U2DbiRef& dstDbiRef, const QVariantMap& fs, U2OpStatus& os){
    //1. open db
    //2. read all objects
    //3. if there is a DEEP_COPY_OBJECT hint, all objects are cloned to the db defined by dstDbiRef
    //3. close db
    QString url = io->getURL().getURLString();
    U2DbiRef srcDbiRef(id, url);
    DbiConnection handle(srcDbiRef, true, os);
    CHECK_OP(os, NULL);

    U2ObjectDbi* odbi = handle.dbi->getObjectDbi();
    QList<U2DataId> objectIds = odbi->getObjects(U2ObjectDbi::ROOT_FOLDER, 0, U2DbiOptions::U2_DBI_NO_LIMIT, os);
    CHECK_OP(os, NULL);

    QList<GObject*> objects;
    U2EntityRef ref;
    ref.dbiRef = srcDbiRef;

    objects << prepareObjects(handle, objectIds);

    if (fs.value(DEEP_COPY_OBJECT, false).toBool()) {
        QList<GObject *> clonedObjects = cloneObjects(objects, dstDbiRef, fs, os);
        qDeleteAll(objects);
        CHECK_OP_EXT(os, qDeleteAll(clonedObjects), NULL);
        objects = clonedObjects;
    } else {
        renameObjectsIfNamesEqual(objects);
    }

    QString lockReason = handle.dbi->isReadOnly() ? "The database is read-only" : "";
    Document* d = new Document(this, io->getFactory(), io->getURL(), dstDbiRef, objects, fs, lockReason);
    d->setDocumentOwnsDbiResources(false);

    return d;
}

QList<GObject *> DbiDocumentFormat::prepareObjects(DbiConnection &handle, const QList<U2DataId> &objectIds) {
    QList<GObject *> objects;
    U2EntityRef ref;
    ref.dbiRef = handle.dbi->getDbiRef();

    foreach(const U2DataId &id, objectIds) {
        U2OpStatus2Log status;
        ref.entityId = id;

        U2Object object;
        handle.dbi->getObjectDbi()->getObject(object, id, status);
        CHECK_OPERATION(!status.isCoR(), continue);

        if (object.visualName.isEmpty()) {
            object.visualName = "Unnamed object";
        }

        GObject *gobject = GObjectUtils::createObject(ref.dbiRef, id, object.visualName);
        CHECK_OPERATION(NULL != gobject, continue);
        objects << gobject;
    }

    return objects;
}

QList<GObject *> DbiDocumentFormat::cloneObjects(const QList<GObject *> &srcObjects, const U2DbiRef &dstDbiRef, const QVariantMap &hints, U2OpStatus &os) {
    QList<GObject *> clonedObjects;
    CHECK_EXT(dstDbiRef.isValid(), os.setError(tr("Invalid destination database reference")), clonedObjects);

    int number = 0;
    int total = srcObjects.size();
    foreach (GObject *srcObject, srcObjects) {
        U2OpStatusMapping mapping(number, (number + 1) / total);
        U2OpStatusChildImpl childOs(&os, mapping);
        GObject *clonedObject = srcObject->clone(dstDbiRef, childOs, hints);
        CHECK_OP(os, clonedObjects);
        clonedObjects << clonedObject;
        number++;
    }

    return clonedObjects;
}

void DbiDocumentFormat::storeDocument(Document* d, IOAdapter* ioAdapter, U2OpStatus& os) {
    const QString url = ioAdapter->getURL().getURLString();

    const U2DbiRef dstDbiRef(id, url);
    DbiConnection dstCon(dstDbiRef, true, os);
    CHECK_OP(os, );
    Q_UNUSED(dstCon);

    foreach (GObject *object, d->getObjects()) {
        if (!supportedObjectTypes.contains(object->getGObjectType())) {
            continue;
        }

        U2DbiRef srcDbiRef = object->getEntityRef().dbiRef;
        if (srcDbiRef == dstDbiRef) {
            // do not need to import
            continue;
        }

        GObject *resultObject = object->clone(dstDbiRef, os);
        CHECK_OP(os, );

        delete resultObject;
    }
}

FormatCheckResult DbiDocumentFormat::checkRawData(const QByteArray& rawData, const GUrl& url) const {
    U2DbiFactory* f = AppContext::getDbiRegistry()->getDbiFactoryById(id);
    if (f != NULL) {
        QHash<QString, QString> props;
        props[U2DbiOptions::U2_DBI_OPTION_URL] = url.getURLString();
        U2OpStatusImpl os;
        FormatCheckResult r = f->isValidDbi(props, rawData, os);
        if (!os.hasError()) {
            return r;
        }
    }
    return FormatDetection_NotMatched;
}


}//namespace
