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
#include <math.h>

namespace U2 {

NumberType* BinaryFindOpenCL::launch() {

    time_t time1 = time(NULL);

    size_t usageGPUMem = 0;
    size_t usageGPUConstantMem = 0;

    const int lowerBound = 0;
    const int upperBound = haystackSize;

    cl_int err = CL_SUCCESS;

	// the number of needles a particular kernel execution should search for
    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();
    if (!openCLHelper.isLoaded()) {
        coreLog.error(openCLHelper.getErrorString());
        return 0;
    }

	cl_uint clNumDevices = 1;
	cl_device_id deviceId = (cl_device_id) AppContext::getOpenCLGpuRegistry()->getAnyEnabledGpu()->getId();

    algoLog.trace("Creating a context");
    clContext = openCLHelper.clCreateContext_p(0, clNumDevices, &deviceId, NULL, NULL, &err);
    if (hasOPENCLError(err, "clContext() failed")) return 0;

    //open and read file contains OPENCL code
    clProgram = OpenCLUtils::createProgramByResource(clContext, deviceId, ":src/util_gpu/opencl/BinaryFind.cl", openCLHelper, err);
    if (hasOPENCLError(err, "createProgramByResource() failed")) return 0;

    clKernel = openCLHelper.clCreateKernel_p(clProgram, "binarySearch_classic", &err);
    if(hasOPENCLError(err, "clCreateKernel() failed")) return 0;

	size_t preferredWorkGroupSize;
	err = openCLHelper.clGetKernelWorkGroupInfo_p(clKernel, deviceId, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
		sizeof(size_t), &preferredWorkGroupSize, NULL);
	if(hasOPENCLError(err, "clGetKernelWorkGroupInfo() failed")) return 0;
	algoLog.details(QObject::tr("Device's preferred work group size multiple is %1, using this number").arg(preferredWorkGroupSize));

	// if the global work size is greater than the device's preferred workgroup size (PWS), round it up to a multiple
	// of PWS to ensure that work groups of exactly that size are created by OpenCL impl
	size_t globalWorkSize = (size_t)ceil((double)needlesSize/ preferredWorkGroupSize) * preferredWorkGroupSize;

	algoLog.details(QString("needles: %1, haystack size: %2").arg(needlesSize).arg(haystackSize));
	algoLog.details(QString("global work size = %1").arg(globalWorkSize));

    buf_needlesArray = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * needlesSize, (void*)needles, &err);
    if (hasOPENCLError(err, "clCreateBuffer(buf_needlesArray)")) return 0;
    usageGPUMem += sizeof(NumberType) * needlesSize;

    const int outputArraySize = (int)needlesSize;

	buf_windowSizesArray = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
		sizeof(cl_int) * needlesSize, (void*)windowSizes, &err);
	if (hasOPENCLError(err, "clCreateBuffer(buf_windowSizesArray)")) return 0;
	usageGPUConstantMem += sizeof(cl_int) * needlesSize;

    buf_sortedHaystackArray = openCLHelper.clCreateBuffer_p(clContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             sizeof(NumberType) * haystackSize, (void*)haystack, &err);
    if (hasOPENCLError(err, "clCreateBuffer (buf_sortedHaystackArray)")) return 0;
    usageGPUConstantMem += sizeof(NumberType) * haystackSize;

    cl_uint kernelArgNum = 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, kernelArgNum++, sizeof(cl_mem), (void*)&buf_sortedHaystackArray);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(kernelArgNum))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, kernelArgNum++, sizeof(cl_int), (void*)&haystackSize);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(kernelArgNum))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, kernelArgNum++, sizeof(cl_mem), (void*)&buf_needlesArray);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(kernelArgNum))) return 0;

    err = openCLHelper.clSetKernelArg_p(clKernel, kernelArgNum++, sizeof(cl_int), (void*)&needlesSize);
    if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(kernelArgNum))) return 0;

	err = openCLHelper.clSetKernelArg_p(clKernel, kernelArgNum++, sizeof(cl_mem), (void*)&buf_windowSizesArray);
	if (hasOPENCLError(err, QString("clSetKernelArg argNum: %1").arg(kernelArgNum))) return 0;

	// try creating a queue with profiling enabled if that's possible and without if not
    clCommandQueue = openCLHelper.clCreateCommandQueue_p(clContext, deviceId, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | CL_QUEUE_PROFILING_ENABLE, &err);
	if(CL_INVALID_QUEUE_PROPERTIES == err) {
		clCommandQueue = openCLHelper.clCreateCommandQueue_p(clContext, deviceId, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
	}
	if (hasOPENCLError(err, "clCommandQueue() failed ")) return 0;

    err = openCLHelper.clEnqueueNDRangeKernel_p(
                    clCommandQueue,
                    clKernel,
                    1,
                    NULL,
                    &globalWorkSize,
                    NULL,
                    0,
                    NULL,
                    &clEvent1);
    if (hasOPENCLError(err, "clEnqueueNDRangeKernel")) return 0;

	err = openCLHelper.clFinish_p(clCommandQueue);
	if (hasOPENCLError(err, "clFinish 1")) return 0;

	logProfilingInfo(openCLHelper, clEvent1, QString("OpenCL kernel execution time (binary search)"));

    NumberType* outputArray = new NumberType[outputArraySize];
    err = openCLHelper.clEnqueueReadBuffer_p(clCommandQueue, buf_needlesArray, CL_FALSE, 0, sizeof(NumberType) * outputArraySize, outputArray, 0, NULL, &clEvent2);
    if (hasOPENCLError(err, "clEnqueueReadBuffer")) {
        delete[] outputArray;
        return 0;
    }

	err = openCLHelper.clFinish_p(clCommandQueue);
	if (hasOPENCLError(err, "clFinish 2")) {
		delete[] outputArray;
		return 0;
	}

	logProfilingInfo(openCLHelper, clEvent2, QString("OpenCL binary search results copying time"));

    time_t time2 = time(NULL);
    algoLog.details(QObject::tr("GPU execution time: %1 sec, GPUMem usage: %2 Mb, GPUConstantMem usage: %3 Mb")
                 .arg((time2 - time1))
                 .arg(usageGPUMem / (1 << 20))
                 .arg(usageGPUConstantMem / (1 << 20)));

    return outputArray;
}

