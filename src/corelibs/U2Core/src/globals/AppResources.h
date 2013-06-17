/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#ifndef _U2_APPRESOURCES_H_
#define _U2_APPRESOURCES_H_

#include <U2Core/global.h>
#include <U2Core/U2SafePoints.h>
#include <QtCore/QHash>
#include <QtCore/QSemaphore>
#include <U2Core/U2OpStatus.h>

namespace U2 {

/** Thread resource - number of threads */
#define RESOURCE_THREAD     1

/** Memory resource - amount of memory in megabytes */
#define RESOURCE_MEMORY     2

/** CUDA GPU resource - ensures that device is busy or not*/
#define RESOURCE_CUDA_GPU   3

/** OPENCL GPU resource - ensures that device is busy or not*/
#define RESOURCE_OPENCL_GPU 4

/** 
    Project resource. There is 1 project active in the system and if the resource is locked  
    no project load/unload operation is possible
*/
#define RESOURCE_PROJECT    5

#define LOG_TRACE(METHOD) \
    coreLog.trace(QString("AppResource %1 ::" #METHOD " %2, available %3").arg(name).arg(n).arg(available()));

class U2CORE_EXPORT AppResource {
public:
    AppResource(int id, int _maxUse, const QString& _name, const QString& _suffix = QString()) 
        : resourceId(id), _maxUse(_maxUse), name(_name), suffix(_suffix), resource(NULL) {
        resource = new QSemaphore(_maxUse);
    }
    virtual ~AppResource(){
        delete resource; resource = NULL;
    }

    void acquire(int n = 1) {
        LOG_TRACE(acquire);
        resource->acquire(n);
    }

    bool tryAcquire(int n = 1) {
        LOG_TRACE(tryAcquire);
        return resource->tryAcquire(n);
    }

    bool tryAcquire(int n, int timeout) {
        LOG_TRACE(tryAcquire_timeout);
        return resource->tryAcquire(n, timeout);
    }

    void release(int n = 1) {
        LOG_TRACE(release);
        SAFE_POINT(n>=0, QString("AppResource %1 release %2 < 0 called").arg(name).arg(n), );
        resource->release(n);

        // QSemaphore allow to create resources by releasing, we do not want to get such behavior
        SAFE_POINT_EXT(resource->available() <= _maxUse,,);
    }

    int available() const {
        return resource->available();
    }

    int maxUse() const { return _maxUse; }

    void setMaxUse (int n) {
        LOG_TRACE(setMaxUse);
        int diff = n - _maxUse;
        if (diff > 0) {
            // adding resources
            resource->release(diff);
            _maxUse += diff;
        } else {
            diff = -diff;
            // safely remove resources
            for (int i=diff; i>0; i--) {
                bool ok = resource->tryAcquire(i, 0);
                if (ok) {
                    // successfully acquired i resources
                    _maxUse -= i;
                    break;
                }
            }
        }
    }

    int resourceId;
    int _maxUse;
    QString name;
    QString suffix;
private:
    AppResource( const AppResource& other );
    AppResource& operator= (const AppResource& other);

    QSemaphore *resource;
};

#define MIN_MEMORY_SIZE 200

class U2CORE_EXPORT AppResourcePool : public QObject {
    Q_OBJECT
public:
    AppResourcePool();
    virtual ~AppResourcePool();
    
    int getIdealThreadCount() const {return idealThreadCount;}
    void setIdealThreadCount(int n);
    
    int getMaxThreadCount() const {return threadResource->maxUse();}
    void setMaxThreadCount(int n);
    
    int getMaxMemorySizeInMB() const {return memResource->maxUse();}
    void setMaxMemorySizeInMB(int m);
    
    static size_t getCurrentAppMemory();
    
    static bool isSSE2Enabled();
    
    void registerResource(AppResource* r);
    AppResource* getResource(int id) const; 
    
    static AppResourcePool* instance();

    static int getTotalPhysicalMemory();

    static const int x32MaxMemoryLimitMb = 3*512;       // 1536Mb
    static const int x64MaxMemoryLimitMb = 2*1024*1024; // 2Tb
private:
    static const int defaultMemoryLimitMb = 8*1024;

    QHash<int, AppResource*> resources;

    int idealThreadCount;

    AppResource* threadResource;
    AppResource* memResource;
    AppResource* projectResouce;
    AppResource* phyTreeResource;
    AppResource* listenLogInGTest;
};


class MemoryLocker {
public:
    MemoryLocker(U2OpStatus& os, int preLockMB = 10)
        : os(&os), preLockMB(preLockMB), lockedMB(0), needBytes(0), resource(NULL) {
        resource = AppResourcePool::instance()->getResource(RESOURCE_MEMORY);

        preLockMB = preLockMB>0?preLockMB : 0;
        tryAcquire(0);
    }
    MemoryLocker(int preLockMB = 10)
        : os(NULL), preLockMB(preLockMB), lockedMB(0), needBytes(0), resource(NULL) {
        resource = AppResourcePool::instance()->getResource(RESOURCE_MEMORY);

        preLockMB = preLockMB>0?preLockMB : 0;
        tryAcquire(0);
    }

    MemoryLocker(MemoryLocker& other) {
        resource = other.resource;
        os = NULL;
        preLockMB = other.preLockMB;
        lockedMB = other.lockedMB; other.lockedMB = 0;
        needBytes = other.needBytes; other.needBytes = 0;
    }

    MemoryLocker& operator=(MemoryLocker& other) {
        MemoryLocker tmp(other);
        qSwap(os, tmp.os);
        qSwap(preLockMB, tmp.preLockMB);
        qSwap(lockedMB, tmp.lockedMB);
        qSwap(needBytes, tmp.needBytes);

        return *this;
    }

    virtual ~MemoryLocker() {
        release();
    }

    bool tryAcquire(qint64 bytes) {
        needBytes += bytes;

        int needMB = needBytes/(1000*1000) + preLockMB;
        if (needMB > lockedMB) {
            int diff = needMB - lockedMB;
            CHECK_EXT(NULL != resource, if (os) os->setError("MemoryLocker - Resource error"), false);
            bool ok = resource->tryAcquire(diff);
            if (ok) {
                lockedMB = needMB;
            } else {
                if (os) {
                    os->setError("MemoryLocker - Not enough memory error");
                }
            }
            return ok;
        }
        return true;
    }

    void release() {
        CHECK_EXT(NULL != resource, if (os) os->setError("MemoryLocker - Resource error"), );
        if (lockedMB > 0) {
            resource->release(lockedMB);
        }
        lockedMB = 0;
        needBytes = 0;
    }

private:
    U2OpStatus* os;
    int preLockMB;
    int lockedMB;
    qint64 needBytes;
    AppResource* resource;
};


}//namespace
#endif
