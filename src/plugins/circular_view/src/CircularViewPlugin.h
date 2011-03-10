#ifndef _U2_CIRCULAR_VIEW_PLUGIN_H_
#define _U2_CIRCULAR_VIEW_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QMap>
#include <QtGui/QAction>

#include <U2View/ADVSequenceWidget.h>

namespace U2 {

class CircularViewSplitter;
class CircularView;

class CircularViewPlugin : public Plugin {
    Q_OBJECT
public:
    CircularViewPlugin();
    ~CircularViewPlugin();
private:
    GObjectViewWindowContext* viewCtx;
};

class CircularViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    CircularViewContext(QObject* p);
protected slots:
    void sl_showCircular();
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
    
protected:
    virtual void initViewContext(GObjectView* view);
    void buildMenu(GObjectView* v, QMenu* m);
    CircularViewSplitter* getView(GObjectView* view, bool create);
    void removeCircularView(GObjectView* view);
private:
    GObjectViewAction* exportAction;
};    

class CircularViewAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    CircularViewAction();
    ~CircularViewAction();
    CircularView* view;
};

} //namespace

#endif
