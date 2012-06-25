/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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
#include <QtCore/QHash>

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

    
class U2CORE_EXPORT AppResource {
public:
    AppResource(): resourceId(9), currentUse(0), maxUse(0){}

    AppResource(int id, int _maxUse, const QString& _name, const QString& _suffix = QString()) 
        : resourceId(id), currentUse(0), maxUse(_maxUse), name(_name), suffix(_suffix){};

    virtual ~AppResource(){}

    bool isAvailable(int n=1) const {return currentUse + n <= maxUse;}
    void acquire(int n=1) {assert(isAvailable(n)); currentUse += n;}
    void release(int n=1) {assert(currentUse-n>=0);currentUse -= n;}

    int resourceId;
    int currentUse;
    int maxUse;
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
    
    int getMaxThreadCount() const {return threadResource->maxUse;}
    void setMaxThreadCount(int n);
    
    int getMaxMemorySizeInMB() const {return memResource->maxUse;}
    void setMaxMemorySizeInMB(int m);
    
    static bool getCurrentAppMemory(int& mb); //size in megabytes, false is estimation only is used
    
    static bool isSSE2Enabled();
    
    void registerResource(AppResource* r);
    AppResource* getResource(int id) const; 
    
    static AppResourcePool* instance();

private:
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
