#ifndef _SPB_PLUGIN_H_
#define _SPB_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>

using namespace U2;

namespace SPB {

class SpbPlugin : public Plugin  {
    Q_OBJECT
public:
    SpbPlugin();
    ~SpbPlugin();
};

class Constraints {
public:
    static const QString WORKFLOW_CATEGORY;
};

} // SPB

#endif // _SPB_PLUGIN_H_
