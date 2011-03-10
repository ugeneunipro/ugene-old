#ifdef SW2_BUILD_WITH_CUDA

#include "SmithWatermanAlgorithmCUDA.h"
#include "sw_cuda_cpp.h"
#include "PairAlignSequences.h"

#include <U2Core/Log.h>

#include <iostream>

using namespace std;

namespace U2 {
    
    
    quint64 SmithWatermanAlgorithmCUDA::estimateNeededGpuMemory( const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq ) {
        const QByteArray & alphChars = sm.getAlphabet()->getAlphabetChars();
        int subLen = alphChars.size();
        int qLen = _patternSeq.size();        
        int profLen = subLen * (qLen + 1) * (alphChars[ alphChars.size()-1 ] + 1);
        
        return sw_cuda_cpp::estimateNeededGpuMemory( _searchSeq.size(), profLen, qLen );
    }

    void SmithWatermanAlgorithmCUDA::launch(const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore) {
    
        algoLog.details("START SmithWatermanAlgorithmCUDA::launch");

        int qLen = _patternSeq.size();

        int subLen = sm.getAlphabet()->getNumAlphabetChars();

        //alphChars is sorted
        const QByteArray & alphChars = sm.getAlphabet()->getAlphabetChars();
        int profLen = subLen * (qLen + 1) * (alphChars[ alphChars.size()-1 ] + 1);

        ScoreType *  queryProfile = new ScoreType[profLen];

        for (int i = 0; i < profLen; i++) {
            queryProfile[i] = 0;
        }
        
        //calculate query profile
        for (int i = 0; i < subLen; i++) {
            for (int j = 0; j < qLen; j++) {
                char ch = alphChars[i];
                queryProfile[ch * qLen + j] = 
                    sm.getScore(ch, _patternSeq.at(j));            
            }        
        }

        sw_cuda_cpp sw;

        QList<resType> pResults = sw.launch(_searchSeq.constData(), _searchSeq.size(), queryProfile, profLen, qLen, (-1)*_gapOpen, (-1)*_gapExtension, _minScore);    

    // Collect results
        PairAlignSequences tmp;
        resType res;
        QMutableListIterator<resType> iter(pResults);
        while( iter.hasNext() ) {

            res = iter.next();

            tmp.score = res.score;

            tmp.intervalSeq1.startPos = res.reg.startPos;

            tmp.intervalSeq1.length = res.reg.length;

            pairAlignmentStrings.append(tmp);    

        }

        //free memory
        delete[] queryProfile;
        
        algoLog.details("FINISH SmithWatermanAlgorithmCUDA::launch");
    }

} //namespase

#endif //SW2_BUILD_WITH_CUDA

