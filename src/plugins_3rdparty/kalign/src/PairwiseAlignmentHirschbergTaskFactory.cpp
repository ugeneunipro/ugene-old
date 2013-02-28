#include "PairwiseAlignmentHirschbergTaskFactory.h"
#include "PairwiseAlignmentHirschbergTask.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>

namespace U2 {

PairwiseAlignmentHirschbergTaskFactory::PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentHirschbergTaskFactory::~PairwiseAlignmentHirschbergTaskFactory() {
}

PairwiseAlignmentTask* PairwiseAlignmentHirschbergTaskFactory::getTaskInstance(PairwiseAlignmentTaskSettings *_settings) const {
    PairwiseAlignmentHirschbergTaskSettings* settings = new PairwiseAlignmentHirschbergTaskSettings(*_settings);
    if (settings->inNewWindow == true && settings->resultFileName.isEmpty()) {
        settings->resultFileName = GUrl(AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath() +
                                        "/" + PairwiseAlignmentHirschbergTaskSettings::PA_H_DEFAULT_RESULT_FILE_NAME);
    }
    return new PairwiseAlignmentHirschbergTask(settings);
}

}   //namespace
