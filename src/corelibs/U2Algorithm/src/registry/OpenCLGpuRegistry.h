/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#ifndef __OPEN_CL_GPU_REGISTRY_H__
#define __OPEN_CL_GPU_REGISTRY_H__

#include <QtCore/QHash>
#include <U2Core/global.h>

namespace U2 {

typedef long OpenCLGpuId;

#define OPENCL_GPU_REGISTRY_SETTINGS "/opencl_gpu_registry"
//stores settings for concrete GPU. The key for appending - textual representation of OpenCLGpuId
#define OPENCL_GPU_REGISTRY_SETTINGS_GPU_SPECIFIC "/opencl_gpu_registry/gpu_specific"
#define OPENCL_GPU_SETTINGS_ENABLED "/enabled"

class U2ALGORITHM_EXPORT OpenCLGpuModel {
public:
    OpenCLGpuModel( const QString & _name,
                    const OpenCLGpuId & _id,
                    quint64 _platformId,
                    quint64 _globalMemorySizeBytes,
                    quint64 _localMemorySizeBytes,
                    quint32 _maxComputeUnits,
                    size_t _maxWorkGroupSize,
                    quint32 _maxClockFrequency,
                    bool _enabled  = true) :
      name(_name),
      id(_id),
      platformId(_platformId),
      globalMemorySizeBytes(_globalMemorySizeBytes),
      localMemorySizeBytes(_localMemorySizeBytes),
      maxComputeUnits(_maxComputeUnits),
      maxWorkGroupSize(_maxWorkGroupSize),
      maxClockFrequency(_maxClockFrequency),
      enabled(_enabled),
      acquired(false) {};

    QString getName() const {return name;}
    OpenCLGpuId getId() const {return id;}
    quint64 getGlobalMemorySizeBytes() const {return globalMemorySizeBytes;}    
    quint64 getLocalMemorySizeBytes() const {return localMemorySizeBytes;}
    quint32 getMaxComputeUnits() const {return maxComputeUnits;}
    size_t getMaxWorkGroupSize() const {return maxWorkGroupSize;}
    quint32 getMaxClockFrequency() const {return maxClockFrequency;}
    quint64 getPlatformId() const {return platformId;}

    bool isEnabled() const {return  enabled;} 
    void setEnabled(bool b) {enabled = b;}

    bool isAcquired() const {return acquired;}
    void setAcquired( bool a) {acquired = a;}

    bool isReady() {return !isAcquired() && isEnabled(); }
private:
    //TODO: add another opencl device properties
    QString name;
    OpenCLGpuId id;
    quint64 platformId;
    quint64 globalMemorySizeBytes;
    quint64 localMemorySizeBytes;
    quint32 maxComputeUnits;
    size_t maxWorkGroupSize;
    quint32 maxClockFrequency;
    bool enabled;
    bool acquired;
};

class U2ALGORITHM_EXPORT OpenCLGpuRegistry {
public:
    ~OpenCLGpuRegistry();

    void registerOpenCLGpu( OpenCLGpuModel * gpu );
    OpenCLGpuModel * getGpuById( OpenCLGpuId id ) const;
    QList<OpenCLGpuModel*> getRegisteredGpus() const;

    OpenCLGpuModel * getAnyEnabledGpu() const;

    OpenCLGpuModel * acquireAnyReadyGpu();

    bool empty() const { return gpus.empty(); }

private:
    void saveGpusSettings() const;
    QHash< OpenCLGpuId, OpenCLGpuModel * > gpus;
};

} //namespace

#endif //__OPEN_CL_GPU_REGISTRY_H__
