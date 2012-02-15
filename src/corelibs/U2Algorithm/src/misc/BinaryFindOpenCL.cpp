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

#ifdef OPENCL_SUPPORT

#include "BinaryFindOpenCL.h"

#include <U2Core/Log.h>
#include <U2Core/GAutoDeleteList.h>

#include <U2Algorithm/OpenCLUtils.h>

#include <QtCore/QString>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#include <ctime>

namespace U2 {

NumberType* BinaryFindOpenCL::launch() {

    time_t time1 = time(NULL);

    size_t usageGPUMem = 0;
    size_t usageGPUConstantMem = 0;

    const int lowerBound = 0;
    const int uperBound = numbersSize;

    cl_int err = CL_SUCCESS;

    int presaveDepthNum = 2000;

    if (numbersSize < presaveDepthNum) {
        presaveDepthNum = numbersSize;
    }

    NumberType* preSaveBounds = new NumberType[presaveDepthNum];
    gauto_array<NumberType> preSaveBoundsPtr(preSaveBounds);

    NumberType* preSaveValues = new NumberType[presaveDepthNum];
    gauto_array<NumberType> preSaveValuesPtr(preSaveValues);

    prepareBinarySearch(numbers, lowerBound, uperBound, preSaveBounds, preSaveValues, presaveDepthNum - 1);

    cl_uint clNumDevices = 1;
    cl_device_id deviceId = (cl_device_id) AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu()->getId();

    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();
    if (!openCLHelper.isLoaded()) {
        coreLog.error(openCLHelper.getErrorString());
        return 0;
    }
    algoLog.trace("Creating a context");

    clContext = openCLHelper.clCreateContext_p(0, clNumDevices, &deviceId, NULL, NULL, &err);
    if (hasOPENCLError(err, "clContext() failed")) return 0;

    //open and read file contains OPENCL code
    clProgram = OpenCLUtils::createProgramByResource(clContext, deviceId, ":src/util_gpu/opencl/BinaryFind.cl", openCLHelper, err);
    if (hasOPENCLError(err, "createProgramByResource() failed")) return 0;

    clKernel = openCLHelper.clCreateKernel_p(clProgram, "binarySearch_classic", &err);
    hasOPENCLError(err, "clCreateKernel");

    int iterationsNum = 20000;
    if (numbersSize < iterationsNum) {
        iterationsNum = numbersSize / 2 + 1;
    }

    size_t localWorkSize = 32;
    if (findNumbersSize < localWorkSize) {
        localWorkSize = findNumbersSize;
    }

    size_t blocksNum = (findNumbersSize + iterationsNum - 1) / iterationsNum;
    blocksNum = (blocksNum + localWorkSize - 1) / localWorkSize;
    const size_t globalWorkSize = blocksNum * localWorkSize;
    algoLog.details(QObject::tr("blocks: %1 globalWorkSize %2 localWorkSize %3 iterationsNum %4").arg(blocksNum).arg(globalWorkSize).arg(localWorkSize).arg(iterationsNum));

    buf_findMeArray = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * findNumbersSize, (void*)findNumbers, &err);
    if (hasOPENCLError(err, "buf_findMeArray clCreateBuffer")) return 0;
    usageGPUMem += sizeof(NumberType) * findNumbersSize;

    const int outputArraySize = findNumbersSize;

    buf_sortedArray = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * numbersSize, (void*)numbers, &err);
    if (hasOPENCLError(err, "buf_sortedArray clCreateBuffer")) return 0;
    usageGPUMem += sizeof(NumberType) * numbersSize;

    buf_preSaveBounds = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * presaveDepthNum, (void*)preSaveBounds, &err);
    if (hasOPENCLError(err, "buf_preSaveBounds clCreateBuffer")) return 0;
    usageGPUMem += sizeof(NumberType) * presaveDepthNum;

    buf_preSaveValues = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * presaveDepthNum, (void*)preSaveValues, &err);
    if (hasOPENCLError(err, "buf_preSaveValues clCreateBuffer")) return 0;
    usageGPUConstantMem += sizeof(NumberType) * presaveDepthNum;

    cl_uint n = 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&buf_sortedArray);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&buf_findMeArray);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&findNumbersSize);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&lowerBound);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&uperBound);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&iterationsNum);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&buf_preSaveBounds);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_mem), (void*)&buf_preSaveValues);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(cl_int), (void*)&presaveDepthNum);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(NumberType) * presaveDepthNum, NULL);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, n++, sizeof(NumberType), (void*)&filter);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(n))) return 0;

    clCommandQueue = openCLHelper.clCreateCommandQueue_p(clContext, deviceId, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    if (hasOPENCLError(err, "cl::CommandQueue() failed ")) return 0;

    err = openCLHelper.clEnqueueNDRangeKernel_p(
                    clCommandQueue,
                    clKernel,
                    1,
                    NULL,
                    &globalWorkSize,
                    &localWorkSize,
                    0,
                    NULL,
                    &clEvent1);
    if (hasOPENCLError(err, "clEnqueueNDRangeKernel")) return 0;

    err = openCLHelper.clWaitForEvents_p(1, &clEvent1);
    if (hasOPENCLError(err, "clWaitForEvents 1")) return 0;

    if (clEvent1) {
        err = openCLHelper.clReleaseEvent_p (clEvent1);
        if (hasOPENCLError(err, "clReleaseEvent 1 failed")) return 0;
    }

    NumberType* outputArray = new NumberType[outputArraySize];
    err = openCLHelper.clEnqueueReadBuffer_p(clCommandQueue, buf_findMeArray, CL_FALSE, 0, sizeof(NumberType) * outputArraySize, outputArray, 0, NULL, &clEvent2);
    if (hasOPENCLError(err, "clCommandQueue")) {
        delete[] outputArray;
        return 0;
    }

    err = openCLHelper.clWaitForEvents_p(1, &clEvent2);
    if (hasOPENCLError(err, "clWaitForEvents")) {
        delete[] outputArray;
        return 0;
    }

    if (clEvent2) {
        err = openCLHelper.clReleaseEvent_p (clEvent2);
        if (hasOPENCLError(err, "clReleaseEvent 2 failed")) return 0;
    }

    time_t time2 = time(NULL);
    algoLog.details(QObject::tr("GPU execution time: %1 GPUMem ussage: %2 Mb GPUContantMem %3 Kb")
                 .arg((time2 - time1))
                 .arg(usageGPUMem / 1024 / 1024)
                 .arg(usageGPUConstantMem / 1024));

    return outputArray;
}

