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

#include "StockholmFormat.h"

#include "DocumentFormatUtils.h"

#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/L10n.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>

#include <U2Core/MAlignmentInfo.h>

#include <U2Core/TextUtils.h>

#include <QtCore/QFileInfo>

/* TRANSLATOR U2::StockholmFormat */
namespace U2 {
const QByteArray StockholmFormat::FILE_ANNOTATION_ID = "#=GF ID";
const QByteArray StockholmFormat::FILE_ANNOTATION_AC = "#=GF AC";
const QByteArray StockholmFormat::FILE_ANNOTATION_DE = "#=GF DE";
const QByteArray StockholmFormat::FILE_ANNOTATION_GA = "#=GF GA";
const QByteArray StockholmFormat::FILE_ANNOTATION_NC = "#=GF NC";
const QByteArray StockholmFormat::FILE_ANNOTATION_TC = "#=GF TC";

const QByteArray StockholmFormat::UNI_ANNOTATION_MARK = "# UNIMARK";

const QByteArray StockholmFormat::COLUMN_ANNOTATION_SS_CONS = "#=GC SS_cons";
const QByteArray StockholmFormat::COLUMN_ANNOTATION_RF      = "#=GC RF";

StockholmFormat::ReadError::ReadError(const GUrl& url) : StockholmBaseException( L10N::errorReadingFile(url) ){}

StockholmFormat::WriteError::WriteError(const GUrl& url) : StockholmBaseException( L10N::errorWritingFile(url) ){}
} // U2

const int  BUF_SZ = 1024;
const int  SMALL_BUF_SZ = 128;
const char TERM_SYM = '\0';
const int  NO_BYTES = 0;
const char NEW_LINE = '\n';

const char* HEADER = "# STOCKHOLM 1.0\n\n";
const char* HEADER_MIN = "# STOCKHOLM 1.";
const int HEADER_SZ_MIN = 15;
const char* EOF_STR = "//";

const char COMMENT_OR_MARKUP_LINE = '#';
const char* EMPTY_STR = "";

const int WRITE_BLOCK_LENGTH = 50;

using namespace U2;

//other not supported
enum AnnotationTag {
    NO_TAG = -1,
    ID,
    AC,
    DE,
    SS_CONS,
    RF,
    GA,
    NC,
    TC
};
//other types not supported
enum AnnotationType {
    FILE_ANNOTATION,
    COLUMN_ANNOTATION,
    UNI_ANNOTATION
};

struct Annotation {
    AnnotationType type;
    AnnotationTag tag;
    QString val;
    Annotation( AnnotationType ty, AnnotationTag t, QString v ) { type = ty, tag = t; val = v; }
    virtual ~Annotation(){}
};
//#=GF annotations
struct FileAnnotation : public Annotation {
    FileAnnotation( AnnotationTag t, QString v ): Annotation( FILE_ANNOTATION, t, v ){}
};
//unipro ugene annotations
struct UniAnnotation : public Annotation {
    UniAnnotation( AnnotationTag t, QString v ): Annotation( UNI_ANNOTATION, t, v ){}
};
//#=GC annotations
struct ColumnAnnotation : public Annotation {
    ColumnAnnotation( AnnotationTag t, QString v ): Annotation( COLUMN_ANNOTATION, t, v ){}
};

static Annotation* findAnnotation( const QList< Annotation* >& annList, AnnotationType t, AnnotationTag tag );

//you should put annotations here after creation
struct AnnotationBank {
    QList<Annotation*> ann_list;
    
    void addAnnotation( Annotation* ann ) {
        if( NULL == ann ) {
            return;
        }
        if( COLUMN_ANNOTATION == ann->type ) { /* Column annotations usually written as blocks with seqs */
            assert( SS_CONS == ann->tag || RF == ann->tag );
            Annotation* nAnn = findAnnotation( ann_list, COLUMN_ANNOTATION, ann->tag );
            if( NULL != nAnn ) {
                nAnn->val.append( ann->val );
                delete ann;
                return;
            }
        }
        ann_list.append( ann );
    }
    ~AnnotationBank() {
        qDeleteAll( ann_list );
    }
}; // AnnotationBank

static Annotation* findAnnotation( const QList< Annotation* >& annList, AnnotationType t, AnnotationTag tag ) {
    Annotation* ret = NULL;
    foreach( Annotation* a, annList ) {
        assert( NULL != a );
        if( a->type == t && a->tag == tag ) {
            ret = a;
            break;
        }
    }
    return ret;
}

