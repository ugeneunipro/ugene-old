#ifndef _GB2_UHMM3_UTILITIES_H_
#define _GB2_UHMM3_UTILITIES_H_

#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/MAlignment.h>

#include <hmmer3/hmmer.h>
#include <hmmer3/easel/esl_scorematrix.h>
#include <hmmer3/easel/esl_msa.h>

namespace U2 {

class SMatrix;

class UHMM3Utilities {
public:
    static const int BAD_ALPHABET = -1;
    
public:
    static ESL_SCOREMATRIX * convertScoreMatrix( const SMatrix& it );
    
    static int convertAlphabetType( const DNAAlphabet * al );
    
    static ESL_MSA * convertMSA( const MAlignment & ma );
    
    static P7_HMM * getHmmFromDocument( Document* doc, TaskStateInfo& ti );
    
}; // UHMM3Utilities

} // U2

#endif // _GB2_UHMM3_UTILITIES_H_
