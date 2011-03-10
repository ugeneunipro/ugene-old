#ifndef _U2_CHROMA_VIEW_PLUGIN_H_
#define _U2_CHROMA_VIEW_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Gui/ObjectViewModel.h>

#include <QtCore/QMap>
#include <QtGui/QAction>

#include <U2View/ADVSequenceWidget.h>

namespace U2 {

class ChromatogramView;

class ChromaViewPlugin : public Plugin {
    Q_OBJECT
public:
    ChromaViewPlugin();
    ~ChromaViewPlugin();
private:
    GObjectViewWindowContext* viewCtx;
};

class ChromaViewContext: public GObjectViewWindowContext {
    Q_OBJECT
public:
    ChromaViewContext(QObject* p);
protected slots:
    void sl_showChromatogram();
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
    
protected:
    virtual void initViewContext(GObjectView* view);
};    

class ChromaViewAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    ChromaViewAction();
    ~ChromaViewAction();
    ChromatogramView* view;
};

} //namespace

#endif
