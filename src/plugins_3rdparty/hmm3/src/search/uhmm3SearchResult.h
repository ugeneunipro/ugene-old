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
