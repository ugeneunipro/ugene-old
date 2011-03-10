#include <algorithm>
#include <functional>
#include <QtCore/QtAlgorithms>

#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>

#include "OpenCLGpuRegistry.h"

namespace U2 {

OpenCLGpuRegistry::~OpenCLGpuRegistry() {
    saveGpusSettings();
    qDeleteAll( gpus.values() );
}

void OpenCLGpuRegistry::registerOpenCLGpu( OpenCLGpuModel * gpu ) {
    assert( !gpus.contains(gpu->getId()) );
    gpus.insert( gpu->getId(), gpu );
}

OpenCLGpuModel * OpenCLGpuRegistry::getGpuById( OpenCLGpuId id ) const {
    return gpus.value( id, 0 );
}

QList<OpenCLGpuModel *> OpenCLGpuRegistry::getRegisteredGpus() const {
    return gpus.values();
}

OpenCLGpuModel * OpenCLGpuRegistry::getAnyEnabledGpu() const {
    QHash<OpenCLGpuId, OpenCLGpuModel*>::const_iterator it = std::find_if( gpus.begin(), gpus.end(), std::mem_fun(&OpenCLGpuModel::isEnabled) );
    if( gpus.end() != it ) {
        return *it;
    }
    return 0;
}

OpenCLGpuModel * OpenCLGpuRegistry::acquireAnyReadyGpu() {
    QHash<OpenCLGpuId, OpenCLGpuModel*>::iterator it = std::find_if( gpus.begin(), gpus.end(), std::mem_fun(&OpenCLGpuModel::isReady) );
    if( gpus.end() != it ) {
        (*it)->setAcquired(true);
        return *it;
    }
    return 0;
}

void OpenCLGpuRegistry::saveGpusSettings() const {
    Settings * s = AppContext::getSettings();
    foreach( OpenCLGpuModel * m, gpus ) {
        QString key = OPENCL_GPU_REGISTRY_SETTINGS_GPU_SPECIFIC + QString::number(m->getId()) + OPENCL_GPU_SETTINGS_ENABLED;
        s->setValue( key, QVariant::fromValue(m->isEnabled()) );
    }
}

} //namespace
