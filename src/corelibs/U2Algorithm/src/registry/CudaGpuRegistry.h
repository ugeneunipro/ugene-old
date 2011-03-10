#ifndef __CUDA_GPU_REGISTRY_H__
#define __CUDA_GPU_REGISTRY_H__

#include <QtCore/QHash>
#include <U2Core/global.h>

namespace U2 {

typedef int CudaGpuId;

#define CUDA_GPU_REGISTRY_SETTINGS "/cuda_gpu_registry"
//stores settings for concrete GPU. The key for appending - textual representation of CudaGpuId
#define CUDA_GPU_REGISTRY_SETTINGS_GPU_SPECIFIC "/cuda_gpu_registry/gpu_specific"
#define CUDA_GPU_SETTINGS_ENABLED "/enabled"

class U2ALGORITHM_EXPORT CudaGpuModel {
public:
    CudaGpuModel( const QString & _name, const CudaGpuId & _id, quint64 _globalMemorySize, bool _enabled  = true ) :
      name(_name), id(_id), globalMemorySizeBytes(_globalMemorySize), enabled(_enabled), acquired(false) {};

    QString getName() const {return name;}
    CudaGpuId getId() const {return id;}
    quint64 getGlobalMemorySizeBytes() const {return globalMemorySizeBytes;}

    bool isEnabled() const { return enabled;}
    void setEnabled(bool b) {enabled = b;}

    bool isAcquired() const {return acquired;}
    void setAcquired( bool a ) {acquired = a;}

    bool isReady() {return !isAcquired() && isEnabled(); }
private:
    //TODO: add another cuda device properties from cudaDeviceProp
    QString name;
    CudaGpuId id;
    quint64 globalMemorySizeBytes;
    bool enabled;
    bool acquired;
};

class U2ALGORITHM_EXPORT CudaGpuRegistry {
public:
    ~CudaGpuRegistry();

    void registerCudaGpu( CudaGpuModel * gpu );
    CudaGpuModel * getGpuById( CudaGpuId id ) const;
    QList<CudaGpuModel*> getRegisteredGpus() const;

    CudaGpuModel * getAnyEnabledGpu() const;

    CudaGpuModel * acquireAnyReadyGpu();

    bool empty() const { return gpus.empty(); }

private:
    void saveGpusSettings() const;
    QHash< CudaGpuId, CudaGpuModel * > gpus;  
};

} //namespace

#endif //__CUDA_GPU_REGISTRY_H__