BinaryFindOpenCL::~BinaryFindOpenCL() {
    algoLog.details(QObject::tr("clear OpenCL resources"));
    cl_int err = CL_SUCCESS;
    const OpenCLHelper& openCLHelper = AppContext::getOpenCLGpuRegistry()->getOpenCLHelper();

    if (clKernel) {
        err = openCLHelper.clReleaseKernel_p(clKernel);
        hasOPENCLError(err, "clReleaseKernel failed");
    }
    if (clProgram)  {
        err = openCLHelper.clReleaseProgram_p(clProgram);
        hasOPENCLError(err, "clReleaseProgram failed");
    }
    if (clCommandQueue) {
        err = openCLHelper.clReleaseCommandQueue_p(clCommandQueue);
        hasOPENCLError(err, "clReleaseCommandQueue failed");
    }
    if (clContext) {
        err = openCLHelper.clReleaseContext_p(clContext);
        hasOPENCLError(err, "clReleaseContext failed");
    }
    if (buf_sortedHaystackArray) {
        err = openCLHelper.clReleaseMemObject_p(buf_sortedHaystackArray);
        hasOPENCLError(err, "clReleaseMemObject(buf_sortedHaystackArray) failed");
    }
    if (buf_needlesArray) {
        err = openCLHelper.clReleaseMemObject_p(buf_needlesArray);
        hasOPENCLError(err, "clReleaseMemObject(buf_needlesArray) failed");
    }
	if (buf_windowSizesArray) {
		err = openCLHelper.clReleaseMemObject_p(buf_windowSizesArray);
		hasOPENCLError(err, "clReleaseMemObject(buf_windowSizesArray) failed");
	}
	if (clEvent1) {
		err = openCLHelper.clReleaseEvent_p (clEvent1);
		hasOPENCLError(err, "clReleaseEvent 1 failed");
	}

	if (clEvent2) {
		err = openCLHelper.clReleaseEvent_p (clEvent2);
		hasOPENCLError(err, "clReleaseEvent 2 failed");
	}
}

BinaryFindOpenCL::BinaryFindOpenCL(const NumberType *_haystack,
                           const int _haystackSize,
                           const NumberType *_needles,
                           const int _needlesSize,
                           const int *_windowSizes) :
        haystack(_haystack),
        haystackSize(_haystackSize),
        needles(_needles),
        needlesSize(_needlesSize),
        windowSizes(_windowSizes) {
    clEvent1 = 0;
    clEvent2 = 0;
    clKernel = 0;
    clProgram = 0;
    clCommandQueue = 0;
    clContext = 0;
    buf_sortedHaystackArray = 0;
    buf_needlesArray = 0;
	buf_windowSizesArray = 0;
}

bool BinaryFindOpenCL::hasOPENCLError(int err, QString errorMessage) {
    if(err != 0) {
		switch(err) {
			case CL_INVALID_BUFFER_SIZE:
				algoLog.error(QString("OPENCL: %1; Error code %2 (Invalid buffer size)").arg(errorMessage).arg(err));
				break;
			case CL_MEM_OBJECT_ALLOCATION_FAILURE:
				algoLog.error(QString("OPENCL: %1; Error code %2 (Memory object allocation failure)").arg(errorMessage).arg(err));
				break;
			case CL_OUT_OF_HOST_MEMORY:
				algoLog.error(QString("OPENCL: %1; Error code %2 (Out of host memory)").arg(errorMessage).arg(err));
				break;
			case CL_OUT_OF_RESOURCES:
				algoLog.error(QString("OPENCL: %1; Error code %2 (Out of resources)").arg(errorMessage).arg(err));
				break;
			default:
				algoLog.error(QString("OPENCL: %1; Error code %2").arg(errorMessage).arg(err));
				break;
		}
        return true;
    } else {
        return false;
    }
}

void BinaryFindOpenCL::logProfilingInfo(const OpenCLHelper &openCLHelper, const cl_event &event, const QString &msgPrefix) {
	cl_int err;
	cl_ulong clt1 = 0, clt2 = 0, clt3 = 0, clt4 = 0;
	if((err = openCLHelper.clGetEventProfilingInfo_p(event, CL_PROFILING_COMMAND_QUEUED, sizeof(cl_ulong), &clt1, NULL)) != 0
		|| (err = openCLHelper.clGetEventProfilingInfo_p(event, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &clt2, NULL)) != 0
		|| (err = openCLHelper.clGetEventProfilingInfo_p(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &clt3, NULL)) != 0
		|| (err = openCLHelper.clGetEventProfilingInfo_p(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &clt4, NULL)) != 0) {
			algoLog.details(QString("OpenCL profiling info unavailable (%1)").arg(err));
	} else {
		algoLog.details(QString("%1: %2/%3/%4 ms (since queued/submitted/execution started)").arg(msgPrefix)
            .arg((clt4 - clt1) / (double)1000000, 0, 'f', 2)
            .arg((clt4 - clt2) / (double)1000000, 0, 'f', 2)
            .arg((clt4 - clt3) / (double)1000000, 0, 'f', 2));
	}
}

}//namespace

#endif /*OPENCL_SUPPORT*/
