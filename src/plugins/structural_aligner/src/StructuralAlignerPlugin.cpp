#include "StructuralAlignerPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmFactory.h>
#include <U2Algorithm/StructuralAlignmentAlgorithmRegistry.h>

#include "StructuralAlignerPtools.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    StructuralAlignerPlugin *plug = new StructuralAlignerPlugin();
    return plug;
}

/* class StructuralAlignerPlugin : public Plugin */

StructuralAlignerPlugin::StructuralAlignerPlugin() : Plugin(tr("Structural Aligner"), tr("This plugin contains structural alignment algorithms (ptools)"))
{
    StructuralAlignmentAlgorithmFactory *ptools = new StructuralAlignerPtoolsFactory();
    AppContext::getStructuralAlignmentAlgorithmRegistry()->registerAlgorithmFactory(ptools, "ptools");
}

StructuralAlignerPlugin::~StructuralAlignerPlugin()
{
}

}   // namespace U2

