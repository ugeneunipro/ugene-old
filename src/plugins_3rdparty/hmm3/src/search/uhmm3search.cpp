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

#include <U2Core/DNAInfo.h>

#include <util/uhmm3Utilities.h>
#include "uhmm3search.h"

using namespace U2;

static void delAllIfYouCan( ESL_SQ* dbsq, ESL_ALPHABET* abc, P7_BG* bg, P7_PIPELINE* pli, P7_TOPHITS* th, P7_PROFILE* gm, P7_OPROFILE* om ) {
    if( NULL != bg )    { p7_bg_Destroy( bg ); }
    if( NULL != pli )   { p7_pipeline_Destroy( pli ); }
    if( NULL != th )    { p7_tophits_Destroy( th ); }
    if( NULL != gm )    { p7_profile_Destroy( gm ); }
    if( NULL != om )    { p7_oprofile_Destroy( om ); }
    if( NULL != dbsq )  { esl_sq_Destroy( dbsq ); }
    if( NULL != abc )   { esl_alphabet_Destroy( abc ); }
}

namespace U2 {

const int SEARCH_PERCENT_PER_FILTERS = 20;

/* we catch all exceptions here. don't worry
*/
UHMM3SearchResult UHMM3Search::search( const P7_HMM* hmm, const char* sq, int sqLen, 
                                      const UHMM3SearchSettings& settings, TaskStateInfo& tsi, int wholeSeqSz ) {
    ESL_SQ            *dbsq    = NULL;
    ESL_ALPHABET      *abc     = NULL; /* copy of abc in hmm */
    P7_BG             *bg      = NULL;
    P7_PIPELINE       *pli     = NULL;
    P7_TOPHITS        *th      = NULL;
    P7_PROFILE        *gm      = NULL;
    P7_OPROFILE       *om      = NULL;
    UHMM3SearchResult  res;
    QByteArray errStr;
    
    if( tsi.hasError() || tsi.cancelFlag ) {
        return res;
    }
    if( NULL == hmm ) {
        tsi.setError( tr( "no_hmm_given" ) );
        return res;
    }
    if( NULL == sq || 0 >= sqLen ) {
        tsi.setError( tr( "no_sequence_given" ) );
        return res;
    }
    
    try {
        // preparing...
		dbsq = esl_sq_CreateFrom( NULL, sq, sqLen, NULL, NULL, NULL );
        if( NULL == dbsq ) {
            errStr = tr( "no_memory:cannot_convert_sequence" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        int dbsqAbcType = eslUNKNOWN;
        esl_sq_GuessAlphabet( dbsq, &dbsqAbcType );
        if( eslUNKNOWN != dbsqAbcType && dbsqAbcType != hmm->abc->type ) { /* if we can recognize */
            errStr = tr( "sequence_and_hmm_alphabets_not_match" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        
        abc = esl_alphabet_Create( hmm->abc->type );
        if( NULL == abc ) {
            errStr = tr( "no_memory:cannot_create_alphabet" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        esl_sq_Digitize( abc, dbsq );
        if( NULL == dbsq || NULL == dbsq->dsq ) {
            errStr = tr( "cannot_digitize_sequence" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        
        bg = p7_bg_Create( abc );
        if( NULL == bg ) {
            errStr = tr( "no_memory:cannot_create_null_model" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        
        gm = p7_profile_Create (hmm->M, abc);
        if( NULL == gm ) {
            errStr = tr( "no_memory:cannot_create_profile" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        om = p7_oprofile_Create(hmm->M, abc);
        if( NULL == om ) {
            errStr = tr( "no_memory:cannot_create_optimized_profile" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        p7_ProfileConfig(hmm, bg, gm, 100, p7_LOCAL); /* 100 is a dummy length for now; and MSVFilter requires local mode */
        p7_oprofile_Convert(gm, om);                  /* <om> is now p7_LOCAL, multihit */
        
        /* Create processing pipeline and hit list */
        pli = p7_pipeline_Create( &settings, om->M, 100, p7_SEARCH_SEQS); /* L_hint = 100 is just a dummy for now */
        if( NULL == pli ) {
            errStr = tr( "no_memory:cannot_create_pipeline" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        th  = p7_tophits_Create();
        if( NULL == th ) {
            errStr = tr( "no_memory:cannot_create_tophits" ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        
        // main part
		p7_pli_NewModel( pli, om, bg );
        p7_pli_NewSeq( pli, dbsq );
        // !!! CODE CHANGED !!!
        //p7_bg_SetLength( bg, dbsq->n );
        p7_bg_SetLength( bg, wholeSeqSz );
        // !!! CODE CHANGED !!!
        //p7_oprofile_ReconfigLength( om, dbsq->n);
        p7_oprofile_ReconfigLength( om, dbsq->n, wholeSeqSz );
        
        tsi.progress = 0;
        int ret = p7_Pipeline( pli, om, bg, dbsq, th, SEARCH_PERCENT_PER_FILTERS, tsi, wholeSeqSz );
        if( eslCANCELED == ret ) {
            errStr = tr( HMMER3_CANCELED_ERROR ).toLatin1();
            throwUHMMER3Exception( errStr.data() );
        }
        assert( eslOK == ret );
        
        // work with results
		p7_tophits_Sort(th);
        p7_tophits_Threshold(th, pli);
        res.fillResults( th, pli );
    } catch( const UHMMER3Exception& ex ) {
        tsi.setError( ex.msg );
    } catch(...) {
        tsi.setError( tr( HMMER3_UNKNOWN_ERROR ) );
    }
    
    delAllIfYouCan( dbsq, abc, bg, pli, th, gm, om );
    return res;
}

} // U2
