#include "OpenCLUtils.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <U2Core/GAutoDeleteList.h>

#include <U2Core/Log.h>

namespace U2 {

OpenCLUtils::OpenCLUtils(){}

cl_program OpenCLUtils::createProgramByResource(
                cl_context clContext,
                cl_device_id deviceId,
                const QString& resourceName,
                const OpenCLHelper& openCLHelper,
                cl_int& err) {
    //open and read file contains OPENCL code
    QByteArray file;
    QFile data(resourceName);

    if (data.open(QFile::ReadOnly)) {
        file = data.readAll();
        data.close();
    } else {
        ioLog.error(QString("No source file: %1").arg(data.errorString()));
        return 0;
    }

    const char* sourceCode = file.constData();
    const size_t sourceLength = file.size();
    cl_program clProgram = openCLHelper.clCreateProgramWithSource_p(clContext, 1, &sourceCode,
        &sourceLength, &err);

    if(err != 0) {
        coreLog.error(QString("OPENCL: clCreateProgramWithSource, Error code (%2)").arg(err));
        return 0;
    }

    err = openCLHelper.clBuildProgram_p(clProgram, 0, NULL, NULL, NULL, NULL);

    if (err != CL_SUCCESS) {
        size_t logSize = 1;
        openCLHelper.clGetProgramBuildInfo_p(clProgram, deviceId, CL_PROGRAM_BUILD_LOG, NULL, NULL, &logSize);
        gauto_array<char> logStr(new char[logSize]);
        openCLHelper.clGetProgramBuildInfo_p(clProgram, deviceId, CL_PROGRAM_BUILD_LOG, logSize, logStr.get(), &logSize);

        coreLog.error(QObject::tr("OPENCL: BUILD LOG \n ******************** \n %1 \n ********************").arg(logStr.get()));
        coreLog.error(QObject::tr("OPENCL: Program::build() failed. (%1)").arg(err));
        return 0;
    }

    return clProgram;
}

}//namespace
