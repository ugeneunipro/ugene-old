#include <U2Core/AppContext.h>
#include <U2Algorithm/MolecularSurfaceFactoryRegistry.h>
#include <SES.h>
#include <SAS.h>

#include "BallPlugin.h"

namespace U2 {

extern "C" Q_DECL_EXPORT U2::Plugin* U2_PLUGIN_INIT_FUNC() {
    BallPlugin* plug = new BallPlugin();
    return plug;
}

BallPlugin::BallPlugin()
: Plugin(tr("BALL"),tr("A port of BALL framework for molecular surface calculation"))
{
    AppContext::getMolecularSurfaceFactoryRegistry()->registerSurfaceFactory(new SolventExcludedSurfaceFactory(),QString("SES"));
    AppContext::getMolecularSurfaceFactoryRegistry()->registerSurfaceFactory(new SolventAccessibleSurfaceFactory(),QString("SAS"));
}

} // namespace
