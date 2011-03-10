#ifndef _U2_CIRCULAR_VIEW_SPLITTER_H_
#define _U2_CIRCULAR_VIEW_SPLITTER_H_

#include "CircularView.h"
#include "CircularItems.h"

#include <U2View/ADVSplitWidget.h>

#include <QToolBar>
#include <QToolButton>
#include <QAction>


class QScrollBar;

namespace U2 {

class CircularViewHeaderWidget;
class HBar;

class CircularViewSplitter : public ADVSplitWidget{
    Q_OBJECT
public:
    CircularViewSplitter( AnnotatedDNAView* view);
    // there are no special object handling with this view
    // it only shows existing AO only
    virtual bool acceptsGObject(GObject*) {return false;}
    virtual void updateState(const QVariantMap& m);
    virtual void saveState(QVariantMap& m);
    void addView(CircularView* view);
    void adaptSize();
    void removeView(CircularView* view);
    bool isEmpty();
    const QList<CircularView*>& getViewList() const { return circularViewList; }
protected slots:
    void sl_export();
    void sl_horSliderMoved(int);
    void sl_moveSlider(int);
    
    void sl_updateZoomInAction(bool);
    void sl_updateZoomOutAction(bool);
    void sl_updateFitInViewAction(bool);
private:
    QSplitter* splitter;
    HBar* toolBar;    
    QScrollBar* horScroll;

    QToolButton* tbZoomIn;
    QToolButton* tbZoomOut;
    QToolButton* tbFitInView;
    QToolButton* tbExport;

    QList<CircularView*> circularViewList;
};

} //namespace U2

#endif //_U2_CIRCULAR_VIEW_SPLITTER_H_
