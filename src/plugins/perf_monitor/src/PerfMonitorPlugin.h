#ifndef _U2_PERF_MON_PLUGIN_H_
#define _U2_PERF_MON_PLUGIN_H_

#include <U2Core/PluginModel.h>

#include <QtGui/QAction>

namespace U2 {

class PerfMonitorPlugin : public Plugin  {
    Q_OBJECT
public:
    PerfMonitorPlugin();

private slots:
    void sl_openWindow();

private:
    QAction*    openWindowAction;
    int         windowId;
};

} //namespace

#endif
