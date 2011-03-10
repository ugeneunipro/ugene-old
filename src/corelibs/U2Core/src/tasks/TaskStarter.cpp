#include "TaskStarter.h"

#include <U2Core/AppContext.h>

namespace U2 {

void TaskStarter::registerTask() {
    if (t) {
        AppContext::getTaskScheduler()->registerTopLevelTask(t);
        t = NULL;
    }
}

} //namespace
