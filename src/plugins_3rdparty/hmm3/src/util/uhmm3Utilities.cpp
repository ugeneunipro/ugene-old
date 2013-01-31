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

#include <gobject/uHMMObject.h>
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
    ok = allocAndCopyData( name.toAscii(), &msa->name );
    if( !ok ) {
        return false;
    }
    
    if( MAlignmentInfo::hasAccession( info ) ) {
        QString acc = MAlignmentInfo::getAccession( info );
        assert( !acc.isEmpty() );
        ok = allocAndCopyData( acc.toAscii(), &msa->acc );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasDescription( info ) ) {
        QString desc = MAlignmentInfo::getDescription( info );
        assert( !desc.isEmpty() );
        ok = allocAndCopyData( desc.toAscii(), &msa->desc );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasSSConsensus( info ) ) {
        QString cs = MAlignmentInfo::getSSConsensus( info );
        assert( !cs.isEmpty() );
        ok = allocAndCopyData( cs.toAscii(), &msa->ss_cons );
        if( !ok ) {
            return false;
        }
    }
    if( MAlignmentInfo::hasReferenceLine( info ) ) {
        QString rf = MAlignmentInfo::getReferenceLine( info );
        assert( !rf.isEmpty() );
        ok = allocAndCopyData( rf.toAscii(), &msa->rf );
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
        ok = allocAndCopyData( row.getName().toAscii(), &msa->sqname[i] );
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

P7_HMM * UHMM3Utilities::getHmmFromDocument( Document* doc, TaskStateInfo& ti ) {
    assert( NULL != doc );
    if( doc->getObjects().isEmpty() ) {
        ti.setError( "no_hmm_found_in_file" );
        return NULL;
    } else {
        UHMMObject* obj = qobject_cast< UHMMObject* >( doc->getObjects().first() );
        if( NULL == obj ) {
            ti.setError( "cannot_cast_to_hmm_object" );
            return NULL;
        }
        return (P7_HMM*)obj->getHMM();
    }
}

} // U2
