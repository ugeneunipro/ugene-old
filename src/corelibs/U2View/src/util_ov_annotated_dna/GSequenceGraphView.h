#ifndef _U2_GSEQUENCE_GRAPH_VIEW_H_
#define _U2_GSEQUENCE_GRAPH_VIEW_H_

#include "GSequenceLineView.h"
#include "ADVGraphModel.h"

#include <QtGui/QMenu>

namespace U2 {

class GSequenceGraphWindowData;
class GSequenceGraphView;
class GObjectView;

//////////////////////////////////////////////////////////////////////////
/// View
class U2VIEW_EXPORT GSequenceGraphView : public GSequenceLineView {
    Q_OBJECT
public:
    GSequenceGraphView(QWidget* p, ADVSequenceObjectContext* ctx, GSequenceLineView* baseView, const QString& vName);
    ~GSequenceGraphView();

    const QString& getGraphViewName() const {return vName;}

    void addGraphData(GSequenceGraphData* g);
    
    void setGraphDrawer(GSequenceGraphDrawer* gd);

    const QList<GSequenceGraphData*>& getGraphs() const {return graphs;}
    
    GSequenceGraphDrawer* getGSequenceGraphDrawer() const {return graphDrawer;}

    void buildPopupMenu(QMenu& m);

protected:
    virtual void pack();

private slots:
    void sl_onShowVisualProperties(bool);

private:
    GSequenceLineView*          baseView;
    QString                     vName;
    QList<GSequenceGraphData*>  graphs;
    GSequenceGraphDrawer*       graphDrawer;
    QAction*                    visualPropertiesAction;
};


class U2VIEW_EXPORT GSequenceGraphViewRA : public GSequenceLineViewRenderArea {
public:
    GSequenceGraphViewRA(GSequenceGraphView* g);
    ~GSequenceGraphViewRA();
    virtual GSequenceGraphView* getGraphView() const {return static_cast<GSequenceGraphView*>(view);}
    
    float getCurrentScale() const;

protected:
    virtual void drawAll(QPaintDevice* pd);
    virtual void drawHeader(QPainter& p);
    void drawSelection(QPainter& p);
private:

    QFont *headerFont;
    int headerHeight;
    QRect graphRect;
    GSequenceGraphDrawer* gd;

};

} // namespace

#endif
