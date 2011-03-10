#ifndef _U2_OVERVIEW_H_
#define _U2_OVERVIEW_H_

#include "GSequenceLineView.h"
#include "ADVSequenceObjectContext.h"

#include <QToolButton>

namespace U2 {

class PanView;
class DetView;
class OverviewRenderArea;
class AnnotationModification;

class Overview : public GSequenceLineView {
    Q_OBJECT
public:
    Overview(QWidget* p, ADVSequenceObjectContext* ctx);

protected slots:
    void sl_visibleRangeChanged();
    void sl_tbToggled();
    void sl_annotationsAdded(const QList<Annotation*>& a);
    void sl_annotationsRemoved(const QList<Annotation*>& a);
    void sl_annotationModified(const AnnotationModification& md);
    void sl_onAnnotationSettingsChanged(const QStringList& changedSettings);
    void sl_annotationObjectAdded(AnnotationTableObject* obj);
    void sl_annotationObjectRemoved(AnnotationTableObject* obj);
    void sl_sequenceChanged();
protected:
    void pack();
    virtual bool event(QEvent* e);
    void mousePressEvent(QMouseEvent *me);
    void mouseMoveEvent(QMouseEvent* me);
    void mouseDoubleClickEvent(QMouseEvent* me);
    void mouseReleaseEvent(QMouseEvent* me);
    void wheelEvent(QWheelEvent* we);

    QString createToolTip(QHelpEvent* he);
    PanView* getPan() const {return panView;};
    DetView* getDet() const {return detView;};

    bool        panSliderClicked;
    bool        detSliderClicked;
    bool        panSliderMovedRight;
    bool        panSliderMovedLeft;

    int         offset;
private:
    PanView*        panView;
    DetView*        detView;
    QPoint          mousePosToSlider;
    QToolButton*    tb;

friend class OverviewRenderArea;
};

class OverviewRenderArea : public GSequenceLineViewRenderArea {
    Q_OBJECT
public:
    OverviewRenderArea(Overview* p);

    const QRectF getPanSlider() const {return panSlider;};
    const QRectF getDetSlider() const {return detSlider;};

    int getAnnotationDensity(int pos) const;

    bool    showGraph;
protected:
    void drawAll(QPaintDevice* pd);
private:
    void drawRuler(QPainter& p);
    void drawSelection(QPainter& p);
    void drawSlider(QPainter& p, QRectF rect, QColor col);
    void drawArrow(QPainter& p, QRectF rect, QColor col);
    void setAnnotationsOnPos();
    void drawGraph(QPainter& p);
    QColor getUnitColor(int count);

    QRectF          panSlider;
    QRectF          detSlider;
    QBrush          gradientMaskBrush;
    QVector<int>    annotationsOnPos;
};

}//namespace

#endif
