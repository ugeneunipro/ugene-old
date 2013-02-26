#include "PairwiseAlignmentHirschbergTaskFactory.h"
#include "PairwiseAlignmentHirschbergTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppFileStorage.h>

namespace U2 {

PairwiseAlignmentHirschbergTaskFactory::PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentHirschbergTaskFactory::~PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentTask* PairwiseAlignmentHirschbergTaskFactory::getTaskInstance(PairwiseAlignmentTaskSettings *_settings) const {
    PairwiseAlignmentHirschbergTaskSettings* settings = new PairwiseAlignmentHirschbergTaskSettings(*_settings);
    if (settings->inNewWindow == true && settings->resultFileName.isEmpty()) {
        settings->resultFileName = GUrl(AppContext::getAppFileStorage()->getStorageDir() + "/" + PA_H_DEFAULT_RESULT_FILE_NAME);
    }
    return new PairwiseAlignmentHirschbergTask(settings);
}

}   //namespace
