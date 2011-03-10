#ifndef _U2_SMITH_WATERMAN_ALG_IMPL_H_
#define _U2_SMITH_WATERMAN_ALG_IMPL_H_

#include <U2Algorithm/SmithWatermanTaskFactory.h>
#include <U2Algorithm/SmithWatermanSettings.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/Task.h>

#include "SWAlgorithmTask.h"


namespace U2 {

class SWTaskFactory: public SmithWatermanTaskFactory {
public:
    SWTaskFactory(SW_AlgType _algType);
    virtual ~SWTaskFactory();
    virtual Task* getTaskInstance(const SmithWatermanSettings& config, const QString& taskName) const;

private:
    bool isValidParameters(const SmithWatermanSettings& sWatermanConfig,  SequenceWalkerSubtask* t) const;
    SW_AlgType algType;

};

} // namespace

#endif
