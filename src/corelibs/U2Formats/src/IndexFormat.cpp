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


#include <U2Core/IOAdapter.h>
#include <U2Core/Task.h>
#include <U2Core/L10n.h>
#include <U2Core/TextUtils.h>
#include <QtCore/QStringList>

#include <U2Core/UIndex.h>
#include <U2Core/UIndexObject.h>
#include "IndexFormat.h"

/* TRANSLATOR U2::IndexFormat */

using namespace U2;

const int BUF_SZ       = 1024;
const int SMALL_BUF_SZ = 128;

const char NULL_SYM = '\0';
const int  NO_BYTES = 0;

const QByteArray    QUOTE  = "\"";
const char          CQUOTE = '"';

const QByteArray NEW_LINE = QByteArray( 1, '\n' );
const QByteArray TAB      = QByteArray( 1, '\t' );

const int  BAD_OFFSET = -1;

const int READ_ONE_CHAR = 1;

const QByteArray EOF_SECTION = "//";

const QByteArray ITEM_SEC = "IT";
const QByteArray IO_SEC   = "IO";

enum SectionId {
    UNKNOWN_SEC_ID = -1,
    ITEM_SEC_ID,
    IO_SEC_ID
};

const QByteArray IO_TAG        = "IO";
const QByteArray DOC_TAG       = "DO";
const QByteArray START_OFF_TAG = "SO";
const QByteArray END_OFF_TAG   = "EO";
const QByteArray ID_TAG        = "ID";
const QByteArray ADAPT_TAG     = "AD";
const QByteArray URL_TAG       = "UR";

enum TagId {
    UNKNOWN_TAG_ID = -1,
    IO_TAG_ID,
    DOC_TAG_ID,
    START_OFF_TAG_ID,
    END_OFF_TAG_ID,
    ID_TAG_ID,
    ADAPT_TAG_ID,
    URL_TAG_ID
};

static SectionId getSectionId( const QByteArray& sec_name ) {
    if( ITEM_SEC == sec_name ) {
        return ITEM_SEC_ID;
    }
    else if( IO_SEC == sec_name ) {
        return IO_SEC_ID;
    }
    return UNKNOWN_SEC_ID;
}

static TagId getTagId( const QByteArray& tag ) {
    if( IO_TAG == tag ) {
        return IO_TAG_ID;
    }
    else if( DOC_TAG == tag ) {
        return DOC_TAG_ID;
    }
    else if( START_OFF_TAG == tag ) {
        return START_OFF_TAG_ID;
    }
    else if( END_OFF_TAG == tag ) {
        return END_OFF_TAG_ID;
    }
    else if( ID_TAG == tag ) {
        return ID_TAG_ID;
    }
    else if( ADAPT_TAG == tag ) {
        return ADAPT_TAG_ID;
    }
    else if( URL_TAG == tag ) {
        return URL_TAG_ID;
    }
    return UNKNOWN_TAG_ID;
}

static void checkReadError( int ret ) {
    if( -1 == ret ) {
        throw IndexFormat::ReadError();
    }
}

static QString getErrorString( UIndex::UIndexError err ) {
    switch( err ) {
    case UIndex::EMPTY_IO_ID:
        return IndexFormat::tr( "Empty io identifier in item section" );
    case UIndex::EMPTY_DOC_FORMAT:
        return IndexFormat::tr( "Empty document format in item section " );
    case UIndex::EMPTY_IO_ADAPTER:
        return IndexFormat::tr( "Empty io adapter identifier in io section" );
    case UIndex::EMPTY_URL:
        return IndexFormat::tr( "Empty url in io section" );
    case UIndex::BAD_OFFSET:
        return IndexFormat::tr( "Bad offset in item section" );
    case UIndex::NO_SUCH_IO:
        return IndexFormat::tr( "Unrecognized io section id in item section" );
    default: break;
    }
    return IndexFormat::tr( "Unknown index error occurred" ); // unrecognized error code
}

static void checkWriteError( quint64 wanted_to_write, quint64 how_many_wrote ) {
    if( wanted_to_write != how_many_wrote ) {
        throw IndexFormat::WriteError();
    }
}

