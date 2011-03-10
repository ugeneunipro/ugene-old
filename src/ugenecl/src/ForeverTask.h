
#ifndef _U2_FOREVER_TASK_H_
#define _U2_FOREVER_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

/* need for congene not to die */
class ForeverTask : public Task {
    Q_OBJECT
public:
    ForeverTask();
    ~ForeverTask();
    
    virtual void run();
    
}; // ForeverTask

} // U2

#endif // _U2_FOREVER_TASK_H_
