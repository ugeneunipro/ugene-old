/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
    QList<RemoteMachineSettingsPtr> items = rmm->getRemoteMachineMonitorItems();
    
    foreach (const RemoteMachineSettingsPtr& item, items) {
        rmm->removeMachineConfiguration(item);
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
        RemoteServiceSettingsPtr settings( new RemoteServiceMachineSettings(EC2_URL) );
        settings->setupCredentials(RemoteServiceMachineSettings::GUEST_ACCOUNT, EC2_PASS, true);
        AppContext::getRemoteMachineMonitor()->addMachineConfiguration(settings);
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
