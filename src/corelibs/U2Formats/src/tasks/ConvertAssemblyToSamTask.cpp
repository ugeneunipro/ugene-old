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

#include <U2Core/AppContext.h>
#include <U2Core/U2AssemblyUtils.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DNASequence.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/U2AssemblyDbi.h>
#include <U2Core/U2AttributeDbi.h>
#include <U2Core/U2DbiUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/U2ObjectDbi.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Formats/SAMFormat.h>

#include <QSharedPointer>

#include "ConvertAssemblyToSamTask.h"

#include <memory>

namespace U2 {

ConvertAssemblyToSamTask::ConvertAssemblyToSamTask(GUrl db, GUrl sam)
: Task("ConvertAssemblyToSamTask", (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
dbFileUrl(db), samFileUrl(sam), handle(NULL)
{
}

ConvertAssemblyToSamTask::ConvertAssemblyToSamTask(const DbiHandle *h, GUrl sam)
: Task("ConvertAssemblyToSamTask", (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
samFileUrl(sam), handle(h)
{
}

void ConvertAssemblyToSamTask::run() {
    //init sam file
    SAMFormat samFormat;
    
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    std::auto_ptr<IOAdapter> io(iof->createIOAdapter());
    bool res = io->open(samFileUrl, IOAdapterMode_Write);
    SAFE_POINT(res == true, QString("Failed to open SAM file for write: %1").arg(samFileUrl.getURLString()),);
    
    //init assembly objects
    U2OpStatusImpl status;
    QSharedPointer<DbiHandle> dbiHandle;
    if (NULL == handle) {
        dbiHandle = QSharedPointer<DbiHandle>(new DbiHandle("SQLiteDbi", dbFileUrl.getURLString(), false, status));
        handle = dbiHandle.data();
    }
    U2ObjectDbi *odbi = handle->dbi->getObjectDbi();
    U2AssemblyDbi *assDbi = handle->dbi->getAssemblyDbi();
    QList<U2DataId> objectIds = odbi->getObjects("/", 0, U2_DBI_NO_LIMIT, status);
    U2Region wholeAssembly;
    wholeAssembly.startPos = 0;

    QVector<QByteArray> names;
    QVector<int> lengths;
    foreach(U2DataId id, objectIds) {
        U2DataType objectType = handle->dbi->getEntityTypeById(id);
        if (U2Type::Assembly == objectType) {
            U2Assembly assembly = handle->dbi->getAssemblyDbi()->getAssemblyObject(id, status);
            int length = handle->dbi->getAttributeDbi()->getIntegerAttribute(id, status).value;
            names.append(assembly.visualName.replace(QRegExp("\\s|\\t"), "_").toAscii());
            lengths.append(length);
        }
    }

    //writing to a sam file for an every object
    samFormat.storeHeader(io.get(), names, lengths);
    foreach(U2DataId id, objectIds) {
        U2DataType objectType = handle->dbi->getEntityTypeById(id);
        if (U2Type::Assembly == objectType) {
            U2Assembly assembly = handle->dbi->getAssemblyDbi()->getAssemblyObject(id, status);
            wholeAssembly.length = assDbi->getMaxEndPos(assembly.id, status);

            QByteArray refSeqName = assembly.visualName.replace(QRegExp("\\s|\\t"), "_").toAscii();
            U2DbiIterator<U2AssemblyRead> *dbiIterator = assDbi->getReads(assembly.id, wholeAssembly, status);

            DNASequence seq;
            while (dbiIterator->hasNext()) {
                U2AssemblyRead read = dbiIterator->next();
                read->cigar;

                seq.seq = read->readSequence;
                seq.quality = read->quality;
                seq.setName(read->name);
                QByteArray cigar = U2AssemblyUtils::cigar2String(read->cigar);
                if(!QRegExp("[!-~]+").exactMatch(seq.quality.qualCodes)) {
                    seq.quality.qualCodes = "*";
                }
                if (cigar.isEmpty()) {
                    cigar = "*";
                }

                samFormat.storeAlignedRead(read->leftmostPos, seq, io.get(), refSeqName, wholeAssembly.length, false, true, cigar);
            }
        }
    }
}

QString ConvertAssemblyToSamTask::generateReport() const {
    if (hasError() || isCanceled()) {
        return QString("Conversion task was finished with an error: %1").arg(getError());
    }

    return QString("Conversion task was finished. A new SAM file is: <a href=\"%1\">%2</a>").arg(samFileUrl.getURLString()).arg(samFileUrl.getURLString());
}

} // U2
