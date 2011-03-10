#include "PerfMonitorPlugin.h"
#include "PerfMonitorView.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#include<QtGui/QMenu>

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        PerfMonitorPlugin * plug = new PerfMonitorPlugin();
        return plug;
    }
    return NULL;
}

PerfMonitorPlugin::PerfMonitorPlugin() : Plugin( tr("Performance Monitor"), tr("Shows performance counters") ) {
    windowId = 0;
    openWindowAction = new QAction(tr("Show counters"), this);
    openWindowAction->setIcon(QIcon(":perf_monitor/images/mon.png"));
    connect(openWindowAction, SIGNAL(triggered()), SLOT(sl_openWindow()));

    QMenu* toolsMenu = AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS);
    toolsMenu->addAction(openWindowAction);
}

void PerfMonitorPlugin::sl_openWindow() {
    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    MWMDIWindow* mdiWindow = mdi->getWindowById(windowId);
    if (mdiWindow==NULL) {
        mdiWindow = new PerfMonitorView();
        mdiWindow->setWindowIcon(QIcon(":perf_monitor/images/mon.png"));
        windowId = mdiWindow->getId();
        mdi->addMDIWindow(mdiWindow);
    }
    mdi->activateWindow(mdiWindow);

}

} //namespace
