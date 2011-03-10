#include "SWTaskFactory.h"

namespace U2 {

SWTaskFactory::SWTaskFactory(SW_AlgType _algType) {
    algType = _algType;
}

SWTaskFactory::~SWTaskFactory() {
}

Task* SWTaskFactory::getTaskInstance( const SmithWatermanSettings& config, const QString& taskName) const {            
    return new SWAlgorithmTask(config, taskName, algType);
}

} // namespace
