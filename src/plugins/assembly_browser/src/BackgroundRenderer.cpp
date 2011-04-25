#include "BackgroundRenderer.h"

namespace U2 {

BackgroundTaskRunner_base::~BackgroundTaskRunner_base() {
}

void BackgroundTaskRunner_base::emitFinished() {
    emit(si_finished());
}

} //ns
