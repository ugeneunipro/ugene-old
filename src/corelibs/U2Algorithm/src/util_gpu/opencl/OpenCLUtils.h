#ifndef __OPENCL_UTILS_H__
#define __OPENCL_UTILS_H__

#include "OpenCLHelper.h"

#include <QtCore/QString>

namespace U2 {

class U2ALGORITHM_EXPORT OpenCLUtils {
public:
    OpenCLUtils();

    static cl_program createProgramByResource(
                    cl_context clContext,
                    cl_device_id deviceId,
                    const QString& resourceName,
                    const OpenCLHelper& openCLHelper,
                    cl_int& err);
};

}//namespace

#endif //__OPENCL_UTILS_H__

