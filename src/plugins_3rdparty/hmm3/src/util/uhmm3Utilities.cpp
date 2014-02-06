/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include <cassert>

#include <U2Core/MAlignmentInfo.h>
#include <U2Core/SMatrix.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapterUtils.h>

#include <gobject/uHMMObject.h>
#include <format/uHMMFormat.h>
#include "uhmm3Utilities.h"

using namespace U2;

const char TERM_SYM = '\0';

static void copyData( const QByteArray& from, char* to ) {
    assert( NULL != to );
    int sz = from.size();
    const char* fromData = from.data();
    qCopy( fromData, fromData + sz, to );
    to[sz] = TERM_SYM;
}

static bool allocAndCopyData( const QByteArray& from, char** to ) {
    assert( NULL != to );
    *to = (char*)calloc( sizeof( char ), from.size() + 1 );
    if( NULL == *to ) {
        return false;
    }
    copyData( from, *to );
    return true;
}

static void setMsaCutoffs( const QVariantMap& info, ESL_MSA* msa, MAlignmentInfo::Cutoffs cof ) {
    assert( NULL != msa );
    if( MAlignmentInfo::hasCutoff( info, cof ) ) {
        int ind = static_cast< int >( cof );
        msa->cutoff[ind] = MAlignmentInfo::getCutoff( info, cof );
        msa->cutset[ind] = TRUE;
    }
}

static bool convertMalignmentInfo( const QVariantMap& info, ESL_MSA* msa ) {
    assert( NULL != msa );
    bool ok = false;
    
    if( !MAlignmentInfo::hasName( info ) ) {
        return false;
    }
    QString name = MAlignmentInfo::getName( info );
    assert( !name.isEmpty() );
    ok = allocAndCopyData( name.toLatin1(), &msa->name );
    if( !ok ) {
        return false;
    }
    
    if( MAlignmentInfo::hasAccession( info ) ) {
        QString acc = MAlignmentInfo::getAccession( info );
        assert( !acc.isEmpty() );
        ok = allocAndCopyData( acc.toLatin1(), &msa->acc );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasDescription( info ) ) {
        QString desc = MAlignmentInfo::getDescription( info );
        assert( !desc.isEmpty() );
        ok = allocAndCopyData( desc.toLatin1(), &msa->desc );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasSSConsensus( info ) ) {
        QString cs = MAlignmentInfo::getSSConsensus( info );
        assert( !cs.isEmpty() );
        ok = allocAndCopyData( cs.toLatin1(), &msa->ss_cons );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasReferenceLine( info ) ) {
        QString rf = MAlignmentInfo::getReferenceLine( info );
        assert( !rf.isEmpty() );
        ok = allocAndCopyData( rf.toLatin1(), &msa->rf );
        if( !ok ) {
            return false;
        }
    }
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_GA1 );
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_GA2 );
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_NC1 );
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_NC2 );
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_TC1 );
    setMsaCutoffs( info, msa, MAlignmentInfo::CUTOFF_TC2 );
    
    return true;
}


namespace U2 {

ESL_SCOREMATRIX * UHMM3Utilities::convertScoreMatrix( const SMatrix& it ) {
    if (it.isEmpty()) {
        return NULL;
    }
    assert( false );
    return NULL;
}

int UHMM3Utilities::convertAlphabetType( const DNAAlphabet * al ) {
    assert( NULL != al );
    DNAAlphabetType alType = al->getType();
    int ret = 0;
    
    switch( alType ) {
    case DNAAlphabet_RAW:
        ret = eslNONSTANDARD;
        break;
    case DNAAlphabet_AMINO:
        assert( BaseDNAAlphabetIds::AMINO_DEFAULT() == al->getId() );
        ret = eslAMINO;
        break;
    case DNAAlphabet_NUCL:
        {
            QString alId = al->getId();
            if( BaseDNAAlphabetIds::NUCL_DNA_DEFAULT() == alId || BaseDNAAlphabetIds::NUCL_DNA_EXTENDED() == alId ) {
                ret = eslDNA;
            } else if( BaseDNAAlphabetIds::NUCL_RNA_DEFAULT() == alId || BaseDNAAlphabetIds::NUCL_RNA_EXTENDED() == alId ) {
                ret = eslRNA;
            } else {
                ret = BAD_ALPHABET;
            }
        }
        break;
    default:
        ret = BAD_ALPHABET;
    }
    
    return ret;
}

ESL_MSA * UHMM3Utilities::convertMSA( const MAlignment & ma ) {
    ESL_MSA * msa = NULL;
    int i = 0;
    bool ok = false;
    int nseq = ma.getNumRows();
    int alen = ma.getLength();
    
    assert( 0 < nseq && 0 < alen );
    
    msa = esl_msa_Create( nseq, alen );
    if( NULL == msa ) {
        return NULL;
    }
    msa->nseq = nseq;
    for (i = 0; i < nseq; i++) {
		const MAlignmentRow& row = ma.getRow(i);
        ok = allocAndCopyData( row.getName().toLatin1(), &msa->sqname[i] );
        if( !ok ) {
            esl_msa_Destroy( msa );
            return NULL;
        }
        U2OpStatus2Log os;
		QByteArray sequence = row.toByteArray(ma.getLength(), os);
        copyData(sequence, msa->aseq[i] );
    }
    
    ok = convertMalignmentInfo( ma.getInfo(), msa );
    if( !ok ) {
        esl_msa_Destroy( msa );
        return NULL;
    }
    
    return msa;
}

QList<const P7_HMM *> UHMM3Utilities::getHmmsFromDocument( Document* doc, TaskStateInfo& ti ){
    QList<const P7_HMM *> res;
    SAFE_POINT( NULL != doc, "UHMM3Utilities::getHmmsFromDocument:: doc is NULL",  res);

    const QList<GObject*>& gobjects = doc->getObjects();
    foreach(GObject* gobj, gobjects){
        UHMMObject* obj = qobject_cast< UHMMObject* >( gobj );
        if( NULL != obj ) {
            res.append((P7_HMM*)obj->getHMM());
        }
    }

    if (res.isEmpty()){
        ti.setError( "no_hmm_found_in_file" );
    }
    return res;
}

QList< GObject* > UHMM3Utilities::getDocObjects( const QList< const P7_HMM* >& hmms ){
    QList< GObject* > res;
    foreach(const  P7_HMM* hmm, hmms ) {
        res.append( new UHMMObject( const_cast<P7_HMM*>(hmm), QString( hmm->name ) ) );
    }
    return res;
}

Document * UHMM3Utilities::getSavingDocument( const QList<const  P7_HMM* >& hmms, const QString & outfile ){
    assert( !hmms.isEmpty() );
    QList< GObject* > docObjects = getDocObjects( hmms );
    UHMMFormat* hmmFrmt = qobject_cast< UHMMFormat* >
        ( AppContext::getDocumentFormatRegistry()->getFormatById( UHMMFormat::UHHMER_FORMAT_ID ) );
    assert( NULL != hmmFrmt );

    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( IOAdapterUtils::url2io( outfile ) );
    assert( NULL != iof );

    U2OpStatus2Log os;
    Document* doc = hmmFrmt->createNewLoadedDocument(iof, outfile, os, QVariantMap());
    CHECK_OP(os, NULL);
    foreach(GObject* obj, docObjects) {
        doc->addObject(obj);
    }
    return doc;
}

} // U2
