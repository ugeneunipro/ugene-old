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

#include "CreateFileIndexTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/LocalFileAdapter.h>
#include <U2Core/ZlibAdapter.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/Log.h>
#include <U2Core/Counter.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/BaseDocumentFormats.h>

#include <U2Core/UIndexObject.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/MAlignmentObject.h>
//#include <U2Formats/StockholmFormat.h>

#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <memory>


namespace U2 {

const QString ACCESS_POINTS_SZ_TAG = "APS";
const QString ACCESS_POINT_TAG     = "AP";
const QString ACCESS_POINT_WND_TAG = "APW";
const QString SPACE                = " ";

static void fillGzipIndex( UIndex::IOSection& ioSec, const GZipIndex& gzInd ) {
    int sz = gzInd.points.size();
    int i = 0;
    
    ioSec.keys.insert( ACCESS_POINTS_SZ_TAG, QString::number( sz ) );
    for( i = 0; i < sz; ++i ) {
        const GZipIndexAccessPoint& p = gzInd.points.at( i );

        QString numTag = ACCESS_POINT_TAG + QString::number( i );
        QString numVal = QString::number( p.bits ) + SPACE + QString::number( p.in ) + SPACE + QString::number( p.out );
        ioSec.keys.insert( numTag, numVal );

        QString wndTag = ACCESS_POINT_WND_TAG + QString::number( i );
        QString wndVal = p.window;
        ioSec.keys.insert( wndTag, wndVal );
    }
}

const QString CreateFileIndexTask::KEY_NAME         = "name";
const QString CreateFileIndexTask::KEY_SEQ_LEN      = "sequence_length";
const QString CreateFileIndexTask::KEY_SEQ_COUNT    = "sequence_count";
const QString CreateFileIndexTask::KEY_ACCESSION    = "accession";
const QString CreateFileIndexTask::KEY_DESCRIPTION  = "description";

CreateFileIndexTask::CreateFileIndexTask( const QList< QString >& i, const QString& o,
                                         const QList< IOAdapterFactory* >& ifa, IOAdapterFactory* ofa )
:Task( tr( "Create index: %1" ).arg(QFileInfo(o).fileName()), TaskFlag_None ),
inputUrls( i ), outputUrl( o ), inputFactories( ifa ), outputFactory( ofa ) 
{
    GCOUNTER( cvar, tvar, "CreateFileIndexTask" );
    setVerboseLogMode(true);
    tpm = Progress_Manual;

    if( !inputUrls.size() ) {
        stateInfo.setError(tr( "No files to index" ));
        return;
    }
    if( inputUrls.size() != inputFactories.size() ) {
        stateInfo.setError(tr( "Internal error: inconsistent input data" ));
        return;
    }
    if( outputUrl.isEmpty() || !outputFactory ) {
        stateInfo.setError(tr( "Illegal output file parameters" ));
        return;
    }
}

void CreateFileIndexTask::fillIOSec( UIndex::IOSection& ioSec, const QString& url, IOAdapterFactory* factory, int num ) {
    assert( NULL != factory );
    ioSec.sectionId   = QString( "io_" ) + QString::number( num );
    ioSec.ioAdapterId = factory->getAdapterId();
    ioSec.url         = url;
    
    if( BaseIOAdapters::LOCAL_FILE == ioSec.ioAdapterId ) {
        return; // filled all what we need
    }
    else if ( BaseIOAdapters::GZIPPED_LOCAL_FILE == ioSec.ioAdapterId ) {
        IOAdapterFactory* tmpFactory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( BaseIOAdapters::LOCAL_FILE );
        LocalFileAdapterFactory* localFactory = qobject_cast< LocalFileAdapterFactory* >( tmpFactory );
        assert( NULL != localFactory );
        std::auto_ptr< LocalFileAdapter > localAdapter( qobject_cast< LocalFileAdapter* >( localFactory->createIOAdapter() ) );
        if( !localAdapter->open( url, IOAdapterMode_Read ) ) {
            stateInfo.setError(tr( "Can't open file for read: '%1'" ).arg( url ));
            return;
        }
        bool indOk = false;
        GZipIndex gzInd = ZlibAdapter::buildGzipIndex( localAdapter.get(), GZipIndex::SPAN, &indOk );
        if( !indOk ) {
            stateInfo.setError(tr( "Error creating gzipped output" ));
            return;
        }
        fillGzipIndex( ioSec, gzInd );
        return;
    }
    else { // other io adapters not supported
        stateInfo.setError(tr( "Can't create index using %1 IO adapter").arg(ioSec.ioAdapterId));
    }
}

void CreateFileIndexTask::readOneUrl( const QString& url, IOAdapterFactory* inputFactory, int num ) {
    assert( !url.isEmpty() && NULL != inputFactory && 0 <= num );
    UIndex::IOSection ioSec;
    fillIOSec( ioSec, url, inputFactory, num );
    if( stateInfo.hasError() ) {
        return;
    }
    ind.ios.append( ioSec );
    
    std::auto_ptr< IOAdapter > io( inputFactory->createIOAdapter() );
    if( !io->open( url, IOAdapterMode_Read ) ) {
        stateInfo.setError(tr( "Can't open file for read: '%1'" ).arg( url ));
        return;
    }
    
    while( !io->isEof() ) {
        QList<FormatDetectionResult> dfList = DocumentUtils::detectFormat( io.get() );
        if( dfList.isEmpty() ) {
            stateInfo.setError(tr( "Can't detect file format: %1").arg(url));
            return;
        }
        DocumentFormat* df = dfList.first().format;
        if( !df->checkFlags(DocumentFormatFlag_SupportStreaming) ) {
            stateInfo.setError(tr( "UGENE can't index files of a specified format: %1" ).arg(df->getFormatName()));
            return;
        }
        
        qint64 startOff = io->bytesRead();
        std::auto_ptr<Document> doc(df->loadDocument(io.get(), stateInfo, QVariantMap(), DocumentLoadMode_SingleObject));
        if (hasError() || isCanceled()) {
            return;
        }
        assert(doc.get()!=NULL);
        qint64 endOff = io->bytesRead();
        foreach(GObject* obj, doc->getObjects()) {
            UIndex::ItemSection item;
            item.ioSectionId = ioSec.sectionId;
            item.docFormat   = df->getFormatId();
            item.startOff = startOff;
            item.endOff   = endOff;
            item.keys.unite(obj->getIndexInfo());
            item.keys.insert(CreateFileIndexTask::KEY_NAME, obj->getGObjectName());
            ind.items.append( item );
        }
    }
}

void CreateFileIndexTask::readInputUrls() {
    int sz = inputUrls.size();
    assert( sz == inputFactories.size()  && sz > 0);
    for(int i = 0; i < sz; ++i ) {
        if( inputUrls[i].isEmpty() ) {
            uiLog.error("Found zero URL during indexing, skipping..");
            continue;
        }
        if( NULL == inputFactories[i] ) {
            uiLog.error("Found zero IO adapter during indexing, skipping..");
            continue;
        }
        readOneUrl( inputUrls[i], inputFactories[i], i );
        if( stateInfo.hasError() || stateInfo.cancelFlag ) {
            return;
        }
    }
}

void CreateFileIndexTask::writeOutputUrl() {
    DocumentFormat* indFormat = AppContext::getDocumentFormatRegistry()->getFormatById( BaseDocumentFormats::INDEX );
    assert( NULL != indFormat );
    GObject* indObj = new UIndexObject( ind, UIndexObject::OBJ_NAME );
    assert( NULL != indObj );
    QList< GObject* > obj_list;
    obj_list.append( indObj );
    Document* doc = new Document( indFormat, outputFactory, outputUrl, obj_list, QVariantMap() );
    indFormat->storeDocument( doc, stateInfo );
    delete doc;
}

void CreateFileIndexTask::run() {
    if( stateInfo.hasError() ) {
        return;
    }
    readInputUrls();
    if( stateInfo.hasError() || stateInfo.cancelFlag ) {
        return;
    }
    stateInfo.progress = 0;
    writeOutputUrl();
}

const QList<QString> & CreateFileIndexTask::getInputUrls() const {
    return inputUrls;
}

const QString & CreateFileIndexTask::getOutputUrl() const {
    return outputUrl;
}

} // U2
