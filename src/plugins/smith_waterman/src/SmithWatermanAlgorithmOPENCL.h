#ifdef SW2_BUILD_WITH_OPENCL
 
#ifndef _SMITHWATERMANALGORITHM_OPENCL_H
#define _SMITHWATERMANALGORITHM_OPENCL_H

#include <U2Core/AppContext.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <U2Algorithm/OpenCLHelper.h>
 
#include "SmithWatermanAlgorithm.h"

namespace U2 {

class SmithWatermanAlgorithmOPENCL : public SmithWatermanAlgorithm {
public:
    typedef int ScoreType;
 
    SmithWatermanAlgorithmOPENCL();
    ~SmithWatermanAlgorithmOPENCL();

    virtual void launch(const SMatrix& m, QByteArray const & _patternSeq, QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore);
    static quint64 estimateNeededGpuMemory( const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq );
private:

    int calcOverlap(int queryLength);
    int calcPartsNumber(int seqLibLength, int overlapLength);
    int calcPartSeqSize(int seqLibLength, int overlapLength, int partsNumber);
    int calcSizeRow(int partsNumber, int partSeqSize);

    const int MAX_BLOCKS_NUMBER;
    const int MAX_SHARED_VECTOR_LENGTH;

    cl_event clEvent;
    cl_kernel clKernel;
    cl_program clProgram;
    cl_command_queue clCommandQueue;
    cl_context clContext;

    cl_mem queryProfBuf;
    cl_mem seqLibProfBuf;
    cl_mem hDataMaxBuf;
    cl_mem hDataUpBufTmp;
    cl_mem hDataRecBufTmp;
    cl_mem fDataUpBuf;
    cl_mem directionsUpBufTmp;
    cl_mem directionsRecBufTmp;
    cl_mem directionsMaxBuf;
};

} // namespace

#endif
#endif //SW2_BUILD_WITH_OPENCL