static bool checkHeader( const char* data, int sz ) {
    assert( NULL != data && 0 <= sz );
    
    if( IndexFormat::FILE_HEADER.size() > sz ) {
        return false;
    }
    QByteArray header = QByteArray().fromRawData( data, sz ).trimmed();
    return header.startsWith( IndexFormat::FILE_HEADER );
}

static void readLine( IOAdapter* io, QByteArray* to = NULL ) {
    assert( NULL != io );
    
    QByteArray buf( BUF_SZ, NULL_SYM );
    bool there = false;
    while ( !there ) {
        int ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include, &there );
        checkReadError( ret );
        if( NO_BYTES == ret ) {
            break;
        }
        if( NULL != to ) {
            to->append( QByteArray().fromRawData( buf.data(), ret ) );
        }
    }
}

static void skipLinesAndWhites( IOAdapter* io ) {
    assert( NULL != io );
    bool is_line_or_white = true;
    char c = 0;
    
    while( is_line_or_white ) {
        int ret = io->readBlock( &c, READ_ONE_CHAR );
        checkReadError( ret );
        if( NO_BYTES == ret ) {
            return;
        }
        is_line_or_white = TextUtils::LINE_BREAKS[uchar(c)] || TextUtils::WHITES[uchar(c)];
    }
    io->skip( -READ_ONE_CHAR );
}


static bool isEof( IOAdapter* io ) {
    assert( NULL != io );
    
    skipLinesAndWhites( io );
    QByteArray buf( SMALL_BUF_SZ, NULL_SYM );
    int ret = io->readBlock( buf.data(), SMALL_BUF_SZ );
    checkReadError( ret );
    io->skip( -ret );
    return NO_BYTES == ret;
}

static bool isEofSection( IOAdapter* io ) {
    assert( NULL != io );
    skipLinesAndWhites( io );

    QByteArray line( SMALL_BUF_SZ, NULL_SYM );
    int ret = io->readUntil( line.data(), SMALL_BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include );
    checkReadError( ret );
    io->skip( -ret );
    return EOF_SECTION == QByteArray( line.data(), ret ).trimmed();
}

static void readEofSection( IOAdapter* io ) {
    assert( NULL != io );
    if ( !isEofSection( io ) ) {
        return;
    }
    readLine( io );
    skipLinesAndWhites( io );
}

// if odd quote sequence found then end = true
static QByteArray unQuoteWord( const QByteArray& word, bool* end, int* ind = NULL ) {
    assert( NULL != end );
    int sz = word.size();
    assert( sz );
    QByteArray ret;
    int i = 0;
    
    for( i = 0; i < sz; ++i ) {
        if( CQUOTE != word[i] ) {
            ret.append( word[i] );
        }
        else {
            if( sz - 1 != i && CQUOTE == word[i+1] ) {
                ret.append( word[i] );
                i++;
                continue;
            }
            *end = true;
            if( NULL != ind ) {
                *ind = i;
            }
            break;
        }
    }
    return ret;
}

static void getTagValue( QByteArray& tag, QByteArray& val, const QByteArray& ln ) {
    QByteArray line = ln.trimmed();
    assert( line.size() );
    if( CQUOTE != line[0] ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "Bad value in item section: tag" ) );
    }
    bool end = false;
    int tagEnd = -1;
    tag = unQuoteWord( line.mid( 1 ), &end, &tagEnd );
    if( !end ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "Bad value in item section: tag" ) );
    }
    assert( -1 != tagEnd );
    QByteArray vLine = line.mid( tagEnd + 2 ).trimmed();
    assert( vLine.size() );
    if( CQUOTE != vLine[0] ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "Bad value in item section: val" ) );
    }
    val = unQuoteWord( vLine.mid( 1 ), &end );
    if( !end ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "Bad value in item section: val" ) );
    }
}

static void readIOSection( IOAdapter* io, UIndex::IOSection& io_sec ) {
    assert( NULL != io );
    
    while( !isEofSection( io ) ) {
        QByteArray line;
        readLine( io, &line );
        QByteArray tag;
        QByteArray val;
        getTagValue( tag, val, line );
        TagId tag_id   = getTagId( tag );
        
        switch( tag_id ) {
        case ID_TAG_ID:
            io_sec.sectionId = QString( val );
            break;
        case ADAPT_TAG_ID:
            io_sec.ioAdapterId = QString( val );
            break;
        case URL_TAG_ID:
            io_sec.url = QString( val );
            break;
        default:
            io_sec.keys[QString( tag )] = QString( val );
        }
    }
    readEofSection( io );
}

