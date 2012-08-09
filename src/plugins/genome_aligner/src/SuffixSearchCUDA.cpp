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

#ifdef GA_BUILD_WITH_CUDA
#include <cuda_runtime.h>
#endif
#include <U2Core/AppContext.h>
#include <U2Core/Log.h>
#include <U2Algorithm/CudaGpuRegistry.h>
#include "GenomeAlignerIndexPart.h"
#include "GenomeAlignerFindTask.h"
#include "SuffixSearchCUDA.h"


#define SAFE_CALL(call) { \
    cudaError_t err = call; \
    if ( cudaSuccess != err) { \
    stateInfo.setError(QString("CUDA driver error: %1 at %2:%3").arg(cudaGetErrorString(err)).arg(__FILE__).arg(__LINE__)); \
    return; \
    } }\

namespace U2 {

#ifdef GA_BUILD_WITH_CUDA

extern "C" void cudaBinarySearch(quint64* pList1Dev, int nList1Length, quint64* pList2Dev, int nList2Length, int *windowSizes);

extern "C" void cudaAlignReads(char* readsData, 
                               int* readSizes,
                               int* readOffsets,
                               int readsNumber,
                               char* refSeq,
                               int refSeqSize,
                               SAType* sArrayData,
                               int sArraySize,
                               BMType* bmArrayData,
                               int bmArraySize,
                               BMType bitFilter,
                               int refStartPos,
                               int loadedPartSize,
                               int CMAX,
                               int w,
                               quint32* results);

quint64* U2::SuffixSearchCUDA::runSearch( const quint64* numbers, const int numbersSize, 
                                            const quint64* query, const int querySize, const int *windowSizes )
{
    cudaMalloc((void **)&numbersListDev, numbersSize * sizeof(quint64));
    cudaMalloc((void **)&queryListDev,querySize * sizeof(quint64));
	cudaMalloc((void **)&windowSizesDev, querySize* sizeof(int));
    cudaMemcpy(numbersListDev, numbers, numbersSize * sizeof(quint64), cudaMemcpyHostToDevice);
    cudaMemcpy(queryListDev, query, querySize * sizeof(quint64), cudaMemcpyHostToDevice);
	cudaMemcpy(windowSizesDev, windowSizes, querySize * sizeof(int), cudaMemcpyHostToDevice);
    
    cudaBinarySearch(numbersListDev, numbersSize, queryListDev, querySize, windowSizesDev);
    quint64* results = new quint64[querySize];
    cudaMemcpy(results, queryListDev, querySize * sizeof(quint64), cudaMemcpyDeviceToHost);
    
    cudaFree(numbersListDev); 
    cudaFree(queryListDev); 

    return results;
}

#endif //GA_BUILD_WITH_CUDA


GenomeAlignerCUDAHelper::GenomeAlignerCUDAHelper( )
: readsNumber(0), suffixSize(0), seqSize(0)
{
    bufSizeMB = 1;
}


void GenomeAlignerCUDAHelper::loadShortReads(QVector<SearchQuery*>& queries, TaskStateInfo& stateInfo) {

#ifdef GA_BUILD_WITH_CUDA

    // TODO:
    // perform memory checks
    
    readsNumber = queries.size();
        
    QVector<int> sizes;
    QVector<int> offsets;
    int totalReadsSize = 0;
    
    foreach (SearchQuery* q, queries) {
        int len = q->length();
        sizes.append(len);
        offsets.append(totalReadsSize);
        totalReadsSize += len;
    }
    
    // TODO:
    // set counters

    taskLog.details(QString("Allocation data for short reads on CUDA device"));
    SAFE_CALL( cudaMalloc((void **)&readsDev, totalReadsSize * sizeof(char)) );
    SAFE_CALL( cudaMalloc((void **)&readSizesDev,  readsNumber * sizeof(quint32)) );
    SAFE_CALL( cudaMalloc((void **)&readOffsetsDev, readsNumber * sizeof(quint32)) );
    taskLog.details(QString("Copying %1 bytes of short reads data to device").arg(totalReadsSize*sizeof(char)));
    
    // TODO: change the way reads are loaded into memory: use big data chunks when loading!
    char* buf = NULL; // buffer to speed-up reads transfer to GPU
    int bufSizeBytes = bufSizeMB*1024*1024;
    int curSize = 0, lastOffset = 0;
    cudaError_t err = cudaSuccess;
    
    // It's f**king dangerous!
    try {
        buf = new char[bufSizeBytes];
    } catch(std::bad_alloc e) {
        stateInfo.setError("Can't allocate memory for CUDA buffer.\
                 Try to close some of your programs or to decrease \"maxMemorySize\"-option");
        return;
    }

    for (int i = 0; i < readsNumber; ++i) {
        int readSize = sizes.at(i);
        int offset = offsets.at(i);
        if (curSize + readSize > bufSizeBytes) {
            err = cudaMemcpy(readsDev + lastOffset, buf, curSize * sizeof(char), cudaMemcpyHostToDevice) ;
            lastOffset = offset;
            curSize = 0;
            if (err != cudaSuccess) {
                break;
            }
        }
        qMemCopy(buf + curSize, queries.at(i)->constData(), readSize );
        curSize += readSize;
    }
    
    if (curSize > 0) {
        cudaMemcpy(readsDev + lastOffset, buf, curSize * sizeof(char), cudaMemcpyHostToDevice) ;
    }

    delete[] buf;

    if (err != cudaSuccess) {
        stateInfo.setError(QString("Error while coping short reads to CUDA device: %1")
            .arg(cudaGetErrorString(err) ));
        return;
    }

    taskLog.details(QString("Copying %1 bytes of read sizes data to device").arg(readsNumber*sizeof(quint32)));
    SAFE_CALL( cudaMemcpy(readSizesDev, sizes.data(), readsNumber*sizeof(quint32), cudaMemcpyHostToDevice  ) );

    taskLog.details(QString("Copying %1 bytes of read offsets data to device").arg(readsNumber*sizeof(quint32)));
    SAFE_CALL( cudaMemcpy(readOffsetsDev, offsets.data(), readsNumber*sizeof(quint32), cudaMemcpyHostToDevice  ) );

#else
    Q_UNUSED(queries);
    stateInfo.setError(GenomeAlignerFindTask::tr("The CUDA optimization is not included into this bundle.\
        Please make sure to set UGENE_CUDA_DECTECTED=1 when building UGENE") );
#endif
   
}

void GenomeAlignerCUDAHelper::alignReads(IndexPart& indexPart, AlignContext* ctx, TaskStateInfo& stateInfo) {

#ifdef  GA_BUILD_WITH_CUDA

    suffixSize = indexPart.getLoadedPartSize();
    seqSize = suffixSize + ctx->w - 1;
    assert(readsNumber == ctx->queries.size());
    
    SAFE_CALL( cudaMalloc((void **)&bmArrayDev, suffixSize * sizeof(quint64)));
    SAFE_CALL( cudaMalloc((void **)&sArrayDev, suffixSize * sizeof(quint32)));
    SAFE_CALL( cudaMalloc((void **)&seqDev, seqSize*sizeof(char)));
    SAFE_CALL( cudaMalloc((void **)&resultsDev, 2*readsNumber*sizeof(int)));
    
    taskLog.details(QString("Copying %1 bytes of bit mask array data to device").arg(suffixSize*sizeof(quint64)));
    SAFE_CALL( cudaMemcpy(bmArrayDev, indexPart.bitMask, suffixSize*sizeof(quint64), cudaMemcpyHostToDevice  ) );
    taskLog.details(QString("Copying %1 bytes of suffix array data to device").arg(suffixSize*sizeof(quint32)));
    SAFE_CALL( cudaMemcpy(sArrayDev, indexPart.sArray, suffixSize*sizeof(quint32), cudaMemcpyHostToDevice  ) );
    taskLog.details(QString("Copying %1 bytes of reference sequence to device").arg(seqSize*sizeof(char)));
    SAFE_CALL( cudaMemcpy(seqDev, indexPart.seq , seqSize*sizeof(char), cudaMemcpyHostToDevice  ) );
        
    cudaAlignReads(readsDev,readSizesDev,readOffsetsDev,readsNumber,seqDev,seqSize,
        sArrayDev,suffixSize,bmArrayDev,suffixSize,ctx->bitFilter,indexPart.getLoadedSeqStart(),
        indexPart.getLoadedPartSize(), ctx->nMismatches, ctx->w, resultsDev );
    
    QVector<int> results;
    results.resize(2*readsNumber);
    taskLog.details( QString("Copying %1 bytes of results data to host").arg(2*readsNumber*sizeof(int)) );
    SAFE_CALL( cudaMemcpy(results.data(), resultsDev, 2*readsNumber*sizeof(int), cudaMemcpyDeviceToHost ) );
    
    for (int i = 0; i < readsNumber; ++i) {
        int rPos = results[2*i];
        if (rPos != -1) {
            int mCount = results[2*i+1]; 
            ctx->queries[i]->addResult(rPos, mCount);
        }
    }
    
    SAFE_CALL( cudaFree(resultsDev) ); 
    SAFE_CALL( cudaFree(seqDev) );
    SAFE_CALL( cudaFree(bmArrayDev) );
    SAFE_CALL( cudaFree(sArrayDev) );

#else
    
    Q_UNUSED(indexPart);Q_UNUSED(ctx);
    stateInfo.setError(GenomeAlignerFindTask::tr("The CUDA optimization is not included into this bundle.\
                                            Please make sure to set UGENE_CUDA_DECTECTED=1 when building UGENE") );

#endif


}

qint64 GenomeAlignerCUDAHelper::estimateMemoryUsage()
{
    return 0;
}

GenomeAlignerCUDAHelper::~GenomeAlignerCUDAHelper()
{
#ifdef GA_BUILD_WITH_CUDA
    // is it safe to do it in destructor?
    if (readsNumber != 0) {
        cudaFree(readsDev); 
        cudaFree(readSizesDev);
        cudaFree(readOffsetsDev); 
    }
#endif    
}





} //namespace



