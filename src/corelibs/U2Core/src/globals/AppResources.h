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


class U2CORE_EXPORT AppResource : public QSemaphore {
public:
    AppResource(int id, int _maxUse, const QString& _name, const QString& _suffix = QString()) 
        : QSemaphore(_maxUse), _maxUse(_maxUse), resourceId(id), name(_name), suffix(_suffix){};
    virtual ~AppResource(){}

    void release(int n = 1) {
        // QSemaphore allow to create resources by releasing, we do not want to get such behavior
        SAFE_POINT(n>=0, QString("AppResource <%1> release %2 < 0 called").arg(name).arg(n), );
        QSemaphore::release(n);
    }
    int maxUse() const { return _maxUse; }

    void setMaxUse (int n) {
        int diff = n - _maxUse;
        if (diff > 0) {
            // adding resources
            QSemaphore::release(diff);
            _maxUse += diff;
        } else {
            diff = -diff;
            // safely remove resources
            for (int i=diff; i>0; i--) {
                bool ok = tryAcquire(i, 0);
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
    
    static bool getCurrentAppMemory(int& mb); //size in megabytes, false is estimation only is used
    
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

}//namespace
#endif