static void readItemSection( IOAdapter* io, UIndex::ItemSection& item ) {
    assert( NULL != io );
    
    while( !isEofSection( io ) ) {
        QByteArray line;
        readLine( io, &line );
        QByteArray tag;
        QByteArray val;
        getTagValue( tag, val, line );
        TagId tag_id   = getTagId( tag );
        
        switch( tag_id ) {
        case IO_TAG_ID:
            item.ioSectionId = QString( val );
            break;
        case DOC_TAG_ID:
            item.docFormat = QString( val );
            break;
        case START_OFF_TAG_ID:
            {
                bool ok = false;
                qint64 converted = val.toLongLong( &ok );
                item.startOff = ( ok )? converted: BAD_OFFSET;
            }
            break;
        case END_OFF_TAG_ID:
            {
                bool ok = false;
                qint64 converted = val.toLongLong( &ok );
                item.endOff = ( ok )? converted: BAD_OFFSET;
            }
            break;
        default:
            item.keys[QString( tag )] = QString( val );
        }
    } // !EofSection
    readEofSection( io );
}

static void load( IOAdapter* io, QList< GObject* >& obj_list, TaskStateInfo& tsi ) {
    assert( NULL != io );
    
    QByteArray line;
    skipLinesAndWhites( io );
    readLine( io, &line );
    if( !checkHeader( line.data(), line.size() ) ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "invalid header" ) );
    }
    UIndex ind;
    while( !isEof( io ) ) {
        if( tsi.cancelFlag ) {
            return;
        }
        skipLinesAndWhites( io );
        line.clear();
        readLine( io, &line );
        QByteArray section_name = line.trimmed();
        SectionId sec_id = getSectionId( section_name );
        
        switch( sec_id ) {
        case ITEM_SEC_ID:
            {
                UIndex::ItemSection item;
                UIndex::UIndexError err = UIndex::NO_ERR;
                
                readItemSection( io, item );
                err = item.checkConsistentcy();
                if( UIndex::NO_ERR == err ) {
                    ind.items.push_back( item );
                }
                else { // has errors
                    throw IndexFormat::BadDataError( getErrorString( err ) );
                }
            }
            break;
        case IO_SEC_ID:
            {
                UIndex::IOSection   io_sec;
                UIndex::UIndexError err = UIndex::NO_ERR;
                
                readIOSection( io, io_sec );
                err = io_sec.checkConsistentcy();
                if( UIndex::NO_ERR == err ) {
                    ind.ios.push_back( io_sec );
                }
                else {
                    throw IndexFormat::BadDataError( getErrorString( err ) );
                }
            }
            break;
        default:
            throw IndexFormat::BadDataError( IndexFormat::tr( "Unknown section found" ) );
        }
        tsi.progress = io->getProgress();
    }
    UIndex::UIndexError err = ind.checkConsistency();
    if( UIndex::NO_ERR != err ) {
        throw IndexFormat::BadDataError( getErrorString( err ) );
    }
    if( !ind.hasItems() ) {
        throw IndexFormat::BadDataError( IndexFormat::tr( "no items found in index file" ) );
    }
    
    UIndexObject* obj = new UIndexObject( ind, UIndexObject::OBJ_NAME );
    obj_list.append( obj );
}

static void writeOneWord( IOAdapter* io, const QByteArray& word ) {
    assert( NULL != io );
    quint64 ret = 0;
    ret = io->writeBlock( word );
    checkWriteError( word.size(), ret );
}

static QByteArray quotedWord( const QByteArray& str ) {
    QByteArray word;
    int sz = str.size();
    int i = 0;
    for( i = 0; i < sz; ++i ) {
        if( QUOTE[0] == str[i] ) {
            word.push_back( QUOTE );
        }
        word.push_back( str[i] );
    }
    return QUOTE + word + QUOTE;
}

static void writeTagValue( IOAdapter* io, const QByteArray& tag, const QByteArray& val ) {
    assert( NULL != io );
    writeOneWord( io, quotedWord( tag ) );
    writeOneWord( io, TAB );
    writeOneWord( io, quotedWord( val ) );
    writeOneWord( io, NEW_LINE );
}