static Annotation* getAnnotation( const QByteArray& l ) {
    QByteArray line = l.trimmed();

    if ( line.startsWith( StockholmFormat::FILE_ANNOTATION_ID ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_ID.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( ID , val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::FILE_ANNOTATION_AC ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_AC.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( AC, val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::FILE_ANNOTATION_DE ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_DE.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( DE, val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::FILE_ANNOTATION_GA ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_GA.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( GA, val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::FILE_ANNOTATION_NC ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_NC.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( NC, val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::FILE_ANNOTATION_TC ) ) {
        QByteArray val = line.mid( StockholmFormat::FILE_ANNOTATION_TC.size() ).trimmed();
        return ( val.size() )? new FileAnnotation( TC, val ) : NULL;
    }
    else if ( StockholmFormat::UNI_ANNOTATION_MARK == line ) {
        return new UniAnnotation( NO_TAG, line );
    }
    else if( line.startsWith( StockholmFormat::COLUMN_ANNOTATION_SS_CONS ) ) {
        QByteArray val = line.mid( StockholmFormat::COLUMN_ANNOTATION_SS_CONS.size() ).trimmed();
        return ( val.size() )? new ColumnAnnotation( SS_CONS, val ) : NULL;
    }
    else if( line.startsWith( StockholmFormat::COLUMN_ANNOTATION_RF ) ) {
        QByteArray val = line.mid( StockholmFormat::COLUMN_ANNOTATION_RF.size() ).trimmed();
        return ( val.size() )? new ColumnAnnotation( RF, val ) : NULL;
    }
    return NULL;
}

static QString getMsaName( const AnnotationBank& ann_bank ) {
    foreach( Annotation* ann, ann_bank.ann_list ) {
        assert( NULL != ann );
        if ( FILE_ANNOTATION == ann->type && ID == ann->tag ) {
            return ann->val;
        }
    }
    return QString::null;
}

static bool isUniFile( const AnnotationBank& ann_bank ) {
    foreach( Annotation* ann, ann_bank.ann_list ) {
        assert( NULL != ann );
        if ( UNI_ANNOTATION == ann->type && StockholmFormat::UNI_ANNOTATION_MARK == ann->val ) {
            return true;
        }
    }
    return false;
}

static QString getAnnotationName( Annotation* ann ) {
    assert( NULL != ann );
    
    AnnotationType t = ann->type;
    switch( t ) {
    case UNI_ANNOTATION:
        return QString( StockholmFormat::UNI_ANNOTATION_MARK );
    case FILE_ANNOTATION:
        {
            AnnotationTag tag = ann->tag;
            switch( tag ) {
            case ID:
                return QString( StockholmFormat::FILE_ANNOTATION_ID );
            case AC:
                return QString( StockholmFormat::FILE_ANNOTATION_AC );
            case DE:
                return QString( StockholmFormat::FILE_ANNOTATION_DE );
            case GA:
                return QString( StockholmFormat::FILE_ANNOTATION_GA );
            case NC:
                return QString( StockholmFormat::FILE_ANNOTATION_NC );
            case TC:
                return QString( StockholmFormat::FILE_ANNOTATION_TC );
            default:
                assert( false );
            }
        }
    case COLUMN_ANNOTATION:
        {
            AnnotationTag tag = ann->tag;
            switch( tag ) {
            case SS_CONS:
                return QString( StockholmFormat::COLUMN_ANNOTATION_SS_CONS );
            case RF:
                return QString( StockholmFormat::COLUMN_ANNOTATION_RF );
            default:
                assert( false );
            }
        }
    default:
        assert( false );
    }
    return QString();
}

static QHash< QString, QString > getAnnotationMap( const AnnotationBank& annBank ) {
    QHash< QString, QString > ret;
    foreach( Annotation* ann, annBank.ann_list ) {
        assert( NULL != ann );
        QString annName = getAnnotationName( ann );
        ret[annName] = QString( ann->val );
    }
    return ret;
}

template<class T>
static void checkValThrowException( bool expected, T val1, T val2,  const StockholmFormat::StockholmBaseException& e ) {
    if ( expected != ( val1 == val2 ) ) {
        throw e;
    }
}

static bool checkHeader( const char* data, int sz ) {
    assert( NULL != data && 0 <= sz );

    if ( HEADER_SZ_MIN > sz ) {
        return false;
    }
    return QByteArray( data, sz ).startsWith( HEADER_MIN );
}

//returns true if the line was skipped
static bool skipCommentOrMarkup( IOAdapter* io, AnnotationBank& ann_bank ) {
    assert( NULL != io );

    QByteArray buf( BUF_SZ, TERM_SYM );
    bool term_there = false;
    int ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Exclude, &term_there );

    checkValThrowException<int>( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
    if ( COMMENT_OR_MARKUP_LINE == buf[0] ) {
        QByteArray line;
        line.append( QByteArray( buf.data(), ret ) );
        while ( !term_there ) {
            ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Exclude, &term_there );
            checkValThrowException<int>( false, -1, ret,StockholmFormat::ReadError(io->getURL()) );
            if ( NO_BYTES == ret ) {
                break;
            }
            line.append( QByteArray( buf.data(), ret ) );
        }
        ann_bank.addAnnotation( getAnnotation( line ) );
        return true;
    }
    io->skip( -ret );
    return false;
}

static void skipBlankLines( IOAdapter* io, QByteArray* lines = NULL ) {
    assert( NULL != io );

    char c = 0;
    bool work = true;
    while ( work ) {
        int ret = io->readBlock( &c, 1 );
        checkValThrowException<int>( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
        if ( NO_BYTES == ret ) {
            return;
        }
        work = TextUtils::LINE_BREAKS[(uchar)c] || TextUtils::WHITES[(uchar)c];
        if ( lines && TextUtils::LINE_BREAKS[(uchar)c] ) {
            lines->append( c );
        }
    }
    io->skip( -1 );
}

//skips all that it can
static void skipMany( IOAdapter* io, AnnotationBank& ann_bank ) {
    assert( NULL != io );

    char c = 0;
    while ( 1 ) {
        bool ret = io->getChar( &c );
        checkValThrowException<bool>( false, false, ret, StockholmFormat::ReadError(io->getURL()) );
        if ( COMMENT_OR_MARKUP_LINE == c ) {
            io->skip( -1 );
            skipCommentOrMarkup( io, ann_bank );
            continue;
        }
        else if ( TextUtils::LINE_BREAKS[(uchar)c] || TextUtils::WHITES[(uchar)c] ) {
            skipBlankLines( io );
            continue;
        }
        io->skip( -1 );
        break;
    }
}

static bool eofMsa( IOAdapter* io ) {
    assert( NULL != io );
    
    QByteArray buf( SMALL_BUF_SZ, TERM_SYM );
    int ret = io->readUntil( buf.data(), SMALL_BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include );
    checkValThrowException( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
    io->skip( -ret );
    return EOF_STR == QByteArray( buf.data(), ret ).trimmed();
}

static void readEofMsa( IOAdapter* io ) {
    assert( eofMsa( io ) );
    QByteArray buf( SMALL_BUF_SZ, TERM_SYM );
    int ret = io->readUntil( buf.data(), SMALL_BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Include );
    checkValThrowException( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
}

//returns end of sequence name in line
static int getLine( IOAdapter* io, QByteArray& to ) {
    assert( NULL != io );

    QByteArray buf( BUF_SZ, TERM_SYM );
    bool there = false;

    while ( !there ) {
        int ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Exclude, &there );
        checkValThrowException<int>( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
        if( NO_BYTES == ret ) {
            break;
        }
        to.append( QByteArray( buf.data(), ret ) );
    }

    int sz = to.size();
    int i = 0;
    for ( i = 0; i < sz; ++i ) {
        if ( TextUtils::WHITES[(uchar)to[i]] ) {
            break;
        }
    }
    return i;
}

static bool blockEnded( IOAdapter* io ) {
    assert( NULL != io );

    QByteArray lines;
    skipBlankLines( io, &lines );
    if ( eofMsa( io ) ) {
        return true;
    }
    int nl_count = 0;
    int lines_sz = lines.size();

    for( int i = 0; i < lines_sz; ++i ) {
        nl_count = ( NEW_LINE == lines[i] )? nl_count + 1: nl_count;
    }
    return 1 < nl_count;
}

//calls after reading a block
static bool checkSeqLength( const MAlignment& msa ) {
    int sz = msa.getNumRows();
    int seq_length = (sz > 0)? msa.getRow(0).getCoreLength(): -1;
    bool ret = ( sz )? true: false;

    for ( int i = 0; i < sz; ++i ) {
        const MAlignmentRow & row = msa.getRow(i);
        int rowLength = row.getCoreLength();
        if ( !( ret = ret && ( seq_length == rowLength ) ) ) { break; }
    }
    return ret;
}

static bool nameWasBefore( const MAlignment& msa, const QString& name ) {
    int n = msa.getNumRows();
    bool ret = false;

    for( int i = 0; i < n; ++i ) {
        if ( name == msa.getRow(i).getName()) {
            ret = true;
            break;
        }
    }
    return ret;
}

static void changeGaps( QByteArray& seq ) {
    int seq_sz = seq.size();
    for( int i = 0; i < seq_sz; ++i ) {
        if ( '.' == seq[i] ) {
            seq[i] = '-';
        }
    }
}

// returns true if operation was not canceled
static bool loadOneMsa( IOAdapter* io, TaskStateInfo& tsi, MAlignment& msa, AnnotationBank& ann_bank ) {
    assert( NULL != io );

    QByteArray buf( BUF_SZ, TERM_SYM );
    int ret = 0;

    //skip header
    skipBlankLines( io );
    ret = io->readUntil( buf.data(), BUF_SZ, TextUtils::LINE_BREAKS, IOAdapter::Term_Exclude );
    checkValThrowException<int>( false, -1, ret, StockholmFormat::ReadError(io->getURL()) );
    if ( !checkHeader( buf.data(), ret ) ) {
        throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: bad header line" ) );
    }
    //read blocks
    bool firstBlock = true;
    while ( 1 ) {
        if( tsi.cancelFlag ) {
            return false;
        }
        skipMany( io, ann_bank );
        if ( eofMsa( io ) ) {
            break;
        }

        bool hasSeqs = true;
        int seq_ind = 0;
        while ( hasSeqs ) {
            QByteArray line;
            int name_end = getLine( io, line );
            QByteArray name = line.left( name_end );
            QByteArray seq  = line.mid( name_end + 1 ).trimmed();

            if ( name.startsWith( COMMENT_OR_MARKUP_LINE ) ) {
                ann_bank.addAnnotation( getAnnotation( line ) );
                hasSeqs = !blockEnded( io );
                tsi.progress = io->getProgress();
                continue;
            }
            changeGaps( seq );
            if ( firstBlock ) {
                if ( EMPTY_STR == name ) {
                    throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: empty sequence name" ) );
                }
                if ( nameWasBefore( msa, QString( name.data() ) ) ) {
                    throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: equal sequence names in one block" ) );
                }
                MAlignmentRow row(name.data(), seq);
                msa.addRow(row);
            }
            else {
                const MAlignmentRow& row = msa.getRow(seq_ind);
                if( name != row.getName()) {
                    throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: sequence names are not equal in blocks" ) );
                }
                msa.appendChars(seq_ind, seq.constData(), seq.size());
            }
            seq_ind++;
            hasSeqs = !blockEnded( io );
            tsi.progress = io->getProgress();
        }
        firstBlock = false;
        //check sequence length after every block
        if ( !checkSeqLength( msa ) ) {
            throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: sequences in block are not of equal size" ) );
        }
    }// while( 1 )
    readEofMsa( io );
    skipBlankLines( io );
    
    if ( msa.getNumRows() == 0 ) {
        throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: empty sequence alignment" ) );
    }
    DocumentFormatUtils::assignAlphabet( msa );
    if ( msa.getAlphabet() == NULL) {
        throw StockholmFormat::BadFileData( StockholmFormat::tr( "invalid file: unknown alphabet" ) );
    }
    return true;
}

static void setMsaInfoCutoffs( QVariantMap& info, const QString& string, MAlignmentInfo::Cutoffs cof1, 
                               MAlignmentInfo::Cutoffs cof2 ) {
    QByteArray str = string.toAscii();
    QTextStream txtStream( str );
    float val1 = .0f;
    float val2 = .0f;
    txtStream >> val1 >> val2;
    MAlignmentInfo::setCutoff( info, cof1, val1 );
    MAlignmentInfo::setCutoff( info, cof2, val2 );
}

static void setMsaInfo( const QHash< QString, QString>& annMap, MAlignment& ma ) {
    QVariantMap info = ma.getInfo();
    
    if (annMap.contains( StockholmFormat::FILE_ANNOTATION_AC ) ) {
        MAlignmentInfo::setAccession( info, annMap[StockholmFormat::FILE_ANNOTATION_AC] );
    }
    if (annMap.contains( StockholmFormat::FILE_ANNOTATION_DE ) ) {
        MAlignmentInfo::setDescription( info, annMap[StockholmFormat::FILE_ANNOTATION_DE] );
    }
    if (annMap.contains( StockholmFormat::COLUMN_ANNOTATION_SS_CONS ) ) {
        MAlignmentInfo::setSSConsensus( info, annMap[StockholmFormat::COLUMN_ANNOTATION_SS_CONS] );
    }
    if (annMap.contains( StockholmFormat::COLUMN_ANNOTATION_RF ) ) {
        MAlignmentInfo::setReferenceLine( info, annMap[StockholmFormat::COLUMN_ANNOTATION_RF] );
    }
    if (annMap.contains( StockholmFormat::FILE_ANNOTATION_GA ) ) {
        setMsaInfoCutoffs( info, annMap[StockholmFormat::FILE_ANNOTATION_GA], MAlignmentInfo::CUTOFF_GA1,
                                                                              MAlignmentInfo::CUTOFF_GA2 );
    }
    if (annMap.contains( StockholmFormat::FILE_ANNOTATION_NC ) ) {
        setMsaInfoCutoffs( info, annMap[StockholmFormat::FILE_ANNOTATION_NC], MAlignmentInfo::CUTOFF_NC1,
                                                                              MAlignmentInfo::CUTOFF_NC2 );
    }
    if (annMap.contains( StockholmFormat::FILE_ANNOTATION_TC ) ) {
        setMsaInfoCutoffs( info, annMap[StockholmFormat::FILE_ANNOTATION_TC], MAlignmentInfo::CUTOFF_TC1,
                                                                              MAlignmentInfo::CUTOFF_TC2 );
    }
    ma.setInfo(info);
}

static void load( IOAdapter* io, QList<GObject*>& l, TaskStateInfo& tsi, bool& uni_file, bool onlyOne ) {
    assert( NULL != io );

    QStringList names_list;
    QString filename = io->getURL().baseFileName();
    while( !io->isEof() ) {
        MAlignment msa;
        AnnotationBank ann_bank;
        QString name;
        bool notCanceled = true;
        QHash< QString, QString > annMap;
        
        notCanceled = loadOneMsa( io, tsi, msa, ann_bank );
        if( !notCanceled ) {
            break;
        }
        uni_file = uni_file || isUniFile( ann_bank );

        name = getMsaName( ann_bank );
        name = ( QString::null == name || names_list.contains( name ) )?
            filename + "_" + QString::number( l.size() ): name;
        names_list.append( name );
        msa.setName( name );
        
        annMap = getAnnotationMap( ann_bank );
        setMsaInfo( annMap, msa );
        MAlignmentObject* obj = new MAlignmentObject(msa);
        obj->setIndexInfo(annMap);
        l.append( obj );
        if( onlyOne ) {
            break;
        }
    }
}

static int getMaxNameLen( const MAlignment& msa ) {
    assert( msa.getNumRows() != 0 );
    int sz = msa.getNumRows();
    int max_len = msa.getRow(0).getName().size();

    for( int i = 0; i < sz; ++i ) {
        int name_len =  msa.getRow(i).getName().size();
        max_len = ( max_len < name_len )? name_len: max_len;
    }
    return max_len;
}
//returns a gap between name and sequence in block
static QByteArray getNameSeqGap( int diff ) {
    assert( 0 <= diff );
    QByteArray ret = "    ";
    for( int i = 0; i < diff; ++i ) {
        ret.append( " " );
    }
    return ret;
}

static void save( IOAdapter* io, const MAlignment& msa, const QString& name ) {
    assert( NULL != io );
    assert( msa.getNumRows() );
    int ret = 0;

    QByteArray header( HEADER );
    ret = io->writeBlock( header );
    checkValThrowException<int>( true, header.size(), ret, StockholmFormat::WriteError(io->getURL()) );
    QByteArray unimark = StockholmFormat::UNI_ANNOTATION_MARK + "\n\n";
    ret = io->writeBlock( unimark );
    checkValThrowException<int>( true, unimark.size(), ret, StockholmFormat::WriteError(io->getURL()) );
    QByteArray idAnn = StockholmFormat::FILE_ANNOTATION_ID + " " + name.toAscii() + "\n\n";
    ret = io->writeBlock( idAnn );
    checkValThrowException<int>( true, idAnn.size(), ret, StockholmFormat::WriteError(io->getURL()) );
    
    //write sequences
    int name_max_len = getMaxNameLen( msa );
    int seq_len = msa.getLength();
    int cur_seq_pos = 0;
    while ( 0 < seq_len ) {
        int block_len = ( WRITE_BLOCK_LENGTH >= seq_len )? seq_len: WRITE_BLOCK_LENGTH;

        //write block
        int nRows = msa.getNumRows();
        for( int i = 0; i < nRows; ++i ) {
            const MAlignmentRow& row = msa.getRow(i);
            QByteArray name = row.getName().toAscii();
            TextUtils::replace(name.data(), name.length(), TextUtils::WHITES, '_');
            name += getNameSeqGap( name_max_len - row.getName().size() );
            ret = io->writeBlock( name );
            checkValThrowException<int>( true, name.size(), ret, StockholmFormat::WriteError(io->getURL()) );
            QByteArray seq = row.mid( cur_seq_pos, block_len ).toByteArray(block_len) + NEW_LINE;
            ret = io->writeBlock( seq );
            checkValThrowException<int>( true, seq.size(), ret, StockholmFormat::WriteError(io->getURL()) );
        }
        ret = io->writeBlock( QByteArray( "\n\n" ) );
        checkValThrowException<int>( true, 2, ret, StockholmFormat::WriteError(io->getURL()) );
        seq_len -= block_len;
        cur_seq_pos += block_len;
    }
    //write eof
    ret = io->writeBlock( QByteArray( "//\n" ) );
    checkValThrowException<int>( true, 3, ret, StockholmFormat::WriteError(io->getURL()) );
}

namespace U2 {
StockholmFormat::StockholmFormat( QObject *obj ) : DocumentFormat( obj , DocumentFormatFlags_SW, QStringList() << "sto") {
    format_name = tr( "Stockholm" );
    supportedObjectTypes+=GObjectTypes::MULTIPLE_ALIGNMENT;
}


Document* StockholmFormat::loadDocument( IOAdapter* io, TaskStateInfo& ti, const QVariantMap& fs , DocumentLoadMode mode) {
    if( NULL == io || !io->isOpen() ) {
        ti.setError(L10N::badArgument("IO adapter"));
        return NULL;
    }
    
    QList<GObject*> obj_list;
    try {
        bool uniFile = false;
        QString write_lock_reason;
        load( io, obj_list, ti, uniFile, mode);
        if ( !uniFile ) {
            write_lock_reason = DocumentFormat::CREATED_NOT_BY_UGENE;
        }
        return new Document( this, io->getFactory(), io->getURL(), obj_list, fs, write_lock_reason );
    }
    catch ( const StockholmBaseException& e ) {
        ti.setError(e.msg);
    }
    catch ( ... ) {
        ti.setError(tr( "unknown error occurred" ));
    }
    qDeleteAll(obj_list);
    return NULL;
}

void StockholmFormat::storeDocument( Document* doc, TaskStateInfo& ti, IOAdapter* io ) {
    try {
        foreach( GObject* p_obj, doc->getObjects() ) {
            const MAlignmentObject* aln_obj = qobject_cast<const MAlignmentObject*>( p_obj );
            assert( NULL != aln_obj );
            save( io, aln_obj->getMAlignment(), aln_obj->getGObjectName() );
            if( ti.cancelFlag ) {
                return;
            }
        }
    } catch( const StockholmBaseException& ex ) {
        ti.setError( ex.msg );
    } catch(...) {
        ti.setError(tr( "unknown error occurred" ));
    }
}

RawDataCheckResult StockholmFormat::checkRawData(const QByteArray& data, const GUrl&) const {
    bool headerIsOK = checkHeader( data.constData(), data.size());
    return headerIsOK ? FormatDetection_VeryHighSimilarity : FormatDetection_NotMatched;
}

bool StockholmFormat::isObjectOpSupported( const Document *doc, DocObjectOp op, GObjectType t ) const {
    Q_UNUSED( op ); Q_UNUSED( doc );
    assert( NULL != doc );
    if ( GObjectTypes::MULTIPLE_ALIGNMENT != t  ) {
        return false;
    }
    /*if (op == DocumentFormat::DocObjectOp_Add) {
        return doc->getObjects().isEmpty();
    }
    return false;*/
    return true;
}


} // U2
