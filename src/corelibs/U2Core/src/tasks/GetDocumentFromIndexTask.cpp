
#include <U2Core/AppContext.h>

#include <U2Core/IOAdapter.h>
#include <U2Core/LocalFileAdapter.h>
#include <U2Core/ZlibAdapter.h>

#include "GetDocumentFromIndexTask.h"

namespace U2 {

const QString ACCESS_POINTS_SZ_TAG = "APS";
const QString ACCESS_POINT_TAG     = "AP";
const QString ACCESS_POINT_WND_TAG = "APW";
const QString SPACE                = " ";

const int ACCESS_POINTS_NUMS_AMOUNT = 3;


bool GetDocumentFromIndexTask::fillAccessPointNums( GZipIndexAccessPoint& point, const QString& numStr ) {
    QStringList nums = numStr.split( SPACE, QString::SkipEmptyParts );
    if( ACCESS_POINTS_NUMS_AMOUNT != nums.size() ) {
        return false;
    }
    bool ok = false;
    
    point.bits = nums[0].toInt( &ok );
    if( !ok || 0 > point.bits ) {
        return false;
    }
    point.in = nums[1].toLongLong( &ok );
    if( !ok ) {
        return false;
    }
    point.out = nums[2].toLongLong( &ok );
    if( !ok ) {
        return false;
    }
    return true;
}

bool GetDocumentFromIndexTask::getGzipIndexAccessPoint( GZipIndexAccessPoint& ret, const UIndex::IOSection& ioSec, qint64 offset ) {
    assert( 0 <= offset );
    
    if( !ioSec.keys.contains( ACCESS_POINTS_SZ_TAG ) ) {
        return false;
    }
    bool ok = false;
    int indSz = ioSec.keys[ACCESS_POINTS_SZ_TAG].toInt( &ok );
    if( !ok || 0 >= indSz ) {
        return false;
    }
    
    GZipIndexAccessPoint next;
    ok = fillAccessPointNums( next, ioSec.keys.value( ACCESS_POINT_TAG + QString::number( 0 ) ) );
    if( !ok ) {
        return false;
    }
    int i = 0;
    for( i = 0; i < indSz; ++i ) {
        ret = next;
        if( indSz - 1 == i ) {
            break;
        }
        else {
            ok = fillAccessPointNums( next, ioSec.keys.value( ACCESS_POINT_TAG + QString::number( i + 1 ) ) );
            if( !ok ) {
                return false;
            }
            if( next.out > offset ) {
                break;
            }
        }
    }
    QString wndTag = ACCESS_POINT_WND_TAG + QString::number( i );
    if( !ioSec.keys.contains( wndTag ) ) {
        return false;
    }
    QByteArray wnd = QByteArray::fromBase64( ioSec.keys.value( wndTag ).toAscii() );
    ret.window = qUncompress( wnd );
    assert( GZipIndex::WINSIZE == ret.window.size() );
    return true;
}

IOAdapter* GetDocumentFromIndexTask::getOpenedIOAdapter(const UIndex::ItemSection& itemSec, const UIndex::IOSection& ioSec) {
    IOAdapterId adId = ioSec.ioAdapterId;
    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( adId );
    
    if( NULL == factory ) {
        setError(tr( "Can't find IO adapter: %1" ).arg( adId ));
        return NULL;
    }
    IOAdapter* ret = factory->createIOAdapter();
    bool ok = ret->open( ioSec.url, IOAdapterMode_Read );
    if( !ok ) {
        delete ret;
        setError(tr( "Can't open file for read: '%1'" ).arg(ioSec.url));
        return NULL;
    }
    
    if( BaseIOAdapters::LOCAL_FILE == adId ) {
        bool ok = ret->skip( itemSec.startOff );
        if( !ok ) {
            delete ret;
            setError(tr( "Error positioning in indexed file" ));
            return NULL;
        }
    }
    else if( BaseIOAdapters::GZIPPED_LOCAL_FILE == adId ) {
        ZlibAdapter* gzAdapter = qobject_cast< ZlibAdapter* >( ret );
        assert( NULL != gzAdapter );
        
        GZipIndexAccessPoint accessPoint;
        bool ok = getGzipIndexAccessPoint( accessPoint, ioSec, itemSec.startOff );
        if( !ok ) {
            setError(tr( "GZIP index is corrupted" ));
            delete ret;
            return NULL;
        }
        ok = gzAdapter->skip( accessPoint, itemSec.startOff );
        if( !ok ) {
            setError(tr( "Error positioning in indexed file" ));
            delete ret;
            return NULL;
        }
    }
    else { // others not supported
        return NULL;
    }
    return ret;
}

GetDocumentFromIndexTask::GetDocumentFromIndexTask( const UIndex& ind, int num ) 
: Task( tr( "Get document from index" ), TaskFlag_None ), index( ind ), docNum( num ), doc( NULL ) {
    tpm = Progress_Manual;
    if( !index.hasItems() ) {
        stateInfo.setError(tr( "Index is empty" ));
        return;
    }
    if( 0 > docNum || docNum >= index.items.size() ) {
        stateInfo.setError(tr( "Invalid document number: %1, max: %2").arg(docNum).arg(index.items.size()));
        return;
    }
}

GetDocumentFromIndexTask::~GetDocumentFromIndexTask() {
    cleanup();
}

void GetDocumentFromIndexTask::run() {
    if( stateInfo.hasErrors() ) {
        return;
    }
    UIndex::ItemSection itemSec = index.items[docNum];
    UIndex::IOSection   ioSec = index.getIOSection( itemSec.ioSectionId );
    if( ioSec.sectionId.isEmpty() ) {
        stateInfo.setError(tr( "Index is corrupted" ));
        return;
    }
    
    IOAdapter* ioAdapt = getOpenedIOAdapter( itemSec, ioSec);
    if( NULL == ioAdapt ) {
        assert( stateInfo.hasErrors() );
        return;
    }
    assert( ioAdapt->isOpen() );
    DocumentFormat* df = AppContext::getDocumentFormatRegistry()->getFormatById( itemSec.docFormat );
    if( NULL == df ) {
        delete ioAdapt;
        stateInfo.setError(tr( "Unknown document format: %1" ).arg(itemSec.docFormat));
        return;
    }
    
    doc = df->loadDocument( ioAdapt, stateInfo, QVariantMap(), DocumentLoadMode_SingleObject );
    assert( isCanceled() || NULL != doc || hasErrors() );
    assert( NULL == doc || doc->isLoaded() );
    delete ioAdapt;
}

Task::ReportResult GetDocumentFromIndexTask::report() {
    if ( stateInfo.hasErrors() || isCanceled() ) {
        return ReportResult_Finished;
    }
    assert( NULL != doc );
    return ReportResult_Finished;
}

void GetDocumentFromIndexTask::cleanup() {
    if( NULL != doc ) {
        delete doc;
        doc = NULL;
    }
}

Document* GetDocumentFromIndexTask::getDocument() const {
    return doc;
}

Document* GetDocumentFromIndexTask::takeDocument() {
    Document* ret = doc;
    doc = NULL;
    return ret;
}

} // U2
