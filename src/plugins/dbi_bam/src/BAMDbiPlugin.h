#ifndef _U2_BAM_DBI_PLUGIN_H_
#define _U2_BAM_DBI_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/U2Dbi.h>

namespace U2 {
namespace BAM {

class BAMDbiPlugin : public Plugin
{
    Q_OBJECT
public:
    BAMDbiPlugin();
private slots:
    void sl_converter();
};

} // namespace BAM
} // namespace U2

#endif // _U2_BAM_DBI_PLUGIN_H_
