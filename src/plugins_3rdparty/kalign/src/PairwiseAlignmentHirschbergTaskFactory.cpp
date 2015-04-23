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

AbstractAlignmentTask* PairwiseAlignmentHirschbergTaskFactory::getTaskInstance(AbstractAlignmentTaskSettings *_settings) const {
    PairwiseAlignmentTaskSettings* pairwiseSettings = dynamic_cast<PairwiseAlignmentTaskSettings*>(_settings);
    SAFE_POINT(pairwiseSettings != NULL,
        "Pairwise alignment: incorrect settings", NULL);
    PairwiseAlignmentHirschbergTaskSettings* settings = new PairwiseAlignmentHirschbergTaskSettings(*pairwiseSettings);
    SAFE_POINT(false == settings->inNewWindow || false == settings->resultFileName.isEmpty(),
               "Pairwise alignment: incorrect settings, empty output file name", NULL);
    return new PairwiseAlignmentHirschbergTask(settings);
}

}   //namespace
