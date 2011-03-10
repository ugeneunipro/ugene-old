#ifndef _U2_SMITH_WATERMAN_TASK_FACTORY_H_
#define _U2_SMITH_WATERMAN_TASK_FACTORY_H_

#include "SmithWatermanSettings.h"
#include <U2Core/Task.h>


namespace U2 {

class SmithWatermanTaskFactory {
public:
    virtual Task* getTaskInstance(const SmithWatermanSettings& config,
                                  const QString& taskName) const = 0;
    virtual bool hasAdvancedSettings() const { return false; }
    virtual void execAdvancedDialog() {}
    virtual ~SmithWatermanTaskFactory() {}
};

} // namespace

#endif
