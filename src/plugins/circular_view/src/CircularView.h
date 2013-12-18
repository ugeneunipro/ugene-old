/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#ifndef _U2_CIRCULAR_PAN_VIEW_H_
#define _U2_CIRCULAR_PAN_VIEW_H_

#include <U2Core/Annotation.h>
#include <U2Core/GObject.h>
#include <U2Gui/MainWindow.h>
#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>
#include <U2Core/Task.h>

#include <U2View/ADVConstants.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/GSequenceLineViewAnnotated.h>

#include <QtGui/QFont>
#include <QtGui/QAction>
#include <QtGui/QScrollBar>


namespace U2 {

class CircularViewRenderArea;
class CircularAnnotationItem;
class CircularAnnotationLabel;
class TextItem;

class CircularView : public GSequenceLineViewAnnotated {
    Q_OBJECT
public:
    CircularView(QWidget* p, ADVSequenceObjectContext* ctx);
    void pack();
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);
    void wheelEvent(QWheelEvent* we);
    virtual QSize sizeHint() const;

    virtual QList<AnnotationSelectionData> selectAnnotationByCoord(const QPoint& coord) const;

    static qreal coordToAngle(const QPoint point);

    const QMap<Annotation,CircularAnnotationItem*>& getCircularItems() const;
    const QList<CircularAnnotationLabel*>& getLabelList() const;

    bool isCircularTopology() { return true; }

    enum Direction {CW, CCW, UNKNOWN};

    static const int MIN_OUTER_SIZE;
    static const int CV_REGION_ITEM_WIDTH;

    void setAngle(int angle);
    void updateMinHeight();

    //used by export to file function
    void paint(QPainter& p);
signals:
    void si_wheelMoved(int);
    void si_zoomInDisabled(bool);
    void si_zoomOutDisabled(bool);
    void si_fitInViewDisabled(bool);
public slots:
    void sl_zoomIn();
    void sl_zoomOut();
    void sl_fitInView();
    void sl_onSequenceObjectRenamed(const QString& oldName);

protected slots: 
    virtual void sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation>& added, const QList<Annotation>& removed);
    virtual void sl_onDNASelectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);

protected:
    void adaptSizes();
    void updateZoomActions();

    Direction getDirection(float a, float b) const;

    QVBoxLayout *layout;

    int lastMovePos;
    int currectSelectionLen;
    int lastMouseY;
    CircularViewRenderArea* ra;
    bool clockwise, holdSelection;

private:
    static const int graduation;
};

class CircularViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
    friend class CircularView;
    friend class CircularAnnotationItem;
    friend class CircularAnnotationLabel;
    friend class CircurlarAnnotationRegionItem;
    Q_OBJECT
public:
    CircularViewRenderArea(CircularView* d);
    ~CircularViewRenderArea();

    int getAnnotationYLevel (const Annotation &a) const { return annotationYLevel.value(a); }
    void adaptNumberOfLabels(int h);

    static const int MIDDLE_ELLIPSE_SIZE;
    int getCenterY() const { return verticalOffset; }
protected:
    void resizeEvent(QResizeEvent *e);
    virtual void drawAll(QPaintDevice* pd);
    virtual U2Region getAnnotationYRange( const Annotation &a, int ri, const AnnotationSettings* as) const;
    void buildAnnotationItem(DrawAnnotationPass pass, const Annotation &a, bool selected = false, const AnnotationSettings* as = NULL);
    virtual void drawAnnotations(QPainter& p);
    void paintContent(QPainter& p);
    void buildAnnotationLabel(const QFont& font, const Annotation &a, const AnnotationSettings* as);
    void drawSequenceName(QPainter& p);
    void drawRuler(QPainter& p);
    void drawRulerNotches(QPainter& p, int start, int span, int seqLen);
    void drawAnnotationsSelection(QPainter& p);
    void drawSequenceSelection(QPainter& p);
    void drawMarker(QPainter& p);
    void evaluateLabelPositions();
    qreal getVisibleAngle() const;
    QPair<int,int> getVisibleRange() const;

private:
    static const int OUTER_ELLIPSE_SIZE;
    static const int ELLIPSE_DELTA;
    static const int INNER_ELLIPSE_SIZE;
    static const int RULER_ELLIPSE_SIZE;

    static const int ARROW_LENGTH;
    static const int ARROW_HEIGHT_DELTA;
    static const int MAX_DISPLAYING_LABELS;

    static const int MARKER_LEN;
    static const int ARR_LEN;
    static const int ARR_WIDTH;
    static const int NOTCH_SIZE;

    int outerEllipseSize;
    int ellipseDelta;
    int innerEllipseSize;
    int rulerEllipseSize;
    int middleEllipseSize;
    int arrowLength;
    int arrowHeightDelta;
    int maxDisplayingLabels;
    bool fitsInView;
    int verticalOffset;

    CircularView* circularView;
    QList< QVector<U2Region> > regionY;
    QMap<Annotation, CircularAnnotationItem* > circItems;
    TextItem* seqNameItem;
    TextItem* seqLenItem;
    QMap<Annotation, int> annotationYLevel;
    QList<CircularAnnotationLabel*> labelList;
    qreal rotationDegree;
    qreal mouseAngle;
    QVector<QRect> labelEmptyPositions;
    QVector<QRect> labelEmptyInnerPositions;
    int oldYlevel;
};

}//namespace;

#endif
