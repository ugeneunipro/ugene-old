#ifdef SW2_BUILD_WITH_CUDA

#ifndef _SW_CUDA_CPP_H
#define _SW_CUDA_CPP_H

#include <U2Core/U2Region.h>

struct resType
{
    U2::U2Region reg;
    int score;
};

class sw_cuda_cpp {
public:    
    typedef int ScoreType;

    QList<resType> launch(const char * seqLib, int seqLibLength, ScoreType* queryProfile, ScoreType qProfLen, int queryLength, ScoreType gapOpen, ScoreType gapExtension, ScoreType maxScore);
    static quint64 estimateNeededGpuMemory( int seqLibLength, ScoreType qProfLen, int queryLength );
    static const int MAX_BLOCKS_NUMBER;
    static const int MAX_SHARED_VECTOR_LENGTH;
};

#endif

#endif //SW2_BUILD_WITH_CUDA
