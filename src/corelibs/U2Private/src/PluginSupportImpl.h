#ifndef _U2_PLUGINSUPPORT_IMPL_
#define _U2_PLUGINSUPPORT_IMPL_

#include "private.h"
#include <U2Core/PluginModel.h>
#include <U2Core/Task.h>

#include <QtCore/QLibrary>
#include <QtCore/QDir>

#include "PluginDescriptor.h"
#include <memory>

namespace U2 {

class ServiceRegistryImpl;
class AddPluginTask;
class RemovePluginTask;

class PluginRef {
public:
    PluginRef(Plugin* _plugin, QLibrary* _library, const PluginDesc& desc);

    ~PluginRef();

    Plugin*     plugin;
    QLibrary*   library;
    PluginDesc  pluginDesc;
    bool        removeFlag;
};

class U2PRIVATE_EXPORT PluginSupportImpl : public PluginSupport {
    Q_OBJECT

    friend class AddPluginTask;
    
public:
    PluginSupportImpl();
    ~PluginSupportImpl();

    virtual const QList<Plugin*>& getPlugins() {return plugins;}

    virtual Task* addPluginTask(const QString& pathToPlugin);

    //plugin will not be removed from the plugin list during the next app run
    virtual void setRemoveFlag(Plugin* p, bool v);
    virtual bool getRemoveFlag(Plugin* p) const;


    static bool isDefaultPluginsDir(const QString& url);
    static QDir getDefaultPluginsDir();

    PluginRef* findRef(Plugin* p) const;
    PluginRef* findRefById(const QString& pluginId) const;

    virtual bool isAllPluginsLoaded() const;

    bool allLoaded;
    
protected:
    void registerPlugin(PluginRef* ref); 
    QString getPluginFileURL(Plugin* p) const;
    
    void updateSavedState(PluginRef* ref);

private:
    QList<PluginRef*>   plugRefs;
    QList<Plugin*>      plugins;
};


class AddPluginTask : public Task {
    Q_OBJECT
public:
    AddPluginTask(PluginSupportImpl* ps, const PluginDesc& desc);
    ReportResult report();
    

private:
    PluginSupportImpl*  ps;
    PluginDesc          desc;
};

class LoadAllPluginsTask : public Task {
    Q_OBJECT
public:
    LoadAllPluginsTask(PluginSupportImpl* ps,const QStringList& pluginFiles);
    void prepare();
    ReportResult report();
private:
    void addToOrderingQueue(const QString& url);

    PluginSupportImpl*  ps;
    QStringList         pluginFiles;
    QList<PluginDesc>   orderedPlugins; // plugins ordered by desc
};


}//namespace

#endif 
