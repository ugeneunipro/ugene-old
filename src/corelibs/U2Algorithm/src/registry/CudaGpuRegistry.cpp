#include <algorithm>
#include <functional>
#include <QtCore/QtAlgorithms>

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include "CudaGpuRegistry.h"

namespace U2 {

CudaGpuRegistry::~CudaGpuRegistry() {
    saveGpusSettings();
    qDeleteAll( gpus.values() );
}

void CudaGpuRegistry::registerCudaGpu( CudaGpuModel * gpu ) {
    assert( !gpus.contains(gpu->getId()) );
    gpus.insert( gpu->getId(), gpu );
}

CudaGpuModel * CudaGpuRegistry::getGpuById( CudaGpuId id ) const {
    return gpus.value( id, 0 );
}

QList<CudaGpuModel *> CudaGpuRegistry::getRegisteredGpus() const {
    return gpus.values();
}

CudaGpuModel * CudaGpuRegistry::getAnyEnabledGpu() const {
    QHash<CudaGpuId, CudaGpuModel*>::const_iterator it = std::find_if( gpus.begin(), gpus.end(), std::mem_fun(&CudaGpuModel::isEnabled) );
    if( gpus.end() != it ) {
        return *it;
    }
    return 0;
}

CudaGpuModel * CudaGpuRegistry::acquireAnyReadyGpu() {
    QHash<CudaGpuId, CudaGpuModel*>::iterator it = std::find_if( gpus.begin(), gpus.end(), std::mem_fun(&CudaGpuModel::isReady) );
    if( gpus.end() != it ) {
        (*it)->setAcquired(true);
        return *it;
    }
    return 0;
}

void CudaGpuRegistry::saveGpusSettings() const {
    Settings * s = AppContext::getSettings();
    foreach( CudaGpuModel * m, gpus ) {
        QString key = CUDA_GPU_REGISTRY_SETTINGS_GPU_SPECIFIC + QString::number(m->getId()) + CUDA_GPU_SETTINGS_ENABLED;
        s->setValue( key, QVariant::fromValue(m->isEnabled()) );
    }
}

} //namespace
