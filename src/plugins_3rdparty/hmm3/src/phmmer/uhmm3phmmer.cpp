/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#include <util/uhmm3Utilities.h>
#include "uhmm3phmmer.h"

namespace U2 {

UHMM3PhmmerSettings::UHMM3PhmmerSettings() {
    UHMM3SearchSettings defaultSearch;
    setDefaultUHMM3SearchSettings( &defaultSearch );
    setSearchSettings( defaultSearch );
    
    UHMM3BuildSettings defaultBuild;
    setDefaultUHMM3BuildSettings( &defaultBuild );
    setBuildSettings( defaultBuild );
    
    popen   = 0.02;
    pextend = 0.4;
    substMatr = SMatrix();
}

UHMM3BuildSettings UHMM3PhmmerSettings::getBuildSettings() const {
    UHMM3BuildSettings ret;
    setDefaultUHMM3BuildSettings( &ret );
    ret.seed    = seed;
    ret.eml     = eml;
    ret.emn     = emn;
    ret.evl     = evl;
    ret.evn     = evn;
    ret.efl     = efl;
    ret.efn     = efn;
    ret.eft     = eft;
    return ret;
}

void UHMM3PhmmerSettings::setBuildSettings( const UHMM3BuildSettings & from ) {
    eml  = from.eml;
    emn  = from.emn;
    evl  = from.evl;
    evn  = from.evn;
    efl  = from.efl;
    efn  = from.efn;
    eft  = from.eft;
    seed = from.seed;
}

UHMM3SearchSettings UHMM3PhmmerSettings::getSearchSettings() const {
    UHMM3SearchSettings ret;
    setDefaultUHMM3SearchSettings( &ret );
    ret.e       = e;
    ret.t       = t;
    ret.z       = z;
    ret.domE    = domE;
    ret.domT    = domT;
    ret.domZ    = domZ;
    ret.incE    = incE;
    ret.incT    = incT;
    ret.incDomE = incDomE;
    ret.incDomT = incDomT;
    ret.f1      = f1;
    ret.f2      = f2;
    ret.f3      = f3;
    ret.doMax   = doMax;
    ret.noNull2 = noNull2;
    ret.noBiasFilter = noBiasFilter;
    return ret;
}

void UHMM3PhmmerSettings::setSearchSettings( const UHMM3SearchSettings & from ) {
    e        = from.e;
    t        = from.t;
    z        = from.z;
    domE     = from.domE;
    domT     = from.domT;
    domZ     = from.domZ;
    incE     = from.incE;
    incT     = from.incT;
    incDomE  = from.incDomE;
    incDomT  = from.incDomT;
    f1       = from.f1;
    f2       = from.f2;
    f3       = from.f3;
    doMax    = from.doMax;
    noNull2  = from.noNull2;
    noBiasFilter = from.noBiasFilter;
}

bool UHMM3PhmmerSettings::isValid() const {
    if( !( 0 <= popen && popen <= 0.5 ) ) { return false; }
    if( !( 0 <= pextend && pextend < 1 ) ) { return false; }
    if( !( 0 < e ) ) { return false; }
    if( !( 0 < t || OPTION_NOT_SET == t ) ) { return false; }
    if( !( 0 < z || OPTION_NOT_SET == z ) ) { return false; }
    if( !( 0 < domE ) ) { return false; }
    if( !( 0 < domT || OPTION_NOT_SET == domT ) ) { return false; }
    if( !( 0 < domZ || OPTION_NOT_SET == domZ ) ) { return false; }
    if( !( 0 < incE ) ) { return false; }
    if( !( 0 < incT || OPTION_NOT_SET == incT ) ) { return false; }
    if( !( 0 < incDomE ) ) { return false; }
    if( !( 0 < incDomT || OPTION_NOT_SET == incDomT ) ) { return false; }
    if( !( (bool)doMax == TRUE || (bool)doMax == FALSE ) ) { return false; }
    if( !( (bool)noBiasFilter == TRUE || (bool)noBiasFilter == FALSE ) ) { return false; }
    if( !( (bool)noNull2 == TRUE || (bool)noNull2 == FALSE ) ) { return false; }
    if( !( 0 < eml ) ) { return false; }
    if( !( 0 < emn ) ) { return false; }
    if( !( 0 < evl ) ) { return false; }
    if( !( 0 < evn ) ) { return false; }
    if( !( 0 < efl ) ) { return false; }
    if( !( 0 < efn ) ) { return false; }
    if( !( 0 < eft && eft < 1 ) ) { return false; }
    if( !( 0 <= seed ) ) { return false; }
    return true;
}

static void destroyAllIfYouCan( ESL_SQ * query, ESL_SQ * db, ESL_ALPHABET * abc, P7_BG * bg, P7_BUILDER * bld, P7_PIPELINE * pli,
                                P7_TOPHITS * th, P7_OPROFILE * om ) {
    if( NULL != query ) { esl_sq_Destroy( query ); }
    if( NULL != db )    { esl_sq_Destroy( db ); }
    if( NULL != abc )   { esl_alphabet_Destroy( abc ); }
    if( NULL != bg )    { p7_bg_Destroy( bg ); }
    if( NULL != bld )   { p7_builder_Destroy( bld ); }
    if( NULL != pli )   { p7_pipeline_Destroy( pli ); }
    if( NULL != th )    { p7_tophits_Destroy( th ); }
    if( NULL != om )    { p7_oprofile_Destroy( om ); }
}

const int PHMMER_PERCENT_PER_FILTER = 17;
const int PHMMER_SINGLE_BUILDER_PROGRESS = 15;

/* we catch all exceptions here */
UHMM3SearchResult UHMM3Phmmer::phmmer( const char * p_querySq, int querySqLen, const char * p_dbSq, int dbSqLen, 
                                       const UHMM3PhmmerSettings& settings, TaskStateInfo& ti, int wholeSeqSz ) {
    UHMM3SearchResult res;
    ESL_SQ          *querySq    = NULL;
    ESL_SQ          *dbSq       = NULL;
    ESL_ALPHABET    *abc        = NULL;
    P7_BG           *bg         = NULL;
    P7_BUILDER      *bld        = NULL;
    P7_PIPELINE     *pli        = NULL;
    P7_TOPHITS      *th         = NULL;
    P7_OPROFILE     *om         = NULL;
    int              status     = eslOK;
    QByteArray       errStr;
    
    if( ti.hasError() ) {
        return res;
    }
    if( NULL == p_querySq || 0 >= querySqLen ) {
        ti.setError( tr( "query_sequence_not_given" ) );
        return res;
    }
    if( NULL == p_dbSq || 0 >= dbSqLen ) {
        ti.setError( tr( "sequence_to_search_in_not_given" ) );
        return res;
    }
    
    try {
        abc = esl_alphabet_Create( eslAMINO );
        if( NULL == abc ) {
            errStr = tr( "no_memory:cannot_create_alphabet" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        bg = p7_bg_Create( abc );
        if( NULL == bg ) {
            errStr = tr( "no_memory:cannot_create_null_model" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        UHMM3BuildSettings bldSettings = settings.getBuildSettings();
        bld = p7_builder_Create( &bldSettings, abc );
        if( NULL == bld ) {
            errStr = tr( "no_memory:cannot_create_builder" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        ESL_SCOREMATRIX * hmmer3SubstMatr = UHMM3Utilities::convertScoreMatrix( settings.substMatr );
        status = p7_builder_SetScoreSystem( bld, hmmer3SubstMatr, settings.popen, settings.pextend );
        if( status != eslOK ) {
            errStr = tr( "failed_to_set_scoring_system_with_error:%1" ).arg( bld->errbuf ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        dbSq = esl_sq_CreateFrom( NULL, p_dbSq, dbSqLen, NULL, NULL, NULL );
        esl_sq_Digitize( abc, dbSq );
        if( NULL == dbSq || NULL == dbSq->dsq ) {
            errStr = tr( "cannot_convert_sequence:sequence_to_search_in" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        querySq = esl_sq_CreateFrom( NULL, p_querySq, querySqLen, NULL, NULL, NULL );
        esl_sq_Digitize( abc, querySq );
        if( NULL == querySq || NULL == querySq->dsq ) {
            errStr = tr( "cannot_convert_sequence:query_sequence" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        /* bypass HMM - only need model */
        status = p7_SingleBuilder( bld, querySq, bg, NULL, NULL, NULL, &om, PHMMER_SINGLE_BUILDER_PROGRESS, ti );
        if( eslCANCELED == status ) {
            errStr = tr( HMMER3_CANCELED_ERROR ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        } else if( eslOK != status ) {
            errStr = tr( "cannot_create_optimized_profile_for_query_sequence" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        th   = p7_tophits_Create();
        if( NULL == th ) {
            errStr = tr( "no_memory:cannot_create_top_hits_list" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        UHMM3SearchSettings searchSettings = settings.getSearchSettings();
        pli  = p7_pipeline_Create( &searchSettings, om->M, 400, p7_SEARCH_SEQS );  /* 400 is a dummy length for now */
        if( NULL == pli ) {
            errStr = tr( "no_memory:cannot_create_pipeline" ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        
        p7_pli_NewModel( pli, om, bg );
        p7_pli_NewSeq( pli, dbSq );
        p7_bg_SetLength( bg, wholeSeqSz );
        p7_oprofile_ReconfigLength( om, dbSq->n, wholeSeqSz );
        int ret = p7_Pipeline( pli, om, bg, dbSq, th, PHMMER_PERCENT_PER_FILTER, ti, wholeSeqSz );
        if( eslCANCELED == ret ) {
            errStr = tr( HMMER3_CANCELED_ERROR ).toAscii();
            throwUHMMER3Exception( errStr.data() );
        }
        assert( eslOK == ret );
        
        p7_tophits_Sort(th);
        p7_tophits_Threshold( th, pli );
        res.fillResults( th, pli );
    } catch( const UHMMER3Exception& ex ) {
        ti.setError( ex.msg );
    } catch(...) {
        ti.setError( tr( HMMER3_UNKNOWN_ERROR ) );
    }
    
    destroyAllIfYouCan( querySq, dbSq, abc, bg, bld, pli, th, om );
    return res;
}

} // U2
