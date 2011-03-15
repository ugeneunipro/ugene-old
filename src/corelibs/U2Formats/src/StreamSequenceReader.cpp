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
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>

#include "StreamSequenceReader.h"

namespace U2 {

const DNASequenceObject* StreamSequenceReader::getNextSequenceObject()
{
    if (hasNext()) {
        
        QList<GObject*> objs = currentDoc->findGObjectByType(GObjectTypes::SEQUENCE);
        if (objs.size() != 1) {
            return NULL;
        }
        DNASequenceObject* result =  qobject_cast<DNASequenceObject*>(objs.first());
        lookupPerformed = false;

        return result;
    }   
    
    return NULL;
}

StreamSequenceReader::StreamSequenceReader()
: currentReaderIndex(-1), currentDoc(NULL), errorOccured(false), lookupPerformed(false)
{

}

bool StreamSequenceReader::hasNext()
{
    if (readers.isEmpty()) {
        return false;
    }

    if (!lookupPerformed) {
    
        if (currentReaderIndex < 0 || currentReaderIndex >= readers.count()) {
            return false;
        }
        
        lookupPerformed = true;
        ReaderContext ctx = readers.at(currentReaderIndex);
        
        Document* newDoc = ctx.format->loadDocument(ctx.io, taskInfo, QVariantMap(), DocumentLoadMode_SingleObject);
        currentDoc.reset(newDoc);
        int progress = ctx.io->getProgress();
        if (progress == 100 && currentReaderIndex + 1 < readers.count()) {
            ++currentReaderIndex;    
        }
    }

    if (currentDoc.get() == NULL) {
        return false;
    }

    return ( currentDoc->getObjects().size() == 1 );
}

bool StreamSequenceReader::init( const QList<GUrl>& urls )
{
    foreach (const GUrl& url, urls) {
        QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(url);    
        if (detectedFormats.isEmpty()) {
            taskInfo.setError(QString("File %1 unsupported format.").arg(url.getURLString()));
            break;
        }
        ReaderContext ctx;
        ctx.format = detectedFormats.first();
        if ( ctx.format->getFlags().testFlag(DocumentFormatFlag_SupportStreaming) == false  ) {
            break;
        }
        IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        IOAdapter* io = factory->createIOAdapter();
        if (!io->open(url, IOAdapterMode_Read)) {
            break;
        }
        ctx.io = io;
        readers.append(ctx);
    }
    
    if (readers.isEmpty()) {
        taskInfo.setError("Unsupported file format or short reads list is empty");
        return false;
    } else {
        currentReaderIndex = 0;
        return true;
    }


}

QString StreamSequenceReader::getErrorMessage()
{
    return taskInfo.getError();
}

int StreamSequenceReader::getProgress()
{
    if (readers.count() == 0) {
        return 0;
    }

    float factor = 1/readers.count();
    int progress = 0;
    for (int i = 0; i < readers.count(); ++i) {
        progress += (int)( (readers[i].io->getProgress() / 100.0 )*factor) * 100;
    }

    return progress;
}

StreamSequenceReader::~StreamSequenceReader()
{
    for(int i =0; i < readers.size(); ++i) {
        delete readers[i].io;
        readers[i].io = NULL;
    }
}


} //namespace
