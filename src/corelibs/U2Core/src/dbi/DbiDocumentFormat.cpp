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

#include "DbiDocumentFormat.h"

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2DbiRegistry.h>
#include <U2Core/AssemblyObject.h>

namespace U2 {

DbiDocumentFormat::DbiDocumentFormat(const U2DbiFactoryId& _id, const DocumentFormatId& _formatId, 
                                     const QString& _formatName, const QStringList& exts, QObject* p) 
: DocumentFormat(p, DocumentFormatFlags_W1, exts) 
{
    id = _id;
    formatId = _formatId;
    formatName = _formatName;
    supportedObjectTypes+=GObjectTypes::ASSEMBLY;
    formatFlags|=DocumentFormatFlag_NoPack;
    formatFlags|=DocumentFormatFlag_DoNotLoadsMemory;
}    


Document* DbiDocumentFormat::createNewDocument(IOAdapterFactory* io, const QString& url, const QVariantMap& fs) {
    Document* d = DocumentFormat::createNewDocument(io, url, fs);
    return d;
}

Document* DbiDocumentFormat::loadDocument(IOAdapter* io, TaskStateInfo& ts, const QVariantMap& fs, DocumentLoadMode) {
    //1. open db
    //2. read all assembly & sequence objects
    //3. close db
    QString url = io->getURL().getURLString();
    U2OpTaskStatus os(&ts);
    DbiHandle handle(id, url, true, os);
    if(os.isCoR()) {
        return NULL;
    }
    
    U2ObjectDbi* odbi = handle.dbi->getObjectDbi();
    QList<U2DataId> objectIds = odbi->getObjects("/", 0, U2_DBI_NO_LIMIT, os);
    if (os.isCoR()) {
        return NULL;
    }
    QList<GObject*> objects;
    U2DataRef ref;
    ref.dbiId = url;
    ref.factoryId = id;
    int assemblyNumber = 0;
    foreach(U2DataId id, objectIds) {
        U2DataType objectType = handle.dbi->getEntityTypeById(id);
        switch (objectType) {
            case U2Type::Assembly: 
                ref.entityId = id;
                 
                objects.append(new AssemblyObject(ref, "Assembly_" + QString::number(assemblyNumber++), QVariantMap()));
                break;
            default: // do nothing
                break;
        }
    }

    Document* d = new Document(this, io->getFactory(), io->getURL(), objects, fs);
    return d;
}

void DbiDocumentFormat::storeDocument(Document* d, TaskStateInfo& ts, IOAdapter* io) {
    // 1. get db
    // 2. call sync
    QString url = d->getURLString();
    U2OpTaskStatus os(&ts);
    DbiHandle handle(id, url, true, os);
    if (!os.isCoR()) {
        handle.dbi->flush(os);
    }
}

FormatDetectionResult DbiDocumentFormat::checkRawData(const QByteArray& rawData, const GUrl& url) const {
    U2DbiFactory* f = AppContext::getDbiRegistry()->getDbiFactoryById(id);
    if (f != NULL) {
        QHash<QString, QString> props;
        props[U2_DBI_OPTION_URL] = url.getURLString();
        U2OpStatusImpl os;
        bool ok = f->isValidDbi(props, rawData, os) && !os.hasError();
        if (ok) {
            return FormatDetection_Matched;
        }
    }
    return FormatDetection_NotMatched;
}


}//namespace
