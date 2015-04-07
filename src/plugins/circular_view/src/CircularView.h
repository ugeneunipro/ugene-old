/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QAction>
#include <QtGui/QScrollBar>
#else
#include <QtWidgets/QAction>
#include <QtWidgets/QScrollBar>
#endif

namespace U2 {

class CircularViewRenderArea;
class CircularAnnotationItem;
class CircularAnnotationLabel;
class CircularAnnotationRegionItem;
struct CircularViewSettings;
class TextItem;

class CircularView : public GSequenceLineViewAnnotated {
    Q_OBJECT
public:
    CircularView(QWidget* p, ADVSequenceObjectContext* ctx, CircularViewSettings* settings);
    void pack();
    void mousePressEvent(QMouseEvent * e);
    void mouseMoveEvent(QMouseEvent * e);
    void mouseReleaseEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

    void wheelEvent(QWheelEvent* we);
    virtual QSize sizeHint() const;

    virtual QList<AnnotationSelectionData> selectAnnotationByCoord(const QPoint& coord) const;

    static qreal coordToAngle(const QPoint point);

    const QMap<Annotation *,CircularAnnotationItem *> & getCircularItems() const;
    const QList<CircularAnnotationLabel *> & getLabelList() const;

    bool isCircularTopology() const;

    enum Direction {CW, CCW, UNKNOWN};

    static const int MIN_OUTER_SIZE;
    static const int CV_REGION_ITEM_WIDTH;

    void setAngle(int angle);
    void updateMinSize();

    //used by export to file function
    void paint(QPainter &p, int w, int h, bool paintSelection, bool paintMarker);

    CircularViewSettings* getSettings() { return settings; }
    void redraw();

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
    void sl_onCircularTopologyChange();

protected slots:
    virtual void sl_onAnnotationSelectionChanged(AnnotationSelection *, const QList<Annotation *> &added, const QList<Annotation *> &removed);
    virtual void sl_onDNASelectionChanged(LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed);

protected:
    void adaptSizes();
    void updateZoomActions();

    void setInverseSelection(const U2Region &r);
    /**
     * Used for region that covers the beginning of the sequence and the end of it.
     * Usage: @startSeqRegion.startPos should be 0 and @endSeqRegion.endPos should be equal to sequence length.
     */
    void setInverseSelection(const U2Region &startSeqRegion, const U2Region &endSeqRegion);
    /**
     * Use for continuous region selection only.
     * TODO: if inverse selection function would be fully available (for splitted kind of selection), rewrite it.
     */
    void invertCurrentSelection();

    Direction getDirection(float a, float b) const;

    QVBoxLayout *layout;

    int lastMovePos;
    int currectSelectionLen;
    int lastMouseY;
    CircularViewRenderArea* ra;
    bool clockwise, holdSelection;
    CircularViewSettings*   settings;

private:
    static const int graduation;
};

class CircularViewRenderArea : public GSequenceLineViewAnnotatedRenderArea {
    friend class CircularView;
    friend class CircularAnnotationItem;
    friend class CircularAnnotationLabel;
    friend class CircularAnnotationRegionItem;
    Q_OBJECT
public:
    CircularViewRenderArea(CircularView* d);
    ~CircularViewRenderArea();

    int getAnnotationYLevel(Annotation *a) const { return annotationYLevel.value(a); }
    void adaptNumberOfLabels(int h);

    static const int MIDDLE_ELLIPSE_SIZE;
    int getCenterY() const { return verticalOffset; }
protected:
    void resizeEvent(QResizeEvent *e);
    virtual void drawAll(QPaintDevice* pd);
    virtual U2Region getAnnotationYRange(Annotation *a, int ri, const AnnotationSettings *as) const;
    virtual U2Region getMirroredYRange(const U2Strand &mirroredStrand) const {Q_UNUSED(mirroredStrand); return U2Region(-1,0);}
    void buildAnnotationItem(DrawAnnotationPass pass, Annotation *a, bool selected = false, const AnnotationSettings *as = NULL);
    virtual void drawAnnotations(QPainter& p);

    void redraw();

    void paintContent(QPainter& p, bool paintSelection = true, bool paintMarker = true);
    void paintContent(QPainter &p, int w, int h, bool paintSelection, bool paintMarker);

    void buildAnnotationLabel( const QFont &font, Annotation *a, const AnnotationSettings *as, bool isAutoAnnotation = false);
    void drawSequenceName(QPainter& p);
    void drawRuler(QPainter& p);
    void drawRulerCoordinates(QPainter& p, int startPos, int seqLen);
    void drawRulerNotches(QPainter& p, int start, int span, int seqLen);
    void drawAnnotationsSelection(QPainter& p);
    void drawSequenceSelection(QPainter& p);
    void drawMarker(QPainter& p);
    void evaluateLabelPositions();
    qreal getVisibleAngle() const;
    QPair<int,int> getVisibleRange() const;

private:
    int findOrbit(const QVector<U2Region>& location, Annotation *a);
    CircularAnnotationRegionItem* createAnnotationRegionItem(const U2Region &region, int seqLen, int yLevel, const SharedAnnotationData &aData, int index);
    QPainterPath createAnnotationArrowPath(float startAngle, float spanAngle, float dAlpha, const QRect &outerRect, const QRect &innerRect,
        const QRect &middleRect, bool complementary, bool isShort) const;
    void removeRegionsOutOfRange(QVector<U2Region> &location, int seqLen) const;

    /**
     * Returns a pair of merged regions: the second one was added to the first
     */
    QPair<U2Region, U2Region> mergeCircularJunctoinRegion(QVector<U2Region> &location, int seqLen) const;

    static const int OUTER_ELLIPSE_SIZE;
    static const int ELLIPSE_DELTA;
    static const int INNER_ELLIPSE_SIZE;
    static const int RULER_ELLIPSE_SIZE;

    static const int ARROW_LENGTH;
    static const int ARROW_HEIGHT_DELTA;
    static const int MAX_DISPLAYING_LABELS;
    static const int MAX_LABEL_WIDTH;
    static const int FREE_SPACE_HEIGHT_FOR_INTERNAL_LABELS;

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
    int verticalOffset;
    int currentScale; // > 0 -- zoomed IN; < 0 -- zoomed OUT; == 0 -- fits in view

    CircularViewSettings*   settings;
    bool settingsWereChanged;

    CircularView* circularView;
    QList<QVector<U2Region> > regionY;
    QMap<Annotation *, CircularAnnotationItem *> circItems;
    TextItem* seqNameItem;
    TextItem* seqLenItem;
    QMap<Annotation *, int> annotationYLevel;
    QList<CircularAnnotationLabel*> labelList;
    qreal rotationDegree;
    qreal mouseAngle;
    QVector<QRect> labelEmptyPositions;
    int oldYlevel;
};

}//namespace;

#endif
