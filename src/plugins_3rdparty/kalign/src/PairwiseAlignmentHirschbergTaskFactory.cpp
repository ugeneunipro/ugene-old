#include "PairwiseAlignmentHirschbergTaskFactory.h"
#include "PairwiseAlignmentHirschbergTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/U2SafePoints.h>

namespace U2 {

PairwiseAlignmentHirschbergTaskFactory::PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentHirschbergTaskFactory::~PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentTask* PairwiseAlignmentHirschbergTaskFactory::getTaskInstance(PairwiseAlignmentTaskSettings *_settings) const {
    PairwiseAlignmentHirschbergTaskSettings* settings = new PairwiseAlignmentHirschbergTaskSettings(*_settings);
    SAFE_POINT(false == settings->inNewWindow || false == settings->resultFileName.isEmpty(),
               "Pairwise alignment: incorrect settings, empty output file name", NULL);
    return new PairwiseAlignmentHirschbergTask(settings);
}

}   //namespace
