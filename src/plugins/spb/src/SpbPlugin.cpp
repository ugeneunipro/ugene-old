#include "DereplicateSequencesWorker.h"
#include "FilterSequencesWorker.h"

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
    FilterSequencesWorkerFactory::init();
}

SpbPlugin::~SpbPlugin() {

}

const QString Constraints::WORKFLOW_CATEGORY("SPB");

} // SPB
