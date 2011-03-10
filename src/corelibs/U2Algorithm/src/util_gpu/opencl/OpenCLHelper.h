#ifndef __OPENCL_HELPER__
#define __OPENCL_HELPER__

#include <U2Core/global.h>

#include <QtCore/QLibrary>
#include <QtCore/QString>

#include <opencl/legacy/cl.h>

#define OPENCL_DRIVER_LIB "OpenCL"

namespace U2 {

class U2ALGORITHM_EXPORT OpenCLHelper {

public:

#ifdef Q_OS_WIN
    #define CALLING_CONVENTION __stdcall
#else
    #define CALLING_CONVENTION
#endif  

    typedef cl_int ( CALLING_CONVENTION *clGetPlatformIDs_f)(
        cl_uint num_entries,
        cl_platform_id *platforms,
        cl_uint *num_platforms);

    typedef cl_int ( CALLING_CONVENTION *clGetPlatformInfo_f)(
        cl_platform_id platform,
        cl_platform_info param_name,
        size_t param_value_size,
        void *param_value,
        size_t *param_value_size_ret);

    typedef cl_int ( CALLING_CONVENTION *clGetDeviceIDs_f)(
        cl_platform_id platform,
        cl_device_type device_type,
        cl_uint num_entries,
        cl_device_id *devices,
        cl_uint *num_devices);

    typedef cl_int ( CALLING_CONVENTION *clGetDeviceInfo_f)(
        cl_device_id device,
        cl_device_info param_name,
        size_t param_value_size,
        void *param_value,
        size_t *param_value_size_ret);

    typedef cl_context ( CALLING_CONVENTION *clCreateContext_f)(
        const cl_context_properties *properties,
        cl_uint num_devices,
        const cl_device_id *devices,
        void (CL_CALLBACK *pfn_notify)(const char *errinfo,
        const void *private_info, size_t cb,
        void *user_data),
        void *user_data,
        cl_int *errcode_ret);

    typedef cl_mem ( CALLING_CONVENTION *clCreateBuffer_f)(
        cl_context context,
        cl_mem_flags flags,
        size_t size,
        void *host_ptr,
        cl_int *errcode_ret);

    typedef cl_program ( CALLING_CONVENTION *clCreateProgramWithSource_f)(
        cl_context context,
        cl_uint count,
        const char **strings,
        const size_t *lengths,
        cl_int *errcode_ret);

    typedef cl_int ( CALLING_CONVENTION *clGetProgramBuildInfo_f)(
        cl_program program,
        cl_device_id device,
        cl_program_build_info param_name,
        size_t param_value_size,
        void *param_value,
        size_t *param_value_size_ret);

    typedef cl_kernel ( CALLING_CONVENTION *clCreateKernel_f)(
        cl_program program,
        const char *kernel_name,
        cl_int *errcode_ret);

    typedef cl_int ( CALLING_CONVENTION *clSetKernelArg_f)(
        cl_kernel kernel,
        cl_uint arg_index,
        size_t arg_size,
        const void *arg_value);

    typedef cl_command_queue ( CALLING_CONVENTION *clCreateCommandQueue_f)(
        cl_context context,
        cl_device_id device,
        cl_command_queue_properties properties,
        cl_int *errcode_ret);

    typedef cl_int ( CALLING_CONVENTION *clEnqueueNDRangeKernel_f)(
        cl_command_queue command_queue,
        cl_kernel kernel,
        cl_uint work_dim,
        const size_t *global_work_offset,
        const size_t *global_work_size,
        const size_t *local_work_size,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event);

    typedef cl_int ( CALLING_CONVENTION *clWaitForEvents_f)(
        cl_uint num_events,
        const cl_event *event_list);

    typedef cl_int ( CALLING_CONVENTION *clEnqueueReadBuffer_f)(cl_command_queue command_queue,
        cl_mem buffer,
        cl_bool blocking_read,
        size_t offset,
        size_t cb,
        void *ptr,
        cl_uint num_events_in_wait_list,
        const cl_event *event_wait_list,
        cl_event *event);

    typedef cl_int ( CALLING_CONVENTION *clFinish_f)(cl_command_queue command_queue);

    typedef cl_int ( CALLING_CONVENTION *clReleaseEvent_f)(cl_event event);

    typedef cl_int ( CALLING_CONVENTION *clReleaseKernel_f)(cl_kernel kernel);

    typedef cl_int ( CALLING_CONVENTION *clReleaseProgram_f)(cl_program program);

    typedef cl_int ( CALLING_CONVENTION *clReleaseCommandQueue_f)(cl_command_queue command_queue);

    typedef cl_int ( CALLING_CONVENTION *clReleaseContext_f)(cl_context context);

    typedef cl_int ( CALLING_CONVENTION *clReleaseMemObject_f)(cl_mem memobj);

    typedef cl_int ( CALLING_CONVENTION *clBuildProgram_f)(cl_program program,
        cl_uint num_devices,
        const cl_device_id *device_list,
        const char *options,
        void (CL_CALLBACK *pfn_notify)(cl_program program,
        void *user_data),
        void *user_data);

    clGetPlatformIDs_f clGetPlatformIDs_p;
    clGetPlatformInfo_f clGetPlatformInfo_p;
    clGetDeviceIDs_f clGetDeviceIDs_p;
    clGetDeviceInfo_f clGetDeviceInfo_p;

    clCreateContext_f clCreateContext_p;
    clCreateBuffer_f clCreateBuffer_p;
    clCreateProgramWithSource_f clCreateProgramWithSource_p;
    clGetProgramBuildInfo_f clGetProgramBuildInfo_p;
    clCreateKernel_f clCreateKernel_p;
    clSetKernelArg_f clSetKernelArg_p;
    clCreateCommandQueue_f clCreateCommandQueue_p;
    clEnqueueNDRangeKernel_f clEnqueueNDRangeKernel_p;
    clWaitForEvents_f clWaitForEvents_p;
    clEnqueueReadBuffer_f clEnqueueReadBuffer_p;
    clFinish_f clFinish_p;
    clBuildProgram_f clBuildProgram_p;
    clReleaseEvent_f clReleaseEvent_p;

    clReleaseKernel_f clReleaseKernel_p;
    clReleaseProgram_f clReleaseProgram_p;
    clReleaseCommandQueue_f clReleaseCommandQueue_p;
    clReleaseContext_f clReleaseContext_p;
    clReleaseMemObject_f clReleaseMemObject_p;




    enum OpenCLHelperError {
        Error_NoError,
        Error_NoDriverLib,
        Error_BadDriverLib,
    };

    bool isLoaded() {return (status == Error_NoError);}

    QString getErrorString();

    OpenCLHelper();
    ~OpenCLHelper();
private:
    OpenCLHelperError status;
    QLibrary openclLib;
};

}

#endif //__OPENCL_HELPER__
