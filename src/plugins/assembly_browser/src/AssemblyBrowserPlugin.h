#ifndef __ASSEMBLY_BROWSER_PLUGIN__
#define __ASSEMBLY_BROWSER_PLUGIN__

#include <U2Core/PluginModel.h>
#include <U2Core/GObjectReference.h>
#include <U2Gui/ObjectViewModel.h>
#include <U2Gui/ObjectViewTasks.h>

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


class AssemblyBrowserFactory : public GObjectViewFactory {
    Q_OBJECT
public:
    AssemblyBrowserFactory(QObject * parent = 0);

    virtual bool canCreateView(const MultiGSelection & multiSelection);
    virtual Task * createViewTask(const MultiGSelection & multiSelection, bool single = false);

    static const GObjectViewFactoryId ID;
};

class AssemblyObject;
class UnloadedObject;
class Document;

class OpenAssemblyBrowserTask : public ObjectViewTask {
    Q_OBJECT
public:
    OpenAssemblyBrowserTask(AssemblyObject * obj);
    OpenAssemblyBrowserTask(UnloadedObject * obj);
    OpenAssemblyBrowserTask(Document * doc);
    virtual void open();
private:
    GObjectReference unloadedObjRef;
};

}

#endif // __ASSEMBLY_BROWSER_PLUGIN__
