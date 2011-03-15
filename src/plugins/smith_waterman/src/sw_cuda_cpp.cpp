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

#ifdef SW2_BUILD_WITH_CUDA

#include <cuda_runtime.h>
#include <stdlib.h>

#include <U2Core/Log.h>

#include "sw_cuda_cpp.h"

#include <stdio.h>

typedef int ScoreType;

extern QList<resType> calculateOnGPU(const char * seqLib, int seqLibLength, ScoreType* queryProfile, ScoreType qProfLen, int queryLength, ScoreType gapOpen, ScoreType gapExtension, ScoreType maxScore);

static U2::Logger log("Smith Waterman CUDA");

QList<resType> sw_cuda_cpp::launch(const char * seqLib, int seqLibLength, ScoreType* queryProfile, ScoreType qProfLen, int queryLength, ScoreType gapOpen, ScoreType gapExtension, ScoreType maxScore) {

    return calculateOnGPU(seqLib, seqLibLength, queryProfile, qProfLen, queryLength, gapOpen, gapExtension, maxScore);
}

//TODO: calculate maximum alignment length
int calcOverlap(int queryLength) {
    return queryLength * 3;
}

//number of parts of the sequence which we divide
int calcPartsNumber(int seqLibLength, int overlapLength) {
    int partsNumber = (seqLibLength + overlapLength - 1) / overlapLength;

    if (partsNumber > sw_cuda_cpp::MAX_BLOCKS_NUMBER) {
        partsNumber = sw_cuda_cpp::MAX_BLOCKS_NUMBER;
    }
    return partsNumber;
}

//size of sequence's part
int calcPartSeqSize(int seqLibLength, int overlapLength, int partsNumber) {
    return (seqLibLength + (partsNumber - 1) * (overlapLength + 1)) / partsNumber;
}

//size of vector that contain all results
int calcSizeRow(int seqLibLength, int overlapLength, int partsNumber, int partSeqSize) {
    return (partSeqSize + 1) * partsNumber;
}


quint64 sw_cuda_cpp::estimateNeededGpuMemory( int seqLibLength, ScoreType qProfLen, int queryLength ) {

    int sizeP = qProfLen * sizeof(ScoreType);
    int sizeL = (seqLibLength) * sizeof(char);

    const int overlapLength = calcOverlap(queryLength);
    int partsNumber = calcPartsNumber(seqLibLength, overlapLength);
    int partSeqSize = calcPartSeqSize(seqLibLength, overlapLength, partsNumber);
    int sizeRow = calcSizeRow(seqLibLength, overlapLength, partsNumber, partSeqSize);

    int sizeN = 7 * sizeRow * sizeof(ScoreType);

    quint64 memToAlloc = sizeL + sizeP + sizeN; //see cudaMallocs in sw_cuda.cu for details
    return memToAlloc * 1.2; //just for safety
}

//IMPORTANT: these settings depend on the video card
//TODO: develop logic for calculation this settings
const int sw_cuda_cpp::MAX_BLOCKS_NUMBER = 14;
//we have 3 shared vector, this mean all shared memory = MAX_SHARED_VECTOR_LENGTH * 3
const int sw_cuda_cpp::MAX_SHARED_VECTOR_LENGTH = 128;

//__global__ 
extern void calculateMatrix_wrap(int blockSize, int threadNum, const char * seqLib, ScoreType* queryProfile,                                       
                                 ScoreType* g_HdataUp, ScoreType* g_HdataRec, ScoreType* g_HdataMax,
                                 ScoreType* g_FdataUp,
                                 ScoreType* g_directionsUp, ScoreType* g_directionsRec, ScoreType* g_directionsMax,
                                 int iteration);

extern void setConstants(int partSeqSize, int partsNumber, int overlapLength, int seqLibLength, int queryLength, int gapOpen, int gapExtension, int maxScore, int queryPartLength);

