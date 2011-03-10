#ifndef __ASSEMBLY_BROWSER_PLUGIN__
#define __ASSEMBLY_BROWSER_PLUGIN__

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

namespace U2 {


class AssemblyBrowserPlugin : public Plugin {
    Q_OBJECT
public:
    AssemblyBrowserPlugin();
public slots:
    void sl_showWindow();
protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);
private:
    QAction * windowAction;
    MWMDIWindow * window;
};


}

#endif // __ASSEMBLY_BROWSER_PLUGIN__
