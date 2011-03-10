#ifndef _U2_APPRESOURCES_H_
#define _U2_APPRESOURCES_H_

#include <U2Core/global.h>
#include <QtCore/QHash>

namespace U2 {

#define RESOURCE_THREAD 1
#define RESOURCE_MEMORY 2
#define RESOURCE_CUDA_GPU 3
#define RESOURCE_OPENCL_GPU 4
#define RESOURCE_PROJECT 5
#define RESOURCE_PHYTREE 6

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
};

}//namespace
#endif