void BinaryFindOpenCL::prepareBinarySearch(const NumberType* arr, int lowerBound, int upperBound,
                         NumberType* resBounds, NumberType* resValues, const int depthNum) {

    if (depthNum != 0) {
        const int step = (upperBound - lowerBound)  / (depthNum);
        for (int i = 1; i < depthNum; i++) {
            resBounds[i] = step * i;
            resValues[i] = arr[resBounds[i]];
        }
    }
    resBounds[0] = lowerBound;
    resValues[0] = arr[resBounds[0]];
    resBounds[depthNum] = upperBound - 1;
    resValues[depthNum] = arr[resBounds[depthNum]];
}

BinaryFindOpenCL::~BinaryFindOpenCL() {
    algoLog.details(QObject::tr("clear OpenCL resources"));
    cl_int err = CL_SUCCESS;
    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();

    if (clKernel) {
            err = openCLHelper.clReleaseKernel_p(clKernel);
            hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clProgram)  {
            err = openCLHelper.clReleaseProgram_p(clProgram);
            hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clCommandQueue) {
            err = openCLHelper.clReleaseCommandQueue_p(clCommandQueue);
            hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (clContext) {
            err = openCLHelper.clReleaseContext_p(clContext);
            hasOPENCLError(err, "clReleaseEvent failed");
    }
    if (buf_sortedArray) {
            err = openCLHelper.clReleaseMemObject_p(buf_sortedArray);
            hasOPENCLError(err, "clReleaseMemObject failed");
    }
    if (buf_findMeArray) {
            err = openCLHelper.clReleaseMemObject_p(buf_findMeArray);
            hasOPENCLError(err, "clReleaseMemObject failed");
    }
    if (buf_outPutArray) {
            err = openCLHelper.clReleaseMemObject_p(buf_outPutArray);
            hasOPENCLError(err, "clReleaseMemObject failed");
    }
    if (buf_preSaveBounds) {
            err = openCLHelper.clReleaseMemObject_p(buf_preSaveBounds);
            hasOPENCLError(err, "clReleaseMemObject failed");
    }
    if (buf_preSaveValues) {
            err = openCLHelper.clReleaseMemObject_p(buf_preSaveValues);
            hasOPENCLError(err, "clReleaseMemObject failed");
    }
}

BinaryFindOpenCL::BinaryFindOpenCL(const NumberType *_numbers,
                           const int _numbersSize,
                           const NumberType *_findNumbers,
                           const int _findNumbersSize,
                           const NumberType _filter) :
        numbers(_numbers),
        numbersSize(_numbersSize),
        findNumbers(_findNumbers),
        findNumbersSize(_findNumbersSize),
        filter(_filter) {
    clEvent1 = 0;
    clEvent2 = 0;
    clKernel = 0;
    clProgram = 0;
    clCommandQueue = 0;
    clContext = 0;
    buf_sortedArray = 0;
    buf_findMeArray = 0;
    buf_outPutArray = 0;
    buf_preSaveBounds = 0;
    buf_preSaveValues = 0;
}

bool BinaryFindOpenCL::hasOPENCLError(int err, QString errorMessage) {
    if(err != 0) {
        algoLog.error(QString("OPENCL: %1; Error code (%2)").arg(errorMessage).arg(err));
        return true;
    } else {
        return false;
    }
}

}//namespace

#endif /*OPENCL_SUPPORT*/