#ifndef __CUDA_SUPPORT_PLUGIN_H__
#define __CUDA_SUPPORT_PLUGIN_H__

#include <cuda.h>
#include <U2Core/PluginModel.h>
#include <U2Algorithm/CudaGpuRegistry.h>

namespace U2 {

#define ULOG_CAT_PLUGIN_CUDA_SUPPORT "Plugin: CudaSupport"

class CudaSupportPlugin : public Plugin {
    Q_OBJECT
public:
    enum Error {
        Error_NoError,
        Error_NoDriverLib,
        Error_BadDriverLib,
        Error_CudaError
    };

    CudaSupportPlugin();
private:
    Error obtainGpusInfo( QString & err );
    void loadGpusSettings();
    void registerAvailableGpus();

    static QString getCudaErrorString( CUresult code );
    static QString getSettingsErrorString( Error err );

    QList<CudaGpuModel *> gpus;
};

}

#endif //__CUDA_SUPPORT_PLUGIN_H__
