#include <U2Core/Version.h>
#include <U2Core/GUrl.h>
#include "private.h"

namespace U2 {

#define PLUGIN_FILE_EXT "plugin"

enum PlatformName {
    PlatformName_Unknown,
    PlatformName_Win,
    PlatformName_UnixNotMac,
    PlatformName_Mac
};

enum PlatformArch {
    PlatformArch_Unknown,
    PlatformArch_32,
    PlatformArch_64
};

enum PluginModeFlag {
    PluginMode_Malformed    = 1,
    PluginMode_Console      = 1 << 1,
    PluginMode_UI           = 1 << 2
};

typedef QFlags<PluginModeFlag> PluginMode;

class PlatformInfo {
public:
    PlatformName name;
    PlatformArch arch;
    bool operator == (const PlatformInfo& p) const  {return name == p.name && arch == p.arch;}
};

class DependsInfo {
public:
    QString id;
    Version version;
};

class PluginDesc {
public:
    QString             id;
    QString             name;
    Version             pluginVersion;
    Version             ugeneVersion;
    Version             qtVersion;
    QString             pluginVendor;
    GUrl                descriptorUrl;
    GUrl                libraryUrl;
    PlatformInfo        platform;
    PluginMode          mode;
    QList<DependsInfo>  dependsList;

    bool isValid() const {
        return !id.isEmpty() && !pluginVersion.text.isEmpty() && !ugeneVersion.text.isEmpty() && !libraryUrl.isEmpty();
    }

    bool operator == (const PluginDesc& pd) const ;
};


class U2PRIVATE_EXPORT PluginDescriptorHelper: public QObject {
    Q_OBJECT
private: 
    PluginDescriptorHelper() {};
public:
    static PluginDesc readPluginDescriptor(const QString& url, QString& error);

    //orders plugins by loading order
    static QList<PluginDesc> orderPlugins(const QList<PluginDesc>& unordered, QString& err);

};


} //namespace

