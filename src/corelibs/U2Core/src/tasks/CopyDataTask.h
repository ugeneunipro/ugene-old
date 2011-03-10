#ifndef _U2_COPY_DATA_TASK_H_
#define _U2_COPY_DATA_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>

namespace U2 {

class IOAdapterFactory;

class U2CORE_EXPORT CopyDataTask : public Task {
    Q_OBJECT
public:
    CopyDataTask( IOAdapterFactory * ioFrom, const GUrl& urlFrom, 
                IOAdapterFactory * ioTo, const GUrl& urlTo );

    virtual void run();

private:
    static const int BUFFSIZE = 32 * 1024;    
    IOAdapterFactory * ioFrom;
    IOAdapterFactory * ioTo;
    GUrl urlFrom;
    GUrl urlTo;

};

}// namespace

#endif
