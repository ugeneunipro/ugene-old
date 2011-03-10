#include <U2Core/AppContext.h>
#include <U2Core/Settings.h>
#include <U2Core/TaskStarter.h>
#include <U2Core/CMDLineRegistry.h>
#include <U2Remote/RemoteMachineMonitor.h>

#include "RemoteServiceCommon.h"
#include "RemoteServicePlugin.h"
#include "RemoteServiceSettingsUI.h"
#include "RemoteServicePingTask.h"

namespace U2 {


#define EC2_URL     "http://184.73.180.209:80/rservice/engine"
#define EC2_PASS    "rulezzz"
#define PING_REMOTE_SERVICE  "ping-remote-service"

extern "C" Q_DECL_EXPORT U2::Plugin* U2_PLUGIN_INIT_FUNC()
{
    RemoteServicePlugin* plug = new RemoteServicePlugin();
    return plug;
}

const QString RemoteServiceCommon::WEB_TRANSPORT_PROTOCOL_ID = "Web transport protocol";

static void cleanupRemoteMachineMonitor() {
    RemoteMachineMonitor* rmm = AppContext::getRemoteMachineMonitor();
    QList<RemoteMachineMonitorItem> items = rmm->getRemoteMachineMonitorItems();
    
    foreach (const RemoteMachineMonitorItem& item, items) {
        rmm->removeMachine(item.machine);
    }
}


RemoteServicePlugin::RemoteServicePlugin():
    Plugin(tr("UGENE Remote Service Support"),
        tr("Launching remote tasks via UGENE Remote Service")),
    protocolUI((NULL == AppContext::getMainWindow())? NULL:(new RemoteServiceSettingsUI())),
    protocolInfo( RemoteServiceCommon::WEB_TRANSPORT_PROTOCOL_ID , protocolUI.get(),
                  &remoteMachineFactory )
{
    AppContext::getProtocolInfoRegistry()->registerProtocolInfo(&protocolInfo);

    if (thisIsFirstLaunch()) {
        cleanupRemoteMachineMonitor();
        RemoteServiceMachineSettings* settings = new RemoteServiceMachineSettings(EC2_URL);
        settings->setupCredentials(RemoteServiceMachineSettings::GUEST_ACCOUNT, EC2_PASS, true);
        AppContext::getRemoteMachineMonitor()->addMachine(settings, false);
    }
    registerCMDLineHelp();
    processCMDLineOptions();
}

RemoteServicePlugin::~RemoteServicePlugin()
{

}

void RemoteServicePlugin::registerCMDLineHelp()
{
    
}

void RemoteServicePlugin::processCMDLineOptions()
{
    CMDLineRegistry * cmdlineReg = AppContext::getCMDLineRegistry();
    assert(cmdlineReg != NULL);

    if( cmdlineReg->hasParameter( PING_REMOTE_SERVICE ) )
    {
        QString machinePath = cmdlineReg->getParameterValue(PING_REMOTE_SERVICE);
        Task * t = new RemoteServicePingTask(machinePath);
        connect(AppContext::getPluginSupport(), SIGNAL(si_allStartUpPluginsLoaded()), new TaskStarter(t), SLOT(registerTask()));
    }
}

#define NOT_FIRST_LAUNCH "remote_service/not_first_launch"

bool RemoteServicePlugin::thisIsFirstLaunch()
{
    Settings* settings = AppContext::getSettings();
    QString key = settings->toVersionKey(NOT_FIRST_LAUNCH);
    if (settings->contains(key)) {
        return false;
    } else {
        settings->setValue(key, QVariant(true));
        return true;
    }
}




} // namespace U2
