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

#ifndef _GB2_UHMM3_SEARCH_RESULT_H_
#define _GB2_UHMM3_SEARCH_RESULT_H_

#include <U2Core/U2Region.h>
#include <U2Core/AnnotationData.h>

#include <hmmer3/hmmer.h>

namespace U2 {

class UHMM3SearchSeqDomainResult {
public:
    float   score;
    float   bias;
    double  ival; /* independent e-value */
    double  cval; /* conditional e-value */
    
    U2Region queryRegion; /* hmm region for hmmsearch and seq region for phmmer */
    U2Region seqRegion;
    U2Region envRegion; /* envelope of domains location */
    
    double  acc; /* expected accuracy per residue of the alignment */
    
    bool    isSignificant; /* domain meets inclusion tresholds */
    
public:
    void writeQualifiersToAnnotation( AnnotationData * annData ) const;
    
}; // UHMM3SearchSeqDomainResult

class UHMM3SearchCompleteSeqResult {
public:
    double  eval;
    float   score;
    float   bias;
    float   expectedDomainsNum;
    int     reportedDomainsNum;
    bool    isReported;
    
    UHMM3SearchCompleteSeqResult() : isReported( false ) {}
}; // UHMM3SearchCompleteSeqResult

class UHMM3SearchResult {
public:
    UHMM3SearchCompleteSeqResult           fullSeqResult;
    QList< UHMM3SearchSeqDomainResult >    domainResList;
    
    void fillResults( const P7_TOPHITS* th, const P7_PIPELINE* pli );
    
private:
    void fillFullSeqResults( const P7_TOPHITS* th, const P7_PIPELINE* pli );
    void fillDomainsResult(  const P7_TOPHITS* th, const P7_PIPELINE* pli );
    
}; // UHMM3SearchResult

} // U2

#endif // _GB2_UHMM3_SEARCH_RESULT_H_
