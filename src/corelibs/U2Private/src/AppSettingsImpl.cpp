#include "AppSettingsImpl.h"

#include <U2Core/NetworkConfiguration.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/AppResources.h>


namespace U2 {

AppSettingsImpl::AppSettingsImpl() {
    nc = new NetworkConfiguration();
    ri = new AppResourcePool();
    userAppsSettings = new UserAppsSettings();
}

AppSettingsImpl::~AppSettingsImpl() {
    delete nc;
    delete ri;
    delete userAppsSettings;
}

}//namespace