static void save( IOAdapter* io, const UIndex& ind ) {
    assert( NULL != io );
    QList< UIndex::ItemSection > items = ind.items;
    QList< UIndex::IOSection >   ios   = ind.ios;
    
    //write header
    writeOneWord( io, IndexFormat::FILE_HEADER );
    writeOneWord( io, NEW_LINE );
    //write io sections
    foreach( UIndex::IOSection io_sec, ios ) {
        writeOneWord( io, IO_SEC + NEW_LINE );
        writeTagValue( io, ID_TAG, io_sec.sectionId.toAscii() );
        writeTagValue( io, ADAPT_TAG, io_sec.ioAdapterId.toAscii() );
        writeTagValue( io, URL_TAG, io_sec.url.toAscii() );
        
        QHash< QString, QString >::const_iterator it = io_sec.keys.begin();
        while( io_sec.keys.end() != it ) {
            writeTagValue( io, it.key().toAscii(), it.value().toAscii() );
            ++it;
        }
        writeOneWord( io, EOF_SECTION + NEW_LINE );
    }
    //write items sections
    foreach( UIndex::ItemSection item, items ) {
        writeOneWord( io, ITEM_SEC + NEW_LINE );
        writeTagValue( io, IO_TAG, item.ioSectionId.toAscii() );
        writeTagValue( io, DOC_TAG, item.docFormat.toAscii() );
        writeTagValue( io, START_OFF_TAG, QString::number( item.startOff ).toAscii() );
        writeTagValue( io, END_OFF_TAG, QString::number( item.endOff ).toAscii() );
        
        QHash< QString, QString >::const_iterator it = item.keys.begin();
        while( item.keys.end() != it ) {
            writeTagValue( io, it.key().toAscii(), it.value().toAscii() );
            ++it;
        }
        writeOneWord( io, EOF_SECTION + NEW_LINE );
    }
}

namespace U2 {

const QByteArray IndexFormat::FILE_HEADER       = "UINDEX";
const QString IndexFormat::WRITE_LOCK_REASON    = "Index files are read only";

IndexFormat::IndexFormat( QObject* obj ) : DocumentFormat( obj, DocumentFormatFlag_SupportWriting, QStringList("uind")) {
    format_name = tr("Index");
    supportedObjectTypes+=GObjectTypes::UINDEX;
}


Document* IndexFormat::loadDocument( IOAdapter* io, TaskStateInfo& tsi, const QVariantMap& fs, DocumentLoadMode mode) {
    Q_UNUSED(mode);

    QList< GObject* > obj_list;
    try {
        load( io, obj_list, tsi );
        assert( OBJECTS_IN_DOC == obj_list.size() || tsi.cancelFlag );
        
        Document* doc = new Document( this, io->getFactory(), io->getURL(), obj_list, fs, WRITE_LOCK_REASON );
        return doc;
    }
    catch ( const IndexFormatException& ex ) {
        tsi.setError(ex.msg);
    }
    catch( ... ) {
        tsi.setError(IndexFormat::tr( "Unknown error occurred" ));
    }
    qDeleteAll( obj_list );
    return NULL;
}

void IndexFormat::storeDocument( Document* doc, TaskStateInfo& ts, IOAdapter* io) {
    try {
        foreach( GObject* p_obj, doc->getObjects() ) {
            const UIndexObject* ind_obj = qobject_cast< const UIndexObject* >( p_obj );
            assert( NULL != ind_obj );
            save( io, ind_obj->getIndex() );
        }
    }
    catch( const IndexFormatException& ex ) {
        ts.setError(ex.msg);
    }
    catch(...) {
        ts.setError(IndexFormat::tr( "Unknown error occurred" ));
    }
}

FormatDetectionResult IndexFormat::checkRawData(const QByteArray& data, const GUrl&) const {
    bool headerIsOk = checkHeader( data.constData(), data.size());
    return headerIsOk ? FormatDetection_Matched: FormatDetection_NotMatched;
}

bool IndexFormat::isObjectOpSupported(const Document* doc, DocumentFormat::DocObjectOp op, GObjectType t) const {
    if (!DocumentFormat::isObjectOpSupported(doc, op, t)) {
        return false;
    }
    if (op == DocumentFormat::DocObjectOp_Add) {
        return doc->getObjects().isEmpty();
    }
    return false;
}


} // U2
