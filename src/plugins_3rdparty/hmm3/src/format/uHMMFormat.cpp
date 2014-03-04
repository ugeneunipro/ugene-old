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

#include <memory>

#include <QtCore/QList>
#include <QtCore/QtAlgorithms>

#include <U2Core/U2SafePoints.h>
#include <U2Core/L10n.h>
#include <U2Core/Task.h>
#include <U2Core/TextUtils.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2DbiUtils.h>

#include <gobject/uHMMObject.h>
#include "uHMMFormat.h"
#include "uHMMFormatReader.h"

using namespace U2;

const QString UHMMFormat::WRITE_FAILED  = UHMMFormat::tr( "write_to_file_failed" );

static void loadOne( IOAdapter* io, QList< GObject* >& objects, U2OpStatus& os) {
    CHECK_OP(os, );
    
    UHMMFormatReader reader( io, os);
    P7_HMM * hmm = reader.getNextHmm();
    
    CHECK_OP(os, );
    
    assert(hmm  != NULL);
    QString objName( hmm->name );
    UHMMObject* obj = new UHMMObject( hmm, objName );
    objects.append( obj );
}

static void loadAll( IOAdapter* io, QList< GObject* >& objects, U2OpStatus& os) {
    assert( NULL != io && io->isOpen() );
    
    while( !io->isEof() && !os.isCoR()) {
        loadOne( io, objects, os);
        os.setProgress(io->getProgress());
    }
}

static void checkBytesWrittenThrowException( qint64 wantedToWrite, qint64 written, const QString& msg ) {
    if( wantedToWrite != written ) {
        throw UHMMFormat::UHMMWriteException( msg );
    }
}

static void writeHMMASCIIStr( IOAdapter* io, const QByteArray& str ) {
    assert( NULL != io && io->isOpen() );
    qint64 bytesWritten = io->writeBlock( str );
    checkBytesWrittenThrowException( str.size(), bytesWritten, UHMMFormat::WRITE_FAILED );
}

static void writeHMMASCIIStr( IOAdapter* io, const char* str, int num ) {
    assert( NULL != io && io->isOpen() );
    assert( NULL != str && 0 < num );
    qint64 bytesWritten = io->writeBlock( str, num );
    checkBytesWrittenThrowException( num, bytesWritten, UHMMFormat::WRITE_FAILED );
}

static void writeHMMASCIIStr( IOAdapter* io, const char* s1, const char* s2 ) {
    QString str;
    QTextStream txtStream( &str );
    txtStream << s1 << s2 << "\n";
    writeHMMASCIIStr( io, str.toLatin1() );
}

static void writeHMMASCIIStr( IOAdapter* io, const char* s1, const QString& s2 ) {
    QString str;
    QTextStream txtStream( &str );
    txtStream << s1 << s2 << "\n";
    writeHMMASCIIStr( io, str.toLatin1() );
}

static void writeHMMHeaderASCII( IOAdapter* io ) {
    QTextStream txtStream;
    QString headerStr;
    txtStream.setString( &headerStr );
    txtStream << "HMMER3/b [" << HMMER_VERSION << " | " << HMMER_DATE << "]\n";
    writeHMMASCIIStr( io, headerStr.toLatin1() );
}

