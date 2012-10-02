#include "DereplicateSequencesWorker.h"
#include "DistanceReportWorker.h"
#include "FilterSequencesWorker.h"
#include "GenerateUrlWorker.h"
#include "RandomFilterWorker.h"

#include "SpbPlugin.h"

namespace SPB {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    SpbPlugin *plug = new SpbPlugin();
    return plug;
}

SpbPlugin::SpbPlugin()
: Plugin(tr("SPB Plugin"), tr("SPB Plugin"))
{
    DereplicateSequencesWorkerFactory::init();
    DistanceReportWorkerFactory::init();
    FilterSequencesWorkerFactory::init();
    GenerateUrlWorkerFactory::init();
    RandomFilterWorkerFactory::init();
}

SpbPlugin::~SpbPlugin() {

}

const QString Constraints::WORKFLOW_CATEGORY("SPB");

} // SPB
