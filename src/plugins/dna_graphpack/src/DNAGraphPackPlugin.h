#ifndef _U2_DNA_GRAPHPACK_PLUGIN_H_
#define _U2_DNA_GRAPHPACK_PLUGIN_H_

#include <U2Core/PluginModel.h>
#include <U2Gui/ObjectViewModel.h>

#include <U2View/GSequenceGraphView.h>
#include <U2View/ADVSequenceWidget.h>

#include <QtGui/QMenu>
#include <QtGui/QToolBar>
#include <QtCore/QPointer>

namespace U2 {

class MWMDIWindow;
class GObjectView;
class GSequenceGraphFactory;
class GSequenceGraphViewWithFactory;
class GSequenceGraphData;
class GraphAction;
class DNAGraphPackViewContext;
class ADVSingleSequenceWidget;

class DNAGraphPackPlugin : public Plugin {
    Q_OBJECT
public:
    DNAGraphPackPlugin();

private:
    DNAGraphPackViewContext*        ctx;
};

class DNAGraphPackViewContext : public GObjectViewWindowContext {
    Q_OBJECT
public:
    DNAGraphPackViewContext(QObject* p);

protected slots:
    void sl_sequenceWidgetAdded(ADVSequenceWidget*);
    void sl_handleGraphAction();

protected:
    virtual void initViewContext(GObjectView* view);

    QList<GSequenceGraphFactory*>   graphFactories;
};

class GraphMenuAction : public ADVSequenceWidgetAction {
    Q_OBJECT
public:
    GraphMenuAction();
};

class GraphAction : public QAction {
    Q_OBJECT
public:
    GraphAction(GSequenceGraphFactory* f);
    GSequenceGraphFactory*  factory;
    GSequenceGraphView*     view;
};


class GSequenceGraphFactory : public QObject{
public:
    GSequenceGraphFactory(const QString& _name, QObject* p) : QObject(p), graphName(_name){}

    virtual QList<GSequenceGraphData*> createGraphs(GSequenceGraphView* v) = 0;
    virtual GSequenceGraphDrawer* getDrawer(GSequenceGraphView* v) = 0;

    virtual bool isEnabled(DNASequenceObject* o) const  = 0;

    const QString& getGraphName() const {return graphName;}

protected:
    QString graphName;
};

class GSequenceGraphViewWithFactory : public GSequenceGraphView {
    Q_OBJECT
public:
    GSequenceGraphViewWithFactory(ADVSingleSequenceWidget* sw, GSequenceGraphFactory* f);
    GSequenceGraphFactory* getFactory() const {return f;}
private: 
    GSequenceGraphFactory* f;
};




} //namespace

#endif