static void writeHMMMultiLine( IOAdapter *io, const char *pfx, char *s ) {
    QString res;
    char *sptr  = s;
    char *end   = NULL;
    int   n     = 0;
    int   nline = 1;
    
    do {
        end = strchr(sptr, '\n');
        if (end != NULL) { 		             /* if there's no \n left, end == NULL */
            n = end - sptr;	                     /* n chars exclusive of \n */
            
            res = QString().sprintf( "%s [%d] ", pfx, nline++ );
            writeHMMASCIIStr( io, res.toLatin1() );
            writeHMMASCIIStr( io, sptr, n );
            writeHMMASCIIStr( io, QByteArray( "\n" ) );
            sptr += n + 1;	                     /* +1 to get past \n */
        } else {
            res = QString().sprintf( "%s [%d] %s\n", pfx, nline++, sptr );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
    } while (end != NULL  && *sptr != '\0');   /* *sptr == 0 if <s> terminates with a \n */
}

static void writeHMMProb( IOAdapter* io, int fieldwidth, float p ) {
    assert( NULL != io && io->isOpen() );
    QString res;
    if( p == 0.0 ) {
        res = QString().sprintf( " %*s", fieldwidth, "*" );
        writeHMMASCIIStr( io, res.toLatin1() );
    } else if( p == 1.0 ) {
        res = QString().sprintf( " %*.5f", fieldwidth, 0.0 );
        writeHMMASCIIStr( io, res.toLatin1() );
    } else {
        res = QString().sprintf( " %*.5f", fieldwidth, -logf(p) );
        writeHMMASCIIStr( io, res.toLatin1() );
    }
}

static void saveOne( IOAdapter* io, const P7_HMM* hmm, U2OpStatus& os) {
    assert( NULL != hmm );
    assert( NULL != io && io->isOpen() );
    assert( !os.hasError() );
    
    try {
        int k = 0;
        int x = 0;
        QString res;
        
        writeHMMHeaderASCII( io );
        writeHMMASCIIStr( io, "NAME  ", hmm->name );
        
        if (hmm->flags & p7H_ACC) {
            writeHMMASCIIStr( io, "ACC   ", hmm->acc );
        }
        if (hmm->flags & p7H_DESC) {
            writeHMMASCIIStr( io, "DESC  ", hmm->desc );
        }
        writeHMMASCIIStr( io, "LENG  ", QString::number( hmm->M ) );
        writeHMMASCIIStr( io, "ALPH  ", esl_abc_DecodeType( hmm->abc->type ) );
        writeHMMASCIIStr( io, "RF    ", ( hmm->flags & p7H_RF )? "yes" : "no" );
        writeHMMASCIIStr( io, "CS    ", ( hmm->flags & p7H_CS )? "yes" : "no" );
        writeHMMASCIIStr( io, "MAP   ", ( hmm->flags & p7H_MAP )? "yes" : "no" );
        
        if (hmm->ctime    != NULL) {
            writeHMMASCIIStr( io, "DATE  ", hmm->ctime );
        }
        if (hmm->comlog   != NULL) {
            writeHMMMultiLine( io, "COM  ", hmm->comlog );
        }
        if (hmm->nseq     >= 0) {
            writeHMMASCIIStr( io, "NSEQ  ", QString::number( hmm->nseq ) );
        }
        if (hmm->eff_nseq >= 0) {
            res = QString().sprintf( "EFFN  %f\n", hmm->eff_nseq );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        if (hmm->flags & p7H_CHKSUM) {
            writeHMMASCIIStr( io, "CKSUM ", QString::number( hmm->checksum ) );
        }
        
        if (hmm->flags & p7H_GA) {
            res = QString().sprintf( "GA    %.2f %.2f\n", hmm->cutoff[p7_GA1], hmm->cutoff[p7_GA2] );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        if (hmm->flags & p7H_TC) {
            res = QString().sprintf( "TC    %.2f %.2f\n", hmm->cutoff[p7_TC1], hmm->cutoff[p7_TC2] );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        if (hmm->flags & p7H_NC) {
            res = QString().sprintf( "NC    %.2f %.2f\n", hmm->cutoff[p7_NC1], hmm->cutoff[p7_NC2] );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        
        if (hmm->flags & p7H_STATS) {
            res = QString().sprintf( "STATS LOCAL MSV      %8.4f %8.5f\n", hmm->evparam[p7_MMU],  hmm->evparam[p7_MLAMBDA] );
            writeHMMASCIIStr( io, res.toLatin1() );
            res = QString().sprintf( "STATS LOCAL VITERBI  %8.4f %8.5f\n", hmm->evparam[p7_VMU],  hmm->evparam[p7_VLAMBDA] );
            writeHMMASCIIStr( io, res.toLatin1() );
            res = QString().sprintf( "STATS LOCAL FORWARD  %8.4f %8.5f\n", hmm->evparam[p7_FTAU], hmm->evparam[p7_FLAMBDA] );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        writeHMMASCIIStr( io, QByteArray( "HMM     " ) );
        
        for (x = 0; x < hmm->abc->K; x++) {
            res = QString().sprintf( "     %c   ", hmm->abc->sym[x] );
            writeHMMASCIIStr( io, res.toLatin1() );
        }
        writeHMMASCIIStr( io, QByteArray( "\n" ) );
        res = QString().sprintf( "        %8s %8s %8s %8s %8s %8s %8s\n", "m->m", "m->i", "m->d", "i->m", "i->i", "d->m", "d->d" );
        writeHMMASCIIStr( io, res.toLatin1() );
        
        if (hmm->flags & p7H_COMPO) {
            writeHMMASCIIStr( io, QByteArray( "  COMPO " ) );

            for (x = 0; x < hmm->abc->K; x++) {
                writeHMMProb( io, 8, hmm->compo[x] );
            }
            writeHMMASCIIStr( io, QByteArray( "\n" ) );
        }
        
        /* node 0 is special: insert emissions, and B-> transitions */
        writeHMMASCIIStr( io, QByteArray( "        " ) );
        
        for (x = 0; x < hmm->abc->K; x++) {
            writeHMMProb( io, 8, hmm->ins[0][x] );
        }
        writeHMMASCIIStr( io, QByteArray( "\n" ) );
        writeHMMASCIIStr( io, QByteArray( "        " ) );
        
        for (x = 0; x < p7H_NTRANSITIONS; x++) {
            writeHMMProb( io, 8, hmm->t[0][x] );
        }
        writeHMMASCIIStr( io, QByteArray( "\n" ) );
        
        for (k = 1; k <= hmm->M; k++) {
            /* Line 1: k; match emissions; optional map, RF, CS */ 
            res = QString().sprintf( " %6d ",  k );
            writeHMMASCIIStr( io, res.toLatin1() );
            
            for (x = 0; x < hmm->abc->K; x++) {
                writeHMMProb( io, 8, hmm->mat[k][x] );
            }
            if (hmm->flags & p7H_MAP) {
                res = QString().sprintf( " %6d", hmm->map[k] );
                writeHMMASCIIStr( io, res.toLatin1() );
            } else {
                res = QString().sprintf( " %6s", "-" );
                writeHMMASCIIStr( io, res.toLatin1() );
            }
            res = QString().sprintf( " %c",   (hmm->flags & p7H_RF) ? hmm->rf[k] : '-' );
            writeHMMASCIIStr( io, res.toLatin1() );
            res = QString().sprintf( " %c\n", (hmm->flags & p7H_CS) ? hmm->cs[k] : '-' );
            writeHMMASCIIStr( io, res.toLatin1() );
            
            /* Line 2:   insert emissions */
            writeHMMASCIIStr( io, QByteArray( "        " ) );
            for (x = 0; x < hmm->abc->K; x++) {
                writeHMMProb( io, 8, hmm->ins[k][x] );
            }
            
            /* Line 3:   transitions */
            writeHMMASCIIStr( io, QByteArray( "\n        " ) );
            for (x = 0; x < p7H_NTRANSITIONS; x++) {
                writeHMMProb( io, 8, hmm->t[k][x] );
            }
            writeHMMASCIIStr( io, QByteArray( "\n" ) );
        }
        writeHMMASCIIStr( io, QByteArray( "//\n" ) );
    } catch( const UHMMFormat::UHMMWriteException& ex ) {
        os.setError( ex.what );
    } catch(...) {
        os.setError( UHMMFormat::tr( "unknown_error_occurred" ) );
    }
}

static void saveAll( IOAdapter* io, const QList< GObject* >& objects, U2OpStatus& os ) {
    assert( NULL != io && io->isOpen() );
    QList< const P7_HMM* > hmms;
    
    foreach( const GObject* obj, objects ) {
        const UHMMObject* hmmObj = qobject_cast< const UHMMObject* >(obj);
        CHECK_EXT(hmmObj != NULL, os.setError(L10N::badArgument("Objects in document")), );
        hmms.append( hmmObj->getHMM() );
    }
    
    foreach( const P7_HMM* hmm, hmms ) {
        saveOne(io, hmm, os);
        CHECK_OP(os, );
    }
}

namespace U2 {

const DocumentFormatId  UHMMFormat::UHHMER_FORMAT_ID   = "hmmer_document_format";
const QString           UHMMFormat::WRITE_LOCK_REASON  = UHMMFormat::tr( "hmm_files_are_read_only" );

UHMMFormat::UHMMFormat( QObject* obj ) : DocumentFormat( obj, DocumentFormatFlags_SW, QStringList("hmm")) {
    formatName = tr( "hmmer_format" );
	formatDescription = tr("hmm is a format for storing hmm profiles");
	supportedObjectTypes+=UHMMObject::UHMM_OT;
}

DocumentFormatId UHMMFormat::getFormatId() const {
    return UHHMER_FORMAT_ID;
}

const QString& UHMMFormat::getFormatName() const {
    return formatName;
}

Document* UHMMFormat::loadDocument(IOAdapter* io, const U2DbiRef& targetDb, const QVariantMap& hints, U2OpStatus& os) {
    QList< GObject* > objects;
    loadAll( io, objects, os );
    CHECK_OP_EXT(os, qDeleteAll(objects), NULL);
    return new Document( this, io->getFactory(), io->getURL(), targetDb, objects, hints, WRITE_LOCK_REASON );
}


void UHMMFormat::storeDocument( Document* doc, IOAdapter* io, U2OpStatus& os) {
    saveAll(io, doc->getObjects(), os);
}

FormatCheckResult UHMMFormat::checkRawData( const QByteArray& data, const GUrl&) const {
    bool result =  data.startsWith( UHMMFormatReader::HMMER2_VERSION_HEADER.toLatin1() ) 
                || data.startsWith( UHMMFormatReader::HMMER3_VERSION_HEADER.toLatin1() );
    
    return result ? FormatDetection_Matched : FormatDetection_NotMatched;
}

} // U2
