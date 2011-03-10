#include <U2Core/AppContext.h>
#include <QtGui/QtEvents>

#include "AssemblyBrowser.h"

#include "AssemblyBrowserPlugin.h"

namespace U2 {

extern "C" Q_DECL_EXPORT Plugin* U2_PLUGIN_INIT_FUNC() {
    AssemblyBrowserPlugin * plug = new AssemblyBrowserPlugin();
    return plug;
}

AssemblyBrowserPlugin::AssemblyBrowserPlugin() : 
Plugin(tr("Assembly Browser"), tr("Visualization of enormous genome assemblies.")), window(0) {
    if(AppContext::isGUIMode()) {
        windowAction = new QAction(tr("Assembly Browser"), this);
        connect(windowAction, SIGNAL(triggered()), SLOT(sl_showWindow()));
        AppContext::getMainWindow()->getTopLevelMenu(MWMENU_TOOLS)->addAction(windowAction);
    }
}

void AssemblyBrowserPlugin::sl_showWindow() {
    if(!window) {
        window = new AssemblyBrowserWindow();
        window->installEventFilter(this);
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(window);
    }
    AppContext::getMainWindow()->getMDIManager()->activateWindow(window);
}

bool AssemblyBrowserPlugin::eventFilter(QObject *obj, QEvent *event) {
    if(obj == window && QEvent::Close == event->type()) {
        window = 0;
    }
    return QObject::eventFilter(obj, event);
}

} //ns
