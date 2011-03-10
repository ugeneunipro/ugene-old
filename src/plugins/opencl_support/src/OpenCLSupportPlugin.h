#ifndef __OPENCL_SUPPORT_PLUGIN_H__
#define __OPENCL_SUPPORT_PLUGIN_H__

#include <U2Core/PluginModel.h>
#include <U2Algorithm/OpenCLGpuRegistry.h>
#include <U2Algorithm/OpenCLHelper.h>

namespace U2 {

#define ULOG_CAT_PLUGIN_OPENCL_SUPPORT "Plugin: OpenCLSupport"

class OpenCLSupportPlugin : public Plugin {
    Q_OBJECT
public:
    enum OpenCLSupportError {
        Error_NoError,
        Error_BadDriverLib,
        Error_OpenCLError
    };

    OpenCLSupportPlugin();
private:
    OpenCLSupportError obtainGpusInfo( QString & err );
    void loadGpusSettings();
    void registerAvailableGpus();
    bool hasOPENCLError(cl_int err, QString& errMessage);

//    static QString getCudaErrorString( CUresult code );
    static QString getSettingsErrorString( OpenCLSupportError err );

    QList<OpenCLGpuModel *> gpus;
};

}

#endif //__OPENCL_SUPPORT_PLUGIN_H__
