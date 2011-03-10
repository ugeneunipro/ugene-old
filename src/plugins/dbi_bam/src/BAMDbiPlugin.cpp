#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/U2DbiUtils.h>
#include "BAMFormat.h"
#include "Dbi.h"
#include "BAMDbiPlugin.h"

namespace U2 {
namespace BAM {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    BAMDbiPlugin* plug = new BAMDbiPlugin();
    return plug;
}


BAMDbiPlugin::BAMDbiPlugin() : Plugin(tr("BAM format support"), tr("Interface for indexed read-only access to BAM files"))
{
    AppContext::getDocumentFormatRegistry()->registerFormat(new BAMFormat());
    AppContext::getDbiRegistry()->registerDbiFactory(new DbiFactory());
}

} // namespace BAM
} // namespace U2
