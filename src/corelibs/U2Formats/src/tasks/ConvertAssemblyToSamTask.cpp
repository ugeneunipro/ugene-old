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
#include <U2Core/U2DbiUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Formats/SAMFormat.h>

#include <QSharedPointer>

#include "ConvertAssemblyToSamTask.h"

namespace U2 {

ConvertAssemblyToSamTask::ConvertAssemblyToSamTask(GUrl db, GUrl sam)
: Task("ConvertAssemblyToSamTask", (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
dbFileUrl(db), samFileUrl(sam), loadDbiTask(NULL), assObj(NULL), handle(NULL)
{
    QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(dbFileUrl);
    if (!detectedFormats.isEmpty()) {
        IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        loadDbiTask = new LoadDocumentTask(detectedFormats.first()->getFormatId(), dbFileUrl, factory);
        addSubTask(loadDbiTask);
    } else {
        setError("Unsupported file format.");
        return;
    }
}

ConvertAssemblyToSamTask::ConvertAssemblyToSamTask(AssemblyObject *o, const DbiHandle *h, GUrl sam)
: Task("ConvertAssemblyToSamTask", (TaskFlag)(TaskFlag_ReportingIsSupported | TaskFlag_ReportingIsEnabled)),
samFileUrl(sam), loadDbiTask(NULL), assObj(o), handle(h)
{
}

QList<Task*> ConvertAssemblyToSamTask::onSubTaskFinished(Task* ) {
    QList<Task*> subTasks;

    Document *d = loadDbiTask->getDocument();
    QList<GObject*> objects = d->findGObjectByType(GObjectTypes::ASSEMBLY);

    if (objects.size() < 1) {
        setError("Can't find assembly objects.");
        return subTasks;
    }
    assObj = qobject_cast<AssemblyObject*>(objects.first());

    return subTasks;
}

void ConvertAssemblyToSamTask::run() {
    U2OpStatusImpl status;
    QSharedPointer<DbiHandle> dbiHandle;
    if (NULL == handle) {
        dbiHandle = QSharedPointer<DbiHandle>(new DbiHandle("SQLiteDbi", dbFileUrl.getURLString(), false, status));
        handle = dbiHandle.data();
    }
    U2Dbi *dbi = handle->dbi;
    U2Assembly assembly = dbi->getAssemblyDbi()->getAssemblyObject(assObj->getDbiRef().entityId, status);

    U2AssemblyDbi *assDbi = dbi->getAssemblyDbi();
    U2Region wholeAssembly;
    wholeAssembly.startPos = 0;
    wholeAssembly.length = assDbi->getMaxEndPos(assembly.id, status);

    QByteArray refSeqName = QString(assObj->getGObjectName()).replace(QRegExp("\\s|\\t"), "_").toAscii();
    DocumentFormat *f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::SAM);
    SAMFormat *format = qobject_cast<SAMFormat*> (f);
    assert(format != NULL);

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
    IOAdapter *io = iof->createIOAdapter();
    bool res = io->open(samFileUrl, IOAdapterMode_Write);
    assert(res == true);
    Q_UNUSED(res);

    U2DbiIterator<U2AssemblyRead> *dbiIterator = assDbi->getReads(assembly.id, wholeAssembly, status);

    bool first = true;
    DNASequence seq;
    while (dbiIterator->hasNext()) {
        U2AssemblyRead read = dbiIterator->next();
        read->cigar;

        
        seq.seq = read->readSequence;
        seq.quality = read->quality;
        seq.setName(read->name);

        format->storeAlignedRead(read->leftmostPos, seq, io, refSeqName, wholeAssembly.length, first, true, U2AssemblyUtils::cigar2String(read->cigar));
        first = false;
    }

    io->close();
}

QString ConvertAssemblyToSamTask::generateReport() const {
    if (hasError() || isCanceled()) {
        return QString("Conversion task was finished with an error: %1").arg(getError());
    }

    return QString("Conversion task was finished. A new SAM file is: <a href=\"%1\">%2</a>").arg(samFileUrl.getURLString()).arg(samFileUrl.getURLString());
}

} // U2
