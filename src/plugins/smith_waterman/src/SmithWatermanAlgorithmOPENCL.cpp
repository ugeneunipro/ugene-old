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

#ifdef SW2_BUILD_WITH_OPENCL

#include "SmithWatermanAlgorithmOPENCL.h"
#include "PairAlignSequences.h"

#include <U2Core/Log.h>
#include <U2Core/GAutoDeleteList.h>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <U2Algorithm/OpenCLUtils.h>
#include <U2Algorithm/OpenCLHelper.h>

const double B_TO_MB_FACTOR = 1048576.0;

namespace U2 {

int SmithWatermanAlgorithmOPENCL::MAX_BLOCKS_NUMBER = 0;
const int SmithWatermanAlgorithmOPENCL::MAX_SHARED_VECTOR_LENGTH = 32;

SmithWatermanAlgorithmOPENCL::SmithWatermanAlgorithmOPENCL()  :
        clEvent(NULL), clKernel(NULL), clProgram(NULL),
        clCommandQueue(NULL), clContext(NULL), queryProfBuf(NULL),
        seqLibProfBuf(NULL), hDataMaxBuf(NULL), hDataUpBufTmp(NULL),
        hDataRecBufTmp(NULL), fDataUpBuf(NULL), directionsUpBufTmp(NULL),
        directionsRecBufTmp(NULL), directionsMaxBuf(NULL)
{}

quint64 SmithWatermanAlgorithmOPENCL::estimateNeededGpuMemory(const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq) {
    const quint64 queryLength = _patternSeq.size();
    const quint64 searchLen = _searchSeq.size();
    const quint64 subLen = sm.getAlphabet()->getNumAlphabetChars();
    const QByteArray & alphChars = sm.getAlphabet()->getAlphabetChars();
    const quint64 profLen = subLen * (queryLength + 1) * (alphChars[ alphChars.size()-1 ] + 1);

    const quint64 overlapLength = queryLength * 3;
    const quint64 partsNumber = calcPartsNumber(searchLen, overlapLength);
    const quint64 partSeqSize = calcPartSeqSize(searchLen, overlapLength, partsNumber);
    const quint64 sizeRow = calcSizeRow(partsNumber, partSeqSize);

    const quint64 memToAlloc = sizeof(char) * (_searchSeq.length() + 1) + profLen * sizeof(cl_int) + sizeof(ScoreType) * sizeRow * 7;

    return memToAlloc * 8 / 5; //factor 8/5 is used because OpenCL won't allocate all or almost all available GPU memory
}

quint64 SmithWatermanAlgorithmOPENCL::estimateNeededRamAmount( const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq ) {
    const quint64 queryLength = _patternSeq.size();
    const quint64 searchLen = _searchSeq.size();
    const quint64 subLen = sm.getAlphabet()->getNumAlphabetChars();
    const QByteArray & alphChars = sm.getAlphabet()->getAlphabetChars();
    const quint64 profLen = subLen * (queryLength + 1) * (alphChars[ alphChars.size()-1 ] + 1);

    const quint64 overlapLength = queryLength * 3;
    const quint64 partsNumber = calcPartsNumber(searchLen, overlapLength);
    const quint64 partSeqSize = calcPartSeqSize(searchLen, overlapLength, partsNumber);
    const quint64 sizeRow = calcSizeRow(partsNumber, partSeqSize);
    
    const quint64 memToAlloc = 2 * sizeof(ScoreType) * sizeRow + sizeof(ScoreType) * profLen;

    return memToAlloc;
}

//TODO: calculate maximum alignment length
int SmithWatermanAlgorithmOPENCL::calcOverlap(int queryLength) {
    return queryLength * 2;
}

//number of parts of the sequence which we divide
int SmithWatermanAlgorithmOPENCL::calcPartsNumber(int searchLen, int overlapLength) {
    if(0 == MAX_BLOCKS_NUMBER) {
        MAX_BLOCKS_NUMBER = AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu()->getMaxComputeUnits();
    }

    int partsNumber = (searchLen + overlapLength - 1) / overlapLength;

    if (partsNumber > MAX_BLOCKS_NUMBER) {
        partsNumber = MAX_BLOCKS_NUMBER;
    }
    return partsNumber;
}

//size of sequence's part
int SmithWatermanAlgorithmOPENCL::calcPartSeqSize(int searchLen, int overlapLength, int partsNumber) {
    return (searchLen + (partsNumber - 1) * (overlapLength + 1)) / partsNumber;
}

//size of vector that contain all results
int SmithWatermanAlgorithmOPENCL::calcSizeRow(int partsNumber, int partSeqSize) {
    return (partSeqSize + 1) * partsNumber;
}

bool hasOPENCLError(int err, QString errorMessage) {
    if(err != 0) {
        algoLog.error(QString("OPENCL: %1; Error code (%2)").arg(errorMessage).arg(err));
        return true;
    } else {
        return false;
    }
}

void SmithWatermanAlgorithmOPENCL::launch(const SMatrix& sm, QByteArray const & _patternSeq, QByteArray const & _searchSeq, int _gapOpen, int _gapExtension, int _minScore) {

    setValues(sm, _patternSeq, _searchSeq, _gapOpen, _gapExtension, _minScore);
    algoLog.details(QObject::tr("START SmithWatermanAlgorithmOPENCL::launch"));

    int queryLength = patternSeq.size();
    int searchLen = searchSeq.size();
    int subLen = sm.getAlphabet()->getNumAlphabetChars();

    //alphChars is sorted
    const QByteArray & alphChars = sm.getAlphabet()->getAlphabetChars();
    quint64 profLen = subLen * (queryLength + 1) * (alphChars[ alphChars.size()-1 ] + 1);
    ScoreType *  queryProfile = NULL;
    queryProfile = new ScoreType[profLen];

    gauto_array<ScoreType> qpm(queryProfile); // to ensure the data is deallocated correctly

    for (int i = 0; i < profLen; i++) {
        queryProfile[i] = 0;
    }

    //calculate query profile
    for (int i = 0; i < subLen; i++) {
        for (int j = 0; j < queryLength; j++) {
            char ch = alphChars[i];
            queryProfile[ch * queryLength + j] = sm.getScore(ch, _patternSeq.at(j));
        }        
    }

    cl_int err = CL_SUCCESS;

    cl_uint clNumDevices = 1;
    cl_device_id deviceId = (cl_device_id) AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu()->getId();

    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();
    if (!openCLHelper.isLoaded()) {
        coreLog.error(openCLHelper.getErrorString());
        return;
    }

    algoLog.trace("Creating a context");

    clContext = openCLHelper.clCreateContext_p(0, clNumDevices, &deviceId, NULL, NULL, &err);
    if (hasOPENCLError(err, "cl::Context() failed")) return;

    //*******************************
    //calculate lengths
    const int overlapLength = queryLength * 3;

    int partsNumber = calcPartsNumber(searchLen, overlapLength);

    int queryDevider = 1;
    if (queryLength > MAX_SHARED_VECTOR_LENGTH) {
        queryDevider = (queryLength + MAX_SHARED_VECTOR_LENGTH - 1) / MAX_SHARED_VECTOR_LENGTH;
    }

    int partQuerySize = (queryLength + queryDevider - 1) / queryDevider;

    int partSeqSize = calcPartSeqSize(searchLen, overlapLength, partsNumber);

    quint64 sizeRow = calcSizeRow(partsNumber, partSeqSize);

    ScoreType* g_HdataTmp = NULL;
    ScoreType* g_directionsRec = NULL;

    g_HdataTmp = new ScoreType[sizeRow];
    g_directionsRec = new ScoreType[sizeRow];

    gauto_array<ScoreType> g_HdataTmpPtr(g_HdataTmp);
    gauto_array<ScoreType> g_directionsRecPtr(g_directionsRec);

    for (int i = 0; i < sizeRow; i++) {
        g_HdataTmp[i] = 0;
        g_directionsRec[i] = 0;
    }

    queryProfBuf = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(cl_int) * profLen, queryProfile, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(cl_int) * profLen / B_TO_MB_FACTOR)))) return;

    seqLibProfBuf = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(char) * (_searchSeq.length() + 1), searchSeq.data(), &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(char) * (_searchSeq.length() + 1) / B_TO_MB_FACTOR)))) return;

    hDataMaxBuf = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_HdataTmp, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;

    hDataUpBufTmp = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_HdataTmp, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;
    cl_mem* hDataUpBuf = &hDataUpBufTmp;

    hDataRecBufTmp = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_HdataTmp, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;
    cl_mem* hDataRecBuf = &hDataRecBufTmp;

    cl_mem* hDataTmpBuf = NULL;

    fDataUpBuf = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_HdataTmp, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;

    directionsUpBufTmp = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_directionsRec, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;
    cl_mem* directionsUpBuf = &directionsUpBufTmp;

    directionsRecBufTmp = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_directionsRec, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;
    cl_mem* directionsRecBuf = &directionsRecBufTmp;

    directionsMaxBuf = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
        sizeof(ScoreType) * (sizeRow), g_directionsRec, &err);
    if (hasOPENCLError(err, QString("Can't allocate %1 MB memory in GPU buffer").arg(QString::number(sizeof(ScoreType) * (sizeRow) / B_TO_MB_FACTOR)))) return;

    algoLog.trace(QString("partsNumber: %1 queryDevider: %2").arg(partsNumber).arg(queryDevider));
    algoLog.trace(QString("seqLen: %1 partSeqSize: %2 overlapSize: %3").arg(searchLen).arg(partSeqSize).arg(overlapLength));
    algoLog.trace(QString("queryLen %1 partQuerySize: %2 sizeRow: %3").arg(queryLength).arg(partQuerySize).arg(sizeRow));

    //open and read file contains OPENCL code
    clProgram = OpenCLUtils::createProgramByResource(clContext, deviceId, ":src/sw_opencl.cl", openCLHelper, err);
    if (hasOPENCLError(err, "createProgramByResource() failed")) return;

    clKernel = openCLHelper.clCreateKernel_p(clProgram, "calculateMatrix", &err);
    if (hasOPENCLError(err, "Kernel::Kernel() failed")) return;

    //************begin: set arguments****************

    int n = 0;

    //0: seqLib
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&seqLibProfBuf);

    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //1: queryProfile
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&queryProfBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //2: g_HdataUp
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)hDataUpBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //3: g_HdataRec
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)hDataRecBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //4: g_HdataMax
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&hDataMaxBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //5: g_FdataUp
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&fDataUpBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;


    //6: g_directionsUp
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)directionsUpBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //7: g_directionsRec
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)directionsRecBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //8: g_directionsMax
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&directionsMaxBuf);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //9: queryStartPos
    cl_int queryStartPos = 0;
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&queryStartPos);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //10: partSeqSize
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&partSeqSize);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //11: partsNumber
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&partsNumber);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //12: overlapLength
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&overlapLength);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //13: searchLen
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&searchLen);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //14: queryLength
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&queryLength);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //15: gapOpen
    cl_int clGapOpen = -1 * gapOpen;
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&clGapOpen);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //16: gapExtension
    cl_int clGapExtension = -1 * gapExtension;
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&clGapExtension);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //17: queryPartLength
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&partQuerySize);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //18: shared_E
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int) * partQuerySize, NULL);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //19: shared_direction
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int) * partQuerySize, NULL);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //20: shared_direction
    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int) * partQuerySize, NULL);
    if (hasOPENCLError(err, QObject::tr("Kernel::setArg(%1) failed").arg(n))) return;

    //************end: set arguments****************

    clCommandQueue = openCLHelper.clCreateCommandQueue_p(clContext, deviceId, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    if (hasOPENCLError(err, "cl::CommandQueue() failed ")) return;

    coreLog.details(QObject::tr("OPENCL: Running CL program"));

    //run kernel
    const size_t szGlobalWorkSize = partsNumber * partQuerySize;
    const size_t szLocalWorkSize = partQuerySize;

    for (int i = 0; i < queryDevider; i++) {
        err = openCLHelper.clEnqueueNDRangeKernel_p(
            clCommandQueue, 
            clKernel, 
            1, 
            NULL, 
            &szGlobalWorkSize, 
            &szLocalWorkSize, 
            0, 
            NULL, 
            &clEvent);
        if (hasOPENCLError(err, "CommandQueue::enqueueNDRangeKernel() failed")) return;

        err = openCLHelper.clWaitForEvents_p(1, &clEvent);
        if (hasOPENCLError(err, "clWaitForEvents failed")) return;

        if (clEvent) {
            err = openCLHelper.clReleaseEvent_p (clEvent);
            if (hasOPENCLError(err, "clReleaseEvent 1 failed")) return;
        }

        //revert arrays
        hDataTmpBuf = hDataRecBuf;
        hDataRecBuf = hDataUpBuf;
        hDataUpBuf = hDataTmpBuf;

        hDataTmpBuf = directionsRecBuf;
        directionsRecBuf = directionsUpBuf;
        directionsUpBuf = hDataTmpBuf;

        //g_HdataUp
        err = openCLHelper.clSetKernelArg_p(clKernel, 2, sizeof(cl_mem), (void*)hDataUpBuf);
        if (hasOPENCLError(err, "Kernel::setArg(2) failed")) return;

        //g_HdataRec
        err = openCLHelper.clSetKernelArg_p(clKernel, 3, sizeof(cl_mem), (void*)hDataRecBuf);
        if (hasOPENCLError(err, "Kernel::setArg(3) failed")) return;

        //g_directionsUp
        err = openCLHelper.clSetKernelArg_p(clKernel, 6, sizeof(cl_mem), (void*)directionsUpBuf);
        if (hasOPENCLError(err, "Kernel::setArg(6) failed")) return;

        //g_directionsRec
        err = openCLHelper.clSetKernelArg_p(clKernel, 7, sizeof(cl_mem), (void*)directionsRecBuf);
        if (hasOPENCLError(err, "Kernel::setArg(7) failed")) return;

        //queryStartPos
        queryStartPos = (i+1) * partQuerySize;
        err = openCLHelper.clSetKernelArg_p(clKernel, 9, sizeof(cl_int), (void*)&queryStartPos);
        if (hasOPENCLError(err, "Kernel::setArg(9) failed")) return;
    }


    //copy from platform to host
    err = openCLHelper.clEnqueueReadBuffer_p(clCommandQueue, hDataMaxBuf, CL_FALSE, 0, sizeof(cl_int) * (sizeRow), g_HdataTmp, 0, NULL, &clEvent);
    if (hasOPENCLError(err, "clEnqueueReadBuffer failed")) return;

    err = openCLHelper.clWaitForEvents_p(1, &clEvent);
    if (hasOPENCLError(err, "clWaitForEvents failed")) return;

    if (clEvent) {
        err = openCLHelper.clReleaseEvent_p (clEvent);
        if (hasOPENCLError(err, "clReleaseEvent 2 failed")) return;
    }

    err = openCLHelper.clEnqueueReadBuffer_p(clCommandQueue, directionsMaxBuf, CL_FALSE, 0, sizeof(ScoreType) * (sizeRow), g_directionsRec, 0, NULL, &clEvent);
    if (hasOPENCLError(err, "clEnqueueReadBuffer failed")) return;

    err = openCLHelper.clWaitForEvents_p(1, &clEvent);
    if (hasOPENCLError(err, "clWaitForEvents failed")) return;

    if (clEvent) {
        err = openCLHelper.clReleaseEvent_p (clEvent);
        if (hasOPENCLError(err, "clReleaseEvent 3 failed")) return;
    }

    err = openCLHelper.clFinish_p(clCommandQueue);
    if (hasOPENCLError(err, "clFinish failed")) return;


    // 	   cout <<"after: " <<endl;	
    //        for (int i = 0; i < sizeRow; i++) {
    //            cout <<g_HdataTmp[i] <<" ";
    //        }
    //        cout <<endl;
    //        cout <<"directions: " <<endl;
    //        for (int i = 0; i < sizeRow; i++) {
    //            cout <<g_directionsRec[i] <<" ";
    //        }
    //        cout <<endl;

    //collect results
    PairAlignSequences tmp;
    for (int j = 0; j < (sizeRow); j++) {
        if (g_HdataTmp[j] >= minScore) {
            tmp.intervalSeq1.startPos = g_directionsRec[j];
            tmp.intervalSeq1.length = j - tmp.intervalSeq1.startPos + 1 - (j) / (partSeqSize + 1) * overlapLength - (j) / (partSeqSize + 1);
            tmp.score = g_HdataTmp[j];
            //                        cout <<"score: " <<tmp.score <<" reg: " <<tmp.intervalSeq1.startPos <<".." <<tmp.intervalSeq1.endPos() <<endl;
            pairAlignmentStrings.append(tmp);
        }
    }

    algoLog.details(QObject::tr("FINISH SmithWatermanAlgorithmOPENCL::launch"));
}

