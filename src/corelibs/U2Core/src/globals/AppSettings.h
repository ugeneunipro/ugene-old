#ifndef _U2_APPSETTINGS_H_
#define _U2_APPSETTINGS_H_

#include <U2Core/global.h>

namespace U2 {

class NetworkConfiguration;
class UserAppsSettings;
class AppResourcePool;

/** A collection for all settings used in app that have C++ model description */

class U2CORE_EXPORT AppSettings {
public:
    AppSettings() : nc(NULL), userAppsSettings(NULL), ri(NULL){}

    virtual ~AppSettings(){}

    NetworkConfiguration* getNetworkConfiguration() const {return nc;}

    UserAppsSettings* getUserAppsSettings() const {return userAppsSettings;}

    AppResourcePool* getAppResourcePool() const {return ri;}

protected:
    NetworkConfiguration*   nc;
    UserAppsSettings*       userAppsSettings;
    AppResourcePool*        ri;
};

}//namespace
#endif
