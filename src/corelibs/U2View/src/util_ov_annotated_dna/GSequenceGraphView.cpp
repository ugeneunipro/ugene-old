#include "GSequenceGraphView.h"

#include "ADVSequenceObjectContext.h"
#include "ADVSingleSequenceWidget.h"

#include <U2Core/DNASequenceSelection.h>

#include <U2Misc/GScrollBar.h>
#include <QtGui/QVBoxLayout>


namespace U2 {
/* TRANSLATOR U2::GSequenceGraphView */    

GSequenceGraphView::GSequenceGraphView(QWidget* p, ADVSequenceObjectContext* ctx, GSequenceLineView* _baseView, const QString& _vName) 
: GSequenceLineView(p, ctx), baseView(_baseView), vName(_vName), graphDrawer(NULL)
{ 
    assert(baseView);

    
    visualPropertiesAction = new QAction(tr("Graph settings"), this);
    visualPropertiesAction->setObjectName("visual_properties_action");

    connect(visualPropertiesAction, SIGNAL(triggered(bool)), SLOT(sl_onShowVisualProperties(bool)));

    scrollBar->setDisabled(true);
    renderArea = new GSequenceGraphViewRA(this);

    visibleRange = baseView->getVisibleRange();
    setConherentRangeView(baseView);
    setFrameView(baseView->getFrameView());

    //process double clicks as centering requests
    ADVSingleSequenceWidget* ssw = baseView == NULL ? NULL : qobject_cast<ADVSingleSequenceWidget*>(baseView->parentWidget());
    if (ssw != NULL) {
        connect(this, SIGNAL(si_centerPosition(int)), ssw, SLOT(sl_onLocalCenteringRequest(int)));
    }

    pack();
}

void GSequenceGraphView::setGraphDrawer(GSequenceGraphDrawer* gd) {
    graphDrawer = gd;
    update();
}

void GSequenceGraphView::pack() {
    assert(layout() == NULL);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addWidget(renderArea);

    // use extra layout with scrollbar to have synchronized width with pan-view
    // in future fake-scroll-bar can be replaced with graph specific toolbar
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    QScrollBar* fakeBar = new QScrollBar(Qt::Vertical, this);
    fakeBar->setDisabled(true);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(fakeBar);

    setLayout(hLayout);

    scrollBar->setHidden(true); //todo: support mode without scrollbar at all??

    setFixedHeight(140);
}

GSequenceGraphView::~GSequenceGraphView() {
    foreach(GSequenceGraphData* g, graphs) {
        delete g;
    }
}
void GSequenceGraphView::addGraphData(GSequenceGraphData* g) {
    assert(!graphs.contains(g));
    graphs.append(g);
}

void GSequenceGraphView::buildPopupMenu(QMenu& m) {
    QPoint cpos = renderArea->mapFromGlobal(QCursor::pos());
    if (!renderArea->rect().contains(cpos)) {
        return;
    }
    m.addAction(visualPropertiesAction);
}


void GSequenceGraphView::sl_onShowVisualProperties(bool) {
    graphDrawer->showSettingsDialog();
}


//////////////////////////////////////////////////////////////////////////
// RA
GSequenceGraphViewRA::GSequenceGraphViewRA(GSequenceGraphView* g) : GSequenceLineViewRenderArea(g) {
    headerFont=  new QFont("Courier", 10);
    headerHeight = 20;
}

GSequenceGraphViewRA::~GSequenceGraphViewRA() {
    delete headerFont;
}

void GSequenceGraphViewRA::drawAll(QPaintDevice* pd) {
    //todo: use cached view here!!

    QPainter p(pd);
    p.fillRect(0, 0, pd->width(), pd->height(), Qt::white);
    p.setPen(Qt::black);

    graphRect = QRect(1, headerHeight+1, pd->width()-2, pd->height()-headerHeight-2);

    if (view->hasFocus()) {
        drawFocus(p);
    }



    gd = getGraphView()->getGSequenceGraphDrawer();
    assert(gd!=NULL);

    drawHeader(p);

    const QList<GSequenceGraphData*>& graphs = getGraphView()->getGraphs();
    foreach(GSequenceGraphData* d, graphs) {
        gd->draw(p, d, graphRect);
    }

    drawFrame(p);
    drawSelection(p);
}

void GSequenceGraphViewRA::drawHeader(QPainter& p) {
    p.setFont(*headerFont);

    const GSequenceGraphWindowData& wd = gd->getWindowData();
    const U2Region& visibleRange = view->getVisibleRange();
    QString text = GSequenceGraphView::tr("%1 [%2, %3], Window: %4, Step %5").arg(getGraphView()->getGraphViewName())
        .arg(QString::number(visibleRange.startPos+1)).arg(QString::number(visibleRange.endPos())).arg(QString::number(wd.window)).arg(QString::number(wd.step));
    QRect rect(1, 1, cachedView->width() - 2, headerHeight - 2);
    p.drawText(rect, Qt::AlignLeft, text);
}


float GSequenceGraphViewRA::getCurrentScale() const {
    return float(graphRect.width()) / view->getVisibleRange().length;
}


void GSequenceGraphViewRA::drawSelection(QPainter& p) {
    const QVector<U2Region>& selection = view->getSequenceContext()->getSequenceSelection()->getSelectedRegions();
    if (selection.isEmpty()) {
        return;
    }
    const U2Region& visibleRange = view->getVisibleRange();
    QPen pen1(Qt::darkGray, 1, Qt::SolidLine);
    foreach(const U2Region& r, selection) {
        if (!visibleRange.intersects(r)) {
            continue;
        }
        int x1 = graphRect.left() + qMax(0, posToCoord(r.startPos, true));
        int x2 = graphRect.left() + qMin(cachedView->width(), posToCoord(r.endPos(), true));

        p.setPen(pen1);
        if (visibleRange.contains(r.startPos)) {
            p.drawLine(x1, graphRect.top(), x1, graphRect.bottom());
        }
        if (visibleRange.contains(r.endPos())) {
            p.drawLine(x2, graphRect.top(), x2, graphRect.bottom());
        }
    }
}

} // namespace