SmithWatermanAlgorithmOPENCL::~SmithWatermanAlgorithmOPENCL() {
    algoLog.details(QObject::tr("Starting cleanup OpenCL resources"));

    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();

    cl_int err = CL_SUCCESS;

    if (clKernel) {
        err = openCLHelper.clReleaseKernel_p (clKernel);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clProgram)  {
        err = openCLHelper.clReleaseProgram_p(clProgram);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clCommandQueue) {
        err = openCLHelper.clReleaseCommandQueue_p (clCommandQueue);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clContext) {
        err = openCLHelper.clReleaseContext_p(clContext);
        hasOPENCLError(err, "clReleaseEvent failed");
    }

    if (queryProfBuf) {
        err = openCLHelper.clReleaseMemObject_p (queryProfBuf);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (seqLibProfBuf) {
        err = openCLHelper.clReleaseMemObject_p (seqLibProfBuf);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (hDataMaxBuf) {
        err = openCLHelper.clReleaseMemObject_p (hDataMaxBuf);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (hDataUpBufTmp) {
        err = openCLHelper.clReleaseMemObject_p (hDataUpBufTmp);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (hDataRecBufTmp) {
        err = openCLHelper.clReleaseMemObject_p (hDataRecBufTmp);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (fDataUpBuf) {
        err = openCLHelper.clReleaseMemObject_p (fDataUpBuf);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (directionsUpBufTmp) {
        err = openCLHelper.clReleaseMemObject_p (directionsUpBufTmp);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (directionsRecBufTmp) {
        err = openCLHelper.clReleaseMemObject_p (directionsRecBufTmp);
        hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (directionsMaxBuf) {
        err = openCLHelper.clReleaseMemObject_p (directionsMaxBuf);
        hasOPENCLError(err, "clReleaseEvent failed");
    }

    algoLog.details(QObject::tr("Finish cleanup OpenCL resources"));
}

} //namespace

#endif //SW2_BUILD_WITH_OPENCL
