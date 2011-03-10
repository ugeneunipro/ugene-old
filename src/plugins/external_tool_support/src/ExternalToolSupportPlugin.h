#ifndef _U2_EXTERNAL_TOOL_SUPPORT_PLUGIN_H_
#define _U2_EXTERNAL_TOOL_SUPPORT_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/ServiceModel.h>

namespace U2 {
class ETSProjectViewItemsContoller;

class ExternalToolSupportPlugin : public Plugin  {
    Q_OBJECT
public:
    ExternalToolSupportPlugin();
    ~ExternalToolSupportPlugin();

private slots:
    void sl_validateTaskStateChanged();
};

class ExternalToolSupportService: public Service {
    Q_OBJECT
public:
    ExternalToolSupportService();

protected:
    virtual void serviceStateChangedCallback(ServiceState oldState, bool enabledStateChanged);

    ETSProjectViewItemsContoller*    projectViewController;
};

}


#endif
