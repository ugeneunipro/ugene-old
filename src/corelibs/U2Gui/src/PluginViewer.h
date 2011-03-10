#ifndef _U2_PLUGIN_VIEWER_H_
#define _U2_PLUGIN_VIEWER_H_

#include <U2Core/global.h>
#include <U2Core/PluginModel.h>
#include <U2Core/ServiceTypes.h>

class QAction;

namespace U2 {

class U2GUI_EXPORT PluginViewer : public Service {
    Q_OBJECT
public:
    PluginViewer(const QString& name, const QString& desc) 
        : Service(Service_PluginViewer, name, desc) 
    {}

};

} //namespace

#endif
