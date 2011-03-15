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

#include <math.h>
#include <cassert>

#include "uhmm3SearchResult.h"

using namespace U2;

static UHMM3SearchSeqDomainResult getDomainRes( const P7_HIT* hit, int dom, const P7_PIPELINE* pli ) {
    assert( NULL != hit );
    assert( 0 <= dom && dom < hit->ndom );
    UHMM3SearchSeqDomainResult res;
    const P7_DOMAIN& domain = hit->dcl[dom];
    
    res.isSignificant = domain.is_included;
    res.score       = domain.bitscore;
    res.bias        = domain.dombias;
    res.ival        = domain.pvalue * pli->Z;
    res.cval        = domain.pvalue * pli->domZ;
    res.queryRegion = U2Region( domain.ad->hmmfrom, domain.ad->hmmto - domain.ad->hmmfrom );
    res.seqRegion   = U2Region( domain.ad->sqfrom - 1, domain.ad->sqto - domain.ad->sqfrom + 1 );
    res.envRegion   = U2Region( domain.ienv, domain.jenv - domain.ienv );
    res.acc         = domain.oasc / ( 1.0 + fabs( (float)( domain.jenv - domain.ienv ) ) );
    return res;
}

namespace U2 {

void UHMM3SearchSeqDomainResult::writeQualifiersToAnnotation( AnnotationData * annData ) const {
    assert( NULL != annData );
    annData->qualifiers << U2Qualifier( "Independent e-value", QString().sprintf( "%.5e", ival ) );
    annData->qualifiers << U2Qualifier( "Conditional e-value", QString().sprintf( "%.5e", cval ) );
    annData->qualifiers << U2Qualifier( "Score", QString().sprintf( "%1.f", score ) );
    annData->qualifiers << U2Qualifier( "Bias", QString().sprintf( "%.5e", bias ) );
    annData->qualifiers << U2Qualifier( "Accuracy per residue", QString().sprintf( "%.5e", acc ) );
    annData->qualifiers << U2Qualifier( "HMM region", QString().sprintf( "%d...%d", queryRegion.startPos, queryRegion.endPos()) );
    annData->qualifiers << U2Qualifier( "Envelope of domain location", QString().sprintf( "%d...%d", 
        envRegion.startPos, envRegion.endPos() ) );
}

void UHMM3SearchResult::fillDomainsResult( const P7_TOPHITS* th, const P7_PIPELINE* pli ) {
    assert( NULL != th && ( 0 == th->N || 1 == th->N ) );
    assert( NULL != pli );
    
    if( 0 == th->N ) {
        return;
    }
    
    P7_HIT* hit = th->hit[0];
    
    int d = 0;
    for( d = 0; d < hit->ndom; d++ ) {
        if( hit->dcl[d].is_reported ) {
            domainResList << getDomainRes( hit, d, pli );
        }
    }
}

void UHMM3SearchResult::fillFullSeqResults( const P7_TOPHITS* th, const P7_PIPELINE* pli ) {
    assert( NULL != th && ( 1 == th->N || 0 == th->N ) );
    assert( NULL != pli );
    fullSeqResult.isReported = th->nreported != 0;
    if( !th->N ) {
        assert( !fullSeqResult.isReported );
        return;
    }
    P7_HIT* hit = th->hit[0];
    if( !(hit->flags & p7_IS_REPORTED) ) {
        return;
    }
    
    fullSeqResult.eval     = hit->pvalue * pli->Z;
    fullSeqResult.score    = hit->score;
    fullSeqResult.bias     = hit->pre_score - hit->score;
    fullSeqResult.expectedDomainsNum = hit->nexpected;
    fullSeqResult.reportedDomainsNum = hit->nreported;
}

void UHMM3SearchResult::fillResults( const P7_TOPHITS* th, const P7_PIPELINE* pli ) {
    assert( NULL != th );
    assert( NULL != pli );
    fillFullSeqResults( th, pli );
    fillDomainsResult(  th, pli );
}

} // U2
