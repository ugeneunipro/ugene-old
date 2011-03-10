#ifndef _REMOTE_SERVICE_PLUGIN_H_
#define _REMOTE_SERVICE_PLUGIN_H_

#include <memory>
#include <U2Core/PluginModel.h>
#include <U2Remote/ProtocolInfo.h>

#include "RemoteServiceMachine.h"

namespace U2 {

class RemoteServiceSettingsUI;

class RemoteServicePlugin : public Plugin {
    Q_OBJECT

public:
    RemoteServicePlugin();
    virtual ~RemoteServicePlugin();

private:
    std::auto_ptr<RemoteServiceSettingsUI> protocolUI;
    RemoteServiceMachineFactory remoteMachineFactory;
    ProtocolInfo protocolInfo;
private:
    bool thisIsFirstLaunch();
    void registerCMDLineHelp();
    void processCMDLineOptions();
};

} // namespace U2

#endif // _REMOTE_SERVICE_PLUGIN_H_