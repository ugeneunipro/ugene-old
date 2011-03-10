#ifdef SW2_BUILD_WITH_CUDA

#ifndef _SMITHWATERMANALGORITHM_CUDA_H
#define _SMITHWATERMANALGORITHM_CUDA_H

#include "SmithWatermanAlgorithm.h"

namespace U2 {

class SmithWatermanAlgorithmCUDA : public SmithWatermanAlgorithm {
public:
    typedef int ScoreType;

    virtual void launch(const SMatrix& m, QByteArray const & _patternSeq, QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore);
    static quint64 estimateNeededGpuMemory( const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq );
private:

};


} // namespace

#endif
#endif //SW2_BUILD_WITH_CUDA
