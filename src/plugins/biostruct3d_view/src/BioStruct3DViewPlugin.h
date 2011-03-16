#ifndef _U2_BIOSTRUCT3D_VIEW_PLUGIN_H_
#define _U2_BIOSTRUCT3D_VIEW_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QMap>
#include <QtGui/QAction>

namespace U2 {

class BioStruct3DSplitter;
class BioStruct3DObject;

class BioStruct3DViewPlugin : public Plugin {
    Q_OBJECT
public:
    BioStruct3DViewPlugin();
    ~BioStruct3DViewPlugin();
private:
    GObjectViewWindowContext* viewContext;
};

class BioStruct3DViewContext: public GObjectViewWindowContext {
    Q_OBJECT
    QMap<GObjectView*, BioStruct3DSplitter*> splitterMap;
public:
    BioStruct3DViewContext(QObject* p);
    
    virtual bool canHandle(GObjectView* v, GObject* o);

    virtual void onObjectAdded(GObjectView* v, GObject* obj);
    virtual void onObjectRemoved(GObjectView* v, GObject* obj);

protected:
    virtual void initViewContext(GObjectView* view);

    void remove3DView(GObjectView* view, BioStruct3DSplitter* view3d);

    QAction* getClose3DViewAction(GObjectView* view);


protected slots:
    void sl_close3DView();
    virtual void sl_windowClosing(MWMDIWindow*);

};

} //namespace

#endif
