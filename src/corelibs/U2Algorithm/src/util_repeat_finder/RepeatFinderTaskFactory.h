#ifndef _U2_REPEAT_FINDER_TASK_FACTORY_H_
#define _U2_REPEAT_FINDER_TASK_FACTORY_H_

#include "RepeatFinderSettings.h"
#include <U2Core/Task.h>

namespace U2 {

class RepeatFinderTaskFactory {
public:
    virtual Task* getTaskInstance(const RepeatFinderSettings& config) const = 0;
    virtual ~RepeatFinderTaskFactory() {}

    virtual void setRFResultsListener(Task*, RFResultsListener*) = 0;
};

} // namespace

#endif