QList<resType> calculateOnGPU(const char * seqLib, int seqLibLength, ScoreType* queryProfile, ScoreType qProfLen, int queryLength, ScoreType gapOpen, ScoreType gapExtension, ScoreType maxScore) {

    //TODO: calculate maximum alignment length
    const int overlapLength = calcOverlap(queryLength);

    int partsNumber = calcPartsNumber(seqLibLength, overlapLength);

    int queryDevider = 1;
    if (queryLength > sw_cuda_cpp::MAX_SHARED_VECTOR_LENGTH) {
        queryDevider = (queryLength + sw_cuda_cpp::MAX_SHARED_VECTOR_LENGTH - 1) / sw_cuda_cpp::MAX_SHARED_VECTOR_LENGTH;
    }

    int partQuerySize = (queryLength + queryDevider - 1) / queryDevider;

    int partSeqSize = calcPartSeqSize(seqLibLength, overlapLength, partsNumber);

    int sizeRow = calcSizeRow(seqLibLength, overlapLength, partsNumber, partSeqSize);

    log.details(QString("partsNumber: %1 queryDevider: %2").arg(partsNumber).arg(queryDevider));

    log.details(QString("seqLen: %1 partSeqSize: %2 overlapSize: %3").arg(seqLibLength).arg(partSeqSize).arg(overlapLength));
    log.details(QString("queryLen %1 partQuerySize: %2").arg(queryLength).arg(partQuerySize));

    //************************** declare some temp variables on host

    ScoreType* tempRow = new ScoreType[sizeRow];
    ScoreType* zerroArr = new ScoreType[sizeRow];
    for (int i = 0; i < sizeRow; i++) {
        zerroArr[i] = 0;
    }

    ScoreType* directionRow = new ScoreType[sizeRow];

    //************************** sizes of arrays

    size_t sizeQ = sizeRow * sizeof(ScoreType);
    size_t sizeQQ = (sizeRow) * sizeof(ScoreType);
    size_t sizeP = qProfLen * sizeof(ScoreType);
    size_t sizeL = (seqLibLength) * sizeof(char);

    //************************** declare arrays on device

    char * g_seqLib;
    ScoreType* g_queryProfile;
    ScoreType* g_HdataMax;
    ScoreType* g_HdataUp;
    ScoreType* g_HdataRec;
    ScoreType* g_HdataTmp;
    ScoreType* g_FdataUp;
    ScoreType* g_directionsUp;
    ScoreType* g_directionsMax;
    ScoreType* g_directionsRec;

    //************************** allocate global memory on device

    cudaMalloc((void **)& g_seqLib, sizeL);
    cudaMalloc((void **)& g_queryProfile, sizeP);
    cudaMalloc((void **)& g_HdataMax, sizeQ);
    cudaMalloc((void **)& g_HdataUp, sizeQ);
    cudaMalloc((void **)& g_FdataUp, sizeQ);
    cudaMalloc((void **)& g_directionsUp, sizeQ);
    cudaMalloc((void **)& g_directionsMax, sizeQ);
    cudaMalloc((void **)& g_HdataRec, sizeQ);
    cudaMalloc((void **)& g_directionsRec, sizeQ);

    log.details(QString("GLOBAL MEMORY USED %1 KB").arg((sizeL + sizeP + sizeQ * 7) / 1024));

    //************************** copy from host to device

    cudaMemcpy(g_seqLib, seqLib, sizeL, cudaMemcpyHostToDevice);
    cudaMemcpy(g_queryProfile, queryProfile, sizeP, cudaMemcpyHostToDevice);
    cudaMemcpy(g_HdataMax, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_HdataUp, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_FdataUp, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_directionsUp, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_directionsMax, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_directionsRec, zerroArr, sizeQ, cudaMemcpyHostToDevice);
    cudaMemcpy(g_HdataRec, zerroArr, sizeQ, cudaMemcpyHostToDevice);

    //************************** start calculation

    int BLOCK_SIZE = partsNumber;

    dim3 dimBlock(BLOCK_SIZE);
    dim3 dimGrid(partQuerySize);

    //move constants variables to constant cuda memory
    setConstants(partSeqSize, partsNumber, overlapLength, seqLibLength, queryLength, gapOpen, gapExtension, maxScore, partQuerySize);

    size_t sh_mem_size = sizeof(ScoreType) * (dimGrid.x + 1) * 3;
    log.details(QString("SHARED MEM SIZE USED: %1 B").arg(sh_mem_size));
    // start main loop
    for (int i = 0; i < queryDevider; i++) {

        calculateMatrix_wrap( dimBlock.x, dimGrid.x, g_seqLib,
            g_queryProfile, g_HdataUp, g_HdataRec, g_HdataMax,
            g_FdataUp, g_directionsUp, g_directionsRec,
            g_directionsMax, i * partQuerySize);

        cudaError hasErrors = cudaThreadSynchronize();

        if (hasErrors != 0) {
            log.trace(QString("CUDA ERROR HAPPEN, errorId: ") + QString::number(hasErrors));
        }

//        cudaMemcpy(tempRow, g_HdataMax, sizeQQ, cudaMemcpyDeviceToHost);
//         QString str1 = "";
//         int maximumScore = 0;
//         if (i == 0) {
//         for (int j = 0; j < 26; j++) {
//             if (j % (partSeqSize + 1) == 0 && j != 0) {
//                 str1 += "|| ";
//             }
//             str1 += QString().sprintf("%2d", j) + " ";
//         }
//         log.details(str1);
//         log.details("**************");
//         }

        //revert arrays
        g_HdataTmp = g_HdataRec;
        g_HdataRec = g_HdataUp;
        g_HdataUp = g_HdataTmp;

        g_HdataTmp = g_directionsRec;
        g_directionsRec = g_directionsUp;
        g_directionsUp = g_HdataTmp;
    }

    //Copy vectors on host and find actual results
    cudaMemcpy(tempRow, g_HdataMax, sizeQQ, cudaMemcpyDeviceToHost);
    cudaMemcpy(directionRow, g_directionsMax, sizeQQ, cudaMemcpyDeviceToHost);

    QList<resType> pas;
    resType res;
    for (int j = 0; j < (sizeRow); j++) {
        if (tempRow[j] >= maxScore) {
            res.reg.startPos = directionRow[j];
            res.reg.length = j - res.reg.startPos + 1 - (j) / (partSeqSize + 1) * overlapLength - (j) / (partSeqSize + 1);
            res.score = tempRow[j];
//             log.info(QString("score: %1, reg: %2..%3").arg(res.score).arg(res.reg.startPos).arg(res.reg.endPos()));
            pas.append(res);
        }
    }

    //deallocation memory
    cudaFree(g_seqLib);
    cudaFree(g_queryProfile);
    cudaFree(g_HdataMax);
    cudaFree(g_HdataUp);
    cudaFree(g_HdataRec);
    cudaFree(g_FdataUp);
    cudaFree(g_directionsUp);
    cudaFree(g_directionsMax);
    cudaFree(g_directionsRec);

    delete[] tempRow;
    delete[] directionRow;
    delete[] zerroArr;

    return pas;
}


#endif //SW2_BUILD_WITH_CUDA
