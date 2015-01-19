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

#include <math.h>

#include <QtGui/QFontMetrics>
#include <QtGui/QPainter>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QTextEdit>
#else
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QTextEdit>
#endif

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Core/FormatUtils.h>
#include <U2Core/Log.h>
#include <U2Core/U1AnnotationUtils.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GraphUtils.h>
#include <U2Gui/GScrollBar.h>

#include <U2View/ADVSequenceWidget.h>

#include "CircularItems.h"
#include "CircularView.h"
#include "CircularViewPlugin.h"


namespace U2 {

const int CircularView::CV_REGION_ITEM_WIDTH = 25;

const int CircularView::MIN_OUTER_SIZE = 100;
const int CircularView::graduation = 16;



CircularView::CircularView(QWidget* p, ADVSequenceObjectContext* ctx, CircularViewSettings* settings)
: GSequenceLineViewAnnotated(p, ctx), clockwise(true), holdSelection(false), settings(settings)
{
    QSet<AnnotationTableObject*> anns = ctx->getAnnotationObjects(true);
    foreach(AnnotationTableObject* obj, anns ) {
        registerAnnotations(obj->getAnnotations());
    }

    renderArea = new CircularViewRenderArea(this);
    ra = qobject_cast<CircularViewRenderArea*>(renderArea);
    setMouseTracking(true);

    connect(ctx->getSequenceGObject(), SIGNAL(si_nameChanged(const QString&)), this, SLOT(sl_onSequenceObjectRenamed(const QString&)));
    pack();
}

void CircularView::pack() {
    updateMinSize();
    layout = new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(renderArea);
    setLayout(layout);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

void CircularView::updateMinSize() {
    int min = (ra->regionY.count() - 1)*ra->ellipseDelta + MIN_OUTER_SIZE;
    setMinimumSize( min, min);
}


void CircularView::mousePressEvent(QMouseEvent * e) {
    GSequenceLineViewAnnotated::mousePressEvent(e);
    QPoint p = toRenderAreaPoint(e->pos());
    int offset = ra->getCenterY();
    QPoint point(p.x() - width()/2 , p.y() - offset);
    qreal arcsin = coordToAngle(point);
    lastPressPos = 180 * graduation * arcsin / PI;
    lastPressPos-=ra->rotationDegree*graduation;
    if(lastPressPos<0) {
        lastPressPos+=360*graduation;
    }
    lastMovePos = lastPressPos;
    lastMouseY = point.y();
    currectSelectionLen = 0;

    holdSelection = false;

    QWidget::mousePressEvent(e);
}

CircularView::Direction CircularView::getDirection(float a, float b) const {
    if (a == b) {
        return UNKNOWN;
    }

    a/=graduation;
    b/=graduation;

    bool cl = ( (a-b>=180) || ((b-a <=180) && (b-a >=0)) );

    if (cl) {
        return CW;
    }
    else {
        return CCW;
    }
}

void CircularView::mouseMoveEvent( QMouseEvent * e )
{
    QWidget::mouseMoveEvent(e);

    QPoint areaPoint = toRenderAreaPoint(e->pos());
    QPoint point(areaPoint.x() - width()/2 , areaPoint.y() - ra->getCenterY());
    qreal arcsin = coordToAngle(point);
    ra->mouseAngle = arcsin;

    if (e->buttons() & Qt::LeftButton) {
        float a = 180 * graduation * arcsin / PI;
        a-=ra->rotationDegree*graduation;
        if(a<0) {
            a+=360*graduation;
        }

        Direction pressMove = getDirection(lastPressPos, lastMovePos);
        Direction moveA = getDirection(lastMovePos, a);

        float totalLen = qAbs(lastPressPos-lastMovePos) + qAbs(lastMovePos-a);
        totalLen/=graduation;

        if ((totalLen<10) && !holdSelection) {
            if ((pressMove!=CW) && (moveA!=CW)) {
                clockwise = false;
            }
            else if ((pressMove != CCW) && (moveA != CCW)) {
                clockwise = true;
            }

            if (totalLen < 1) {
                clockwise = lastPressPos < a;
            }

            holdSelection = true;
        }

        if (!clockwise) {
            float tmp = a;
            a = lastPressPos;
            lastPressPos = tmp;
        }

        // compute selection
        int seqLen = ctx->getSequenceLength();
        int selStart = lastPressPos / (360.0*graduation) * seqLen + 0.5f;
        int selEnd = a / (360.0*graduation) * seqLen + 0.5f;
        int selLen = selEnd-selStart;

        bool twoParts = false;
        if (selLen < 0) {    // 'a' and 'lastPressPos' are swapped so it should be positive
            selLen = selEnd + seqLen - selStart;
            Q_ASSERT(selLen>=0);

            if (selEnd) {    // [0, selEnd]
                twoParts = true;
            }
        }

        if (selLen > seqLen-selStart) {
            selLen = seqLen-selStart;
        }

        if (!clockwise) {
            float tmp = lastPressPos;
            lastPressPos = a;
            a = tmp;
        }

        lastMovePos = a;
        lastMouseY = point.y();

        if (twoParts) {
            if (e->modifiers() & Qt::ControlModifier ) {
                setInverseSelection(U2Region(0, selEnd), U2Region(selStart, seqLen - selStart));
            } else {
                setSelection(U2Region(0, selEnd));
                addSelection(U2Region(selStart, seqLen-selStart));
            }
        }
        else {
            if (e->modifiers() & Qt::ControlModifier) {
                setInverseSelection(U2Region(selStart, selLen));
            } else {
                setSelection(U2Region(selStart, selLen));
            }
        }
    }
    renderArea->update();
}

void CircularView::mouseReleaseEvent(QMouseEvent* e) {
    GSequenceLineViewAnnotated::mouseReleaseEvent(e);
}

void CircularView::setAngle(int angle) {
    assert(angle>=0 && angle<=360);
    ra->rotationDegree=angle;
    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    renderArea->update();
}

void CircularView::sl_onAnnotationSelectionChanged(AnnotationSelection* selection,
    const QList<Annotation>& added, const QList<Annotation>& removed)
{
    GSequenceLineViewAnnotated::sl_onAnnotationSelectionChanged(selection, added, removed);
    renderArea->update();
}

void CircularView::sl_onDNASelectionChanged( LRegionsSelection* thiz, const QVector<U2Region>& added, const QVector<U2Region>& removed )
{
    GSequenceLineViewAnnotated::sl_onDNASelectionChanged(thiz, added, removed);
    renderArea->update();
}

QList<AnnotationSelectionData> CircularView::selectAnnotationByCoord( const QPoint& coord ) const
{
    QList<AnnotationSelectionData> res;
    CircularViewRenderArea* renderArea = qobject_cast<CircularViewRenderArea*>(this->renderArea);
    QPoint cp(coord - QPoint(width()/2, renderArea->getCenterY()));
    foreach(CircularAnnotationItem* item, renderArea->circItems) {

        CircularAnnotationRegionItem* regItem = item->getContainingRegion(cp);
        int region = item->containsRegion(cp);
        if(region != -1) {
            QList<int> indx;
            indx << region;
            if (regItem != NULL && regItem->hasJoinedRegion()) {
                indx << item->getAnnotation().getRegions().indexOf(regItem->getJoinedRegion());
            }

            res.append(AnnotationSelectionData(item->getAnnotation(), indx));
            return res;
        }
    }
    foreach(CircularAnnotationItem* item, renderArea->circItems) {
        foreach(CircularAnnotationRegionItem* r, item->getRegions()) {
            CircularAnnotationLabel* lbl = r->getLabel();
            SAFE_POINT(lbl != NULL, "NULL annotation label item!", res);
            if(lbl->isVisible() && lbl->contains(cp)) {
                QList <int> indx;
                indx << r->getNumber();
                if (r->hasJoinedRegion()) {
                    indx << item->getAnnotation().getRegions().indexOf(r->getJoinedRegion());
                }
                res.append(AnnotationSelectionData(item->getAnnotation(),
                                                   indx));
                return res;
            }
        }
    }
    return res;
}

QSize CircularView::sizeHint() const {
    return ra->size();
}

const QMap<Annotation,CircularAnnotationItem*>& CircularView::getCircularItems() const {
    return ra->circItems;
}

const QList<CircularAnnotationLabel*>& CircularView::getLabelList() const {
    return ra->labelList;
}

bool CircularView::isCircularTopology() const {
    SAFE_POINT(ctx->getSequenceObject() != NULL, "Sequence object is NULL", false);
    return ctx->getSequenceObject()->isCircular();
}

void CircularView::wheelEvent(QWheelEvent* we) {
    if (we->modifiers()&Qt::ControlModifier) {
        if (we->delta()>0) {
            sl_zoomIn();
        } else {
            sl_zoomOut();
        }
    } else {
        emit si_wheelMoved(we->delta());
    }
    QWidget::wheelEvent(we);
}

void CircularView::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control && QApplication::mouseButtons() & Qt::LeftButton) {
        invertCurrentSelection();
    }
}

void CircularView::keyReleaseEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Control && QApplication::mouseButtons() & Qt::LeftButton) {
        invertCurrentSelection();
    }
}


void CircularView::resizeEvent( QResizeEvent* e ) {
    if (ra->currentScale == 0) {
        sl_fitInView();
    }
    QWidget::resizeEvent(e);
}

#define VIEW_MARGIN 10
void CircularView::sl_fitInView() {
    int yLvl = ra->regionY.count() - 1;
    ra->outerEllipseSize = qMin(height(), width()) - ra->ellipseDelta*yLvl - VIEW_MARGIN;
    ra->currentScale = 0;
    adaptSizes();
    updateZoomActions();
}

#define ZOOM_SCALE 1.2
void CircularView::sl_zoomIn() {
    if (ra->outerEllipseSize / width() > 10) {
        return;
    }
    ra->outerEllipseSize *= ZOOM_SCALE;
    ra->currentScale++;
    adaptSizes();
    updateZoomActions();
}

void CircularView::sl_zoomOut() {
    if (ra->outerEllipseSize / ZOOM_SCALE < MIN_OUTER_SIZE) {
        return;
    }
    ra->outerEllipseSize /= ZOOM_SCALE;
    ra->currentScale--;
    adaptSizes();
    updateZoomActions();
}

void CircularView::sl_onSequenceObjectRenamed(const QString&) {
    update();
}

void CircularView::updateZoomActions() {
    if (ra->outerEllipseSize * ZOOM_SCALE / width() > 10) {
        emit si_zoomInDisabled(true);
    } else {
        emit si_zoomInDisabled(false);
    }
    emit si_fitInViewDisabled(ra->currentScale == 0);

    if (ra->outerEllipseSize / ZOOM_SCALE < MIN_OUTER_SIZE) {
        emit si_zoomOutDisabled(true);
    } else {
        emit si_zoomOutDisabled(false);
    }
}

void CircularView::setInverseSelection(const U2Region &r) {
    U2Region part1(0, r.startPos);
    setSelection(part1);
    addSelection(U2Region( r.endPos(), ctx->getSequenceLength() - r.endPos() ));
}

void CircularView::setInverseSelection(const U2Region &startSeqRegion, const U2Region &endSeqRegion) {
    SAFE_POINT(startSeqRegion.startPos == 0 && endSeqRegion.endPos() == ctx->getSequenceLength(), "Invalid regions selection", );
    setSelection(U2Region(startSeqRegion.endPos(), endSeqRegion.startPos - startSeqRegion.endPos()));
}

void CircularView::invertCurrentSelection() {
    DNASequenceSelection* selection = ctx->getSequenceSelection();
    SAFE_POINT(selection != NULL, "Sequence selection is NULL", );
    CHECK(!selection->isEmpty(), );

    QVector<U2Region> selRegions = selection->getSelectedRegions();
    CHECK(selRegions.size() < 3, );
    if (selRegions.size() == 1) {
        setInverseSelection(selRegions.first());
    } else {
        if (selRegions.first().startPos == 0 && selRegions.last().endPos() == ctx->getSequenceLength()) {
            setInverseSelection(selRegions.first(), selRegions.last());
        } else {
            setInverseSelection(selRegions.last(), selRegions.first());
        }
    }
}

void CircularView::adaptSizes() {
    ra->innerEllipseSize = ra->outerEllipseSize - CV_REGION_ITEM_WIDTH;
    ra->rulerEllipseSize = ra->outerEllipseSize - CV_REGION_ITEM_WIDTH;
    ra->middleEllipseSize = (ra->outerEllipseSize + ra->innerEllipseSize)/2;
    updateMinSize();
    addUpdateFlags(GSLV_UF_NeedCompleteRedraw);
    ra->update();
}

qreal CircularView::coordToAngle(const QPoint point) {
    float norm = sqrt((double)point.x()*point.x() + point.y()*point.y());
    float arcsin = 0.0;
    if (abs(norm) > 1.0)
    {
        arcsin = asin(abs((double)point.y())/norm);
    }

    if(point.x() < 0) {
        arcsin = PI - arcsin;
    }
    if(point.y() < 0) {
        arcsin = 2*PI - arcsin;
    }
    return arcsin;
}

void CircularView::paint(QPainter &p, int w, int h, bool paintSelection, bool paintMarker) {
    ra->paintContent(p, w, h, paintSelection, paintMarker);
}

void CircularView::redraw() {
    ra->redraw();
}

/************************************************************************/
/* CircularViewRenderArea                                               */
/************************************************************************/

const int CircularViewRenderArea::OUTER_ELLIPSE_SIZE = 512;
const int CircularViewRenderArea::ELLIPSE_DELTA = CircularView::CV_REGION_ITEM_WIDTH + 2;

const int CircularViewRenderArea::INNER_ELLIPSE_SIZE = OUTER_ELLIPSE_SIZE - CircularView::CV_REGION_ITEM_WIDTH;
const int CircularViewRenderArea::RULER_ELLIPSE_SIZE = INNER_ELLIPSE_SIZE - CircularView::CV_REGION_ITEM_WIDTH;
const int CircularViewRenderArea::MIDDLE_ELLIPSE_SIZE = (INNER_ELLIPSE_SIZE + OUTER_ELLIPSE_SIZE) / 2;
const int CircularViewRenderArea::ARROW_LENGTH = 32;
const int CircularViewRenderArea::ARROW_HEIGHT_DELTA = 4;
const int CircularViewRenderArea::MAX_DISPLAYING_LABELS = 20;
const int CircularViewRenderArea::MAX_LABEL_WIDTH = 80;
const int CircularViewRenderArea::FREE_SPACE_HEIGHT_FOR_INTERNAL_LABELS = 4;

const int CircularViewRenderArea::MARKER_LEN = 30;
const int CircularViewRenderArea::ARR_LEN = 4;
const int CircularViewRenderArea::ARR_WIDTH = 10;
const int CircularViewRenderArea::NOTCH_SIZE = 5;


CircularViewRenderArea::CircularViewRenderArea(CircularView* d)
    : GSequenceLineViewAnnotatedRenderArea(d, true),
      outerEllipseSize(OUTER_ELLIPSE_SIZE),
      ellipseDelta(ELLIPSE_DELTA),
      innerEllipseSize(INNER_ELLIPSE_SIZE),
      rulerEllipseSize(RULER_ELLIPSE_SIZE),
      middleEllipseSize(MIDDLE_ELLIPSE_SIZE),
      arrowLength(ARROW_LENGTH),
      arrowHeightDelta(ARROW_HEIGHT_DELTA),
      maxDisplayingLabels(MAX_DISPLAYING_LABELS),
      currentScale(0),
      circularView(d),
      rotationDegree(0),
      mouseAngle(0),
      oldYlevel(0)
{
    SAFE_POINT(d != NULL, tr("CircularView is NULL"), );
    settings = d->getSettings();
    SAFE_POINT(settings != NULL, tr("Circular view settings are NULL"), );
    settingsWereChanged = false;

    setMouseTracking(true);

    ADVSequenceObjectContext* ctx = view->getSequenceContext();

    //build annotation items to get number of region levels for proper resize
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    QSet<AnnotationTableObject *> anns = ctx->getAnnotationObjects(true);
    foreach ( AnnotationTableObject *ao, anns) {
        foreach ( const Annotation &a, ao->getAnnotations( ) ) {
            AnnotationSettings* as = asr->getAnnotationSettings(a.getName());
            buildAnnotationItem(DrawAnnotationPass_DrawFill, a, false, as);
        }
    }
}

void CircularViewRenderArea::adaptNumberOfLabels(int h) {

    QFont font;
    QFontMetrics fm(font, this);

    int lblHeight = fm.height();
    maxDisplayingLabels = int(h/lblHeight);
}

void CircularViewRenderArea::paintContent( QPainter& p, bool paintSelection, bool paintMarker) {
    int viewSize = qMin(circularView->height(), circularView->width());
    uiLog.details(tr("circular view size %1 %2").arg(circularView->width()).arg(circularView->height()));
    verticalOffset = parentWidget()->height()/2;
    if (outerEllipseSize + (regionY.count()-1)*ellipseDelta + VIEW_MARGIN > viewSize) {
        verticalOffset += rulerEllipseSize/2;
    }

    p.fillRect(0, 0, width(), height(), Qt::white);

    p.save();
    p.translate(parentWidget()->width()/2, verticalOffset);

    drawRuler(p);
    drawAnnotations(p);
    if (settings->showTitle || settings->showLength) {
        drawSequenceName(p);
    }
    if (paintSelection) {
        drawAnnotationsSelection(p);
        drawSequenceSelection(p);
    }
    if (paintMarker) {
        drawMarker(p);
    }

    p.restore();
}

void CircularViewRenderArea::paintContent(QPainter &p, int w, int h, bool paintSelection, bool paintMarker) {
    qreal scaleCoeff;
    if (w <= h) {
        scaleCoeff = (qreal)w / width();
        p.translate(0, (h - scaleCoeff*height()) / 2);
    } else {
        scaleCoeff = (qreal)h / height();
        p.translate( (w - scaleCoeff*width())/ 2, 0);
    }
    p.save();
    p.scale(scaleCoeff, scaleCoeff);
    paintContent(p, paintSelection, paintMarker);
    p.restore();
}

void CircularViewRenderArea::drawAll(QPaintDevice* pd) {
    QPainter p(pd);
    p.setRenderHint(QPainter::Antialiasing);
    GSLV_UpdateFlags uf = view->getUpdateFlags();
    bool completeRedraw = uf.testFlag(GSLV_UF_NeedCompleteRedraw) || uf.testFlag(GSLV_UF_ViewResized) ||
        uf.testFlag(GSLV_UF_AnnotationsChanged) || settingsWereChanged;

    int viewSize = qMin(circularView->height(), circularView->width());
    verticalOffset = parentWidget()->height()/2;
    if (outerEllipseSize + (regionY.count()-1)*ellipseDelta + VIEW_MARGIN > viewSize) {
        verticalOffset += (outerEllipseSize + (regionY.count()-1)*ellipseDelta + VIEW_MARGIN  - viewSize)/2;
        // distance from the ruler to the end of annotation layers
        int topSpace = ((regionY.count()-1)*ellipseDelta + VIEW_MARGIN) / 2;
        if (innerEllipseSize > pd->width()) { // the ruller cannot fit in view
            // the distance from intersection point of ruler and side border to horizontal diameter
            double x = (innerEllipseSize/2)*(innerEllipseSize/2) - (pd->width()/2)*(pd->width()/2);
            x = sqrt(x);
            int ledge = innerEllipseSize/2 + topSpace - pd->height();
            verticalOffset += (x - ledge)/2;
        }
    }

    if (completeRedraw) {
        QPainter pCached(cachedView);
        pCached.setRenderHint(QPainter::Antialiasing);
        pCached.fillRect(0, 0, pd->width(), pd->height(), Qt::white);
        pCached.translate(parentWidget()->width()/2, verticalOffset);
        pCached.setPen(Qt::black);
        drawRuler(pCached);
        drawAnnotations(pCached);
        pCached.end();
    }
    p.drawPixmap(0, 0, *cachedView);
    p.translate(parentWidget()->width()/2, verticalOffset);

    drawSequenceName(p);

    drawAnnotationsSelection(p);

    drawSequenceSelection(p);

    drawMarker(p);

    if (oldYlevel!=regionY.count()) {
        oldYlevel = regionY.count();
        if (verticalOffset<=parentWidget()->height()/2) {
            circularView->sl_fitInView();
            paintEvent(new QPaintEvent(rect()));
        }
    }
}

void CircularViewRenderArea::drawAnnotationsSelection(QPainter& p) {
    ADVSequenceObjectContext* ctx = view->getSequenceContext();

    if(ctx->getAnnotationsSelection()->getSelection().isEmpty()) {
        return;
    }

    foreach(CircularAnnotationItem* item, circItems.values()) {
        item->setSelected(false);
    }
    foreach(const AnnotationSelectionData& asd, ctx->getAnnotationsSelection()->getSelection()) {
        AnnotationTableObject *o = asd.annotation.getGObject();
        if (ctx->getAnnotationObjects(true).contains(o)) {
            if(circItems.contains(asd.annotation)) {
                CircularAnnotationItem* item = circItems[asd.annotation];
                item->setSelected(true);
                item->paint(&p, NULL, this);
                foreach(const CircularAnnotationRegionItem* r, item->getRegions()) {
                    SAFE_POINT(r != NULL, "NULL annotataion region item is CV!", );
                    CircularAnnotationLabel* lbl = r->getLabel();
                    SAFE_POINT(lbl != NULL, "NULL annotataion label item is CV!", );
                    if(lbl->isVisible()) {
                        lbl->paint(&p, NULL, this);
                    }
                }
            }
        }
    }
}

#define RULER_PAD 40
void CircularViewRenderArea::drawSequenceName(QPainter& p) {
    QPen boldPen(Qt::black);
    boldPen.setWidth(3);
    ADVSequenceObjectContext* ctx = view->getSequenceContext();

    assert( ctx->getSequenceGObject() != NULL );

    QString docName = ctx->getSequenceGObject()->getGObjectName();
    QString seqLen = QString::number(ctx->getSequenceLength()) + " bp";
    int docNameFullLength = docName.length();

    QFont font(settings->titleFont, settings->titleFontSize);
    font.setBold(settings->titleBold);
    p.setFont(font);
    QFontMetrics fm(font, this);
    int cw = fm.width('O');
    int symbolsAlowed = (rulerEllipseSize - RULER_PAD)/cw;
    if(symbolsAlowed<docNameFullLength) {
        docName=docName.mid(0,symbolsAlowed - 2);
        docName+="..";
    }

    p.setPen(boldPen);

    QPointF namePt;
    QPointF lenPt;

    QRectF nameBound = fm.boundingRect(docName+' ');
    QRectF lenBound = fm.boundingRect(seqLen+' ');

    if (verticalOffset - parentWidget()->height() > 0) {
        int delta = verticalOffset - parentWidget()->height();
        namePt = QPointF(0, -delta-nameBound.height()-lenBound.height());
        lenPt = namePt + QPointF(0, lenBound.height());
    } else {
        namePt = QPointF(0,0);
        lenPt = QPointF(0, nameBound.height());
    }

    nameBound.moveCenter(namePt);
    if (settings->showTitle) {
        p.drawText(nameBound.bottomLeft(), docName);
    }

    lenBound.moveCenter(lenPt);
    if (settings->showLength) {
        p.drawText(lenBound.bottomLeft(), seqLen);
    }
}

void CircularViewRenderArea::drawSequenceSelection( QPainter& p ) {
    ADVSequenceObjectContext* ctx = view->getSequenceContext();
    int seqLen = ctx->getSequenceLength();
    const QVector<U2Region>& selection = view->getSequenceContext()->getSequenceSelection()->getSelectedRegions();
    if (selection.isEmpty()) {
        return;
    }

    QList<QPainterPath*> paths;

    foreach(const U2Region& r, selection) {
        QPainterPath* path = new QPainterPath();
        int yLevel = regionY.count() - 1;
        QRect outerRect(-outerEllipseSize/2 - yLevel * ellipseDelta/2 - ARROW_HEIGHT_DELTA,
            -outerEllipseSize/2 - yLevel * ellipseDelta/2 - ARROW_HEIGHT_DELTA,
            outerEllipseSize + yLevel * ellipseDelta + ARROW_HEIGHT_DELTA*2,
            outerEllipseSize + yLevel * ellipseDelta + ARROW_HEIGHT_DELTA*2);
        QRectF innerRect(-rulerEllipseSize/2 + NOTCH_SIZE, -rulerEllipseSize/2 + NOTCH_SIZE,
            rulerEllipseSize - 2*NOTCH_SIZE, rulerEllipseSize-2*NOTCH_SIZE);
        float startAngle = r.startPos / (float)seqLen * 360 + rotationDegree;
        float spanAngle = r.length / (float)seqLen * 360;
        path->moveTo(outerRect.width()/2 * cos(-startAngle / 180.0 * PI),
            -outerRect.width()/2 * sin(-startAngle / 180.0 * PI));
        path->arcTo(outerRect, -startAngle, -spanAngle);
        path->arcTo(innerRect, -startAngle-spanAngle, spanAngle);
        path->closeSubpath();
        paths.append(path);
    }
    p.save();
    QPen selectionPen(QColor("#007DE3"));
    selectionPen.setStyle(Qt::DashLine);
    selectionPen.setWidth(1);
    p.setPen(selectionPen);
    foreach(QPainterPath* path, paths) {
        p.drawPath(*path);
    }
    p.restore();
}

void normalizeAngle(qreal& a) {
    while(a>360) {
        a-=360;
    }
    while(a<0) {
        a+=360;
    }
}

void normalizeAngleRad(qreal& a) {
    while(a>2*PI) {
        a-=2*PI;
    }
    while(a<0) {
        a+=2*PI;
    }
}

void CircularViewRenderArea::drawRulerNotches(QPainter& p, int start, int span, int seqLen) {
    int notchSize = 5;
    QFont f;
    QFontMetrics fm(f,this);
    int cw = fm.width('0');
    int N = QString::number(start+span).length()*cw*3/2.0 + 0.5f;
    int rulerLen = span / (float)seqLen * PI * rulerEllipseSize;
    int chunk = GraphUtils::findChunk(rulerLen, span, N);
    CHECK(chunk != 0, );
    start-=start%chunk;
    float halfChar = 180 / (float)seqLen;
    for (int currentNotch=start+chunk; currentNotch < start + span + chunk; currentNotch+=chunk) {
        if (currentNotch > seqLen) {
            currentNotch = seqLen;
        }
        qreal d = currentNotch / (float)seqLen * 360 + rotationDegree - halfChar;
        d*=PI/180.0;
        d = 2*PI - d;
        QPoint point1 ( rulerEllipseSize * cos(d) / 2.0 + 0.5f, - rulerEllipseSize * sin(d) / 2.0 - 0.5f);
        point1-=QPoint(0,0);
        QPoint point2 = point1 - QPoint(notchSize*cos(d), -notchSize*sin(d));
        QPoint point3 = point2 - QPoint(3*cos(d), 0);
        if (currentNotch == 0) {

        }
        QString label = FormatUtils::formatNumber(currentNotch);
        QRect bounding = p.boundingRect(0,0,1000,1000, Qt::AlignLeft, label);

        normalizeAngleRad(d);
        if(d>PI/4 && d<=PI/2 + PI/4) {
            QPoint dP((float)bounding.width() / 2 * (1-cos(d)), 0);
            bounding.moveTopRight(point3 + dP);
        } else if(d>PI/2 + PI/4 && d<=PI + PI/4) {
            QPoint dP(0, (float)bounding.height() / 2 * (1-sin(d)));
            bounding.moveTopLeft(point3 - dP);
        } else if(d>PI + PI/4 && d<=3*PI/2 + PI/4) {
            QPoint dP((float)bounding.width() / 2 * (1-cos(d)), 0);
            bounding.moveBottomRight(point3 + dP);
        } else {
            QPoint dP(0, (float)bounding.height() / 2 * (1-sin(d)));
            bounding.moveTopRight(point3 - dP);
        }

        p.drawLine(point1,point2);
        p.drawText(bounding, label);
    }
}

qreal CircularViewRenderArea::getVisibleAngle() const {
    int w = parentWidget()->width();
    int h = parentWidget()->height();

    int y = verticalOffset - h;
    float x = rulerEllipseSize/2.0;
    assert(y>0);
    assert(x>y);
    int rulerChord = 2*sqrt(double(x*x - y*y));
    rulerChord = qMin(rulerChord, w);
    assert(qAbs(rulerChord)<qAbs(rulerEllipseSize));
    qreal spanAngle = qAbs(asin(rulerChord/(double)rulerEllipseSize));
    return spanAngle;
}

QPair<int,int> CircularViewRenderArea::getVisibleRange() const {
    ADVSequenceObjectContext* ctx = view->getSequenceContext();
    int seqLen = ctx->getSequenceObject()->getSequenceLength();
    if (verticalOffset <= parentWidget()->height()) {
        return qMakePair<int,int>(0, seqLen);
    }
    qreal spanAngle = getVisibleAngle();
    qreal startAngle = 3*PI/2.0 - spanAngle;
    qreal temp = startAngle - rotationDegree*PI/180.0;
    normalizeAngleRad(temp);
    int start = seqLen*temp/(2.0*PI) + 0.5f;
    int span = seqLen*spanAngle/(double(PI)) + 0.5f;
    return qMakePair<int,int>(start, span);
}

void CircularViewRenderArea::drawRuler( QPainter& p ) {
    p.save();
    normalizeAngle(rotationDegree);

    if (settings->showRulerCoordinates) {
        ADVSequenceObjectContext* ctx = view->getSequenceContext();
        U2Region range(0, ctx->getSequenceLength());
        QFont font = p.font();
        font.setPointSize(settings->rulerFontSize);
        p.setFont(font);
        drawRulerCoordinates(p, range.startPos, range.length);
    }
    if (settings->showRulerLine) {
        QPen boldPen(Qt::black);
        boldPen.setWidth(3);
        p.setPen(boldPen);
        QRectF rulerRect(-rulerEllipseSize/2, -rulerEllipseSize/2, rulerEllipseSize, rulerEllipseSize);
        rulerRect.moveCenter(QPointF(0,0));
        p.drawEllipse(rulerRect);
    }
    p.restore();
}

void CircularViewRenderArea::drawRulerCoordinates(QPainter &p, int startPos, int seqLen) {
    if (currentScale == 0) {
        drawRulerNotches(p, startPos, seqLen, seqLen);
    } else {
        const QPair<int,int>& loc = getVisibleRange();
        int start = loc.first;
        int span = loc.second;
        if (start==seqLen) {
            drawRulerNotches(p, 0, span, seqLen);
        } else if (start + span > seqLen) {
            int span1 = seqLen - start;
            int span2 = start + span - seqLen;
            assert(span1);
            assert(span2);
            drawRulerNotches(p, start, span1, seqLen);
            drawRulerNotches(p, 0, span2, seqLen);
        } else {
            drawRulerNotches(p, start, span, seqLen);
        }
    }
}

void CircularViewRenderArea::drawAnnotations( QPainter &p ) {
    ADVSequenceObjectContext *ctx = view->getSequenceContext( );

    foreach ( CircularAnnotationItem *item, circItems ) {
        delete item;
    }
    circItems.clear( );
    labelList.clear( );
    annotationYLevel.clear( );
    regionY.clear( );
    circItems.clear( );

    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry( );
    //TODO: there need const order of annotation tables
    QSet<AnnotationTableObject *> anns = ctx->getAnnotationObjects( true );
    QSet<AnnotationTableObject *> autoAnns = ctx->getAutoAnnotationObjects();
    foreach ( AnnotationTableObject *ao, anns ) {
        bool isAutoAnnotation = autoAnns.contains(ao);
        foreach ( const Annotation &a, ao->getAnnotations( ) ) {
            AnnotationSettings *as = asr->getAnnotationSettings(a.getData());
            buildAnnotationItem(DrawAnnotationPass_DrawFill, a, false, as);
            QFont font = p.font();
            font.setPointSize(settings->labelFontSize);
            buildAnnotationLabel(font, a, as, isAutoAnnotation);
        }
    }

    CircularAnnotationLabel::prepareLabels(labelList);
    evaluateLabelPositions();

    foreach(CircularAnnotationItem* item, circItems) {
        item->paint(&p, NULL, this);
    }

    if (settings->labelMode == CircularViewSettings::None) {
        return;
    }
    foreach(CircularAnnotationLabel* label, labelList) {
        const Annotation ann = label->getAnnotation();
        label->setLabelPosition();
        label->paint(&p, NULL, this);
    }
}

void CircularViewRenderArea::redraw() {
    settingsWereChanged = true;
    repaint();
}

bool isGreater(U2Region r1, U2Region r2) {
    return r1.startPos > r2.startPos;
};

#define REGION_MIN_LEN 3
void CircularViewRenderArea::buildAnnotationItem(DrawAnnotationPass pass, const Annotation &a,
    bool selected /* = false */, const AnnotationSettings* as /* = NULL */)
{
    const AnnotationData aData = a.getData( );
    if (!as->visible && (pass == DrawAnnotationPass_DrawFill || !selected)) {
        return;
    }

    ADVSequenceObjectContext* ctx = view->getSequenceContext();
    SAFE_POINT(ctx != NULL, "Sequence object context is NULL", );
    int seqLen = ctx->getSequenceLength();

    QVector<U2Region> aDataLocation = aData.getRegions();
    QVector<U2Region> location = aData.getRegions();
    removeRegionsOutOfRange(location, seqLen);

    qStableSort(location.begin(), location.end(), isGreater);
    int yLevel = findOrbit(location, a);

    QPair<U2Region, U2Region> mergedRegions = mergeCircularJunctoinRegion(location, seqLen);

    QList<CircularAnnotationRegionItem*> regions;
    foreach(const U2Region& r, location) {
        int idx = aDataLocation.indexOf(r);
        bool addJoinedRegion = false;
        if (location.size() != aDataLocation.size() && !aDataLocation.contains(r)
                && (!mergedRegions.first.isEmpty() && !mergedRegions.second.isEmpty())) {
            idx = aDataLocation.indexOf(mergedRegions.first);
            addJoinedRegion = true;
        }
        CircularAnnotationRegionItem* regItem = createAnnotationRegionItem(r, seqLen, yLevel, aData, idx);
        if (regItem != NULL) {
            regions.append( regItem );
            if (addJoinedRegion) {
                regItem->setJoinedRegion(mergedRegions.second);
            }
        }
    }

    CircularAnnotationItem* item = new CircularAnnotationItem(a, regions, this);
    circItems[a] = item;
}

int CircularViewRenderArea::findOrbit(const QVector<U2Region> &location,
                                      const Annotation &a) {
    int yLevel = 0;
    bool yFind = false;
    for(; yLevel < regionY.count(); yLevel++) {
        bool intersects = false;
        foreach(const U2Region& r, regionY[yLevel]) {
            foreach (const U2Region& locRegion, location) {
                if (r.intersects(locRegion)) {
                    intersects = true;
                }
            }
        }
        if(!intersects) {
            QVector<U2Region>& rY = regionY[yLevel];
            rY << location;
            yFind = true;
            break;
        }
    }
    if(!yFind) {
        QVector<U2Region> newLevel;
        newLevel << location;
        regionY.append(newLevel);
    }
    annotationYLevel[a] = yLevel;

    return yLevel;
}

CircularAnnotationRegionItem* CircularViewRenderArea::createAnnotationRegionItem(const U2Region &r, int seqLen,
                                                                                 int yLevel,
                                                                                 const AnnotationData &aData,
                                                                                 int index) {
    int totalLen = r.length;
    float startAngle = (float)r.startPos / (float)seqLen * 360;
    float spanAngle = (float)totalLen / (float)seqLen * 360;

    //cut annotation border if dna is linear
    if(!circularView->isCircularTopology()) {
        spanAngle = qMin(spanAngle, (float)(360-startAngle));
    }

    startAngle+=rotationDegree;

    QRect outerRect(-outerEllipseSize/2 - yLevel * ellipseDelta/2, -outerEllipseSize/2 - yLevel * ellipseDelta/2, outerEllipseSize + yLevel * ellipseDelta, outerEllipseSize + yLevel * ellipseDelta);
    QRect innerRect(-innerEllipseSize/2 - yLevel * ellipseDelta/2, -innerEllipseSize/2 - yLevel * ellipseDelta/2, innerEllipseSize + yLevel * ellipseDelta, innerEllipseSize + yLevel * ellipseDelta);
    QRect middleRect(-middleEllipseSize/2 - yLevel * ellipseDelta/2, -middleEllipseSize/2 - yLevel * ellipseDelta/2, middleEllipseSize + yLevel * ellipseDelta, middleEllipseSize + yLevel * ellipseDelta);
    arrowLength = qMin(arrowLength, ARROW_LENGTH);
    float dAlpha = 360 * arrowLength / (float)PI / (outerEllipseSize + innerEllipseSize + yLevel*ellipseDelta);
    bool isShort = totalLen / (float)seqLen * 360 < dAlpha;

    float regionLen = spanAngle*PI/180 * outerRect.height()/2;
    if(regionLen < REGION_MIN_LEN) {
        spanAngle = (float)REGION_MIN_LEN / (PI*outerRect.height()) * 360;
    }

    QPainterPath path = createAnnotationArrowPath(startAngle, spanAngle, dAlpha,
                                     outerRect, innerRect, middleRect,
                                     aData.getStrand().isCompementary(), isShort);
    CHECK(path.length() != 0, NULL);
    qreal centerPercent = 0;
    if (!isShort) {
        qreal center = PI * (middleRect.width() / 2) * (spanAngle - dAlpha) / 360;
        centerPercent = center / path.length();
    }

    CircularAnnotationRegionItem* regItem = new CircularAnnotationRegionItem(path, isShort, index);
    CHECK(regItem != NULL, NULL);
    regItem->setArrowCenterPercentage(centerPercent);

    return regItem;
}

QPainterPath CircularViewRenderArea::createAnnotationArrowPath(float startAngle, float spanAngle, float dAlpha,
                                                               const QRect &outerRect, const QRect &innerRect, const QRect &middleRect,
                                                               bool complementary, bool isShort) const {
    bool moreThan360 = spanAngle > 360;
    QPainterPath path;
    if(isShort) {
        path.moveTo(outerRect.width()/2 * cos(-startAngle / 180.0 * PI),-outerRect.height()/2 * sin(-startAngle / 180.0 * PI));
        path.arcTo(outerRect, -startAngle, -spanAngle);
        path.arcTo(innerRect, -startAngle-spanAngle, spanAngle);
        path.closeSubpath();
    } else {
        if(complementary) {
            path.moveTo(outerRect.width()/2 * cos((startAngle + dAlpha) / 180.0 * PI),
                outerRect.height()/2 * sin((startAngle + dAlpha) / 180.0 * PI));
            path.lineTo((outerRect.width()/2 + arrowHeightDelta) * cos((startAngle + dAlpha) / 180.0 * PI),
                (outerRect.width()/2 + arrowHeightDelta) * sin((startAngle + dAlpha) / 180.0 * PI));
            path.lineTo(middleRect.width()/2 * cos(startAngle / 180.0 * PI),
                middleRect.height()/2 * sin(startAngle/ 180.0 * PI));
            path.lineTo((innerRect.width()/2 - arrowHeightDelta) * cos((startAngle + dAlpha) / 180.0 * PI),
                (innerRect.width()/2 - arrowHeightDelta) * sin((startAngle + dAlpha) / 180.0 * PI));

            if (moreThan360) {
                path.setFillRule(Qt::WindingFill);
                path.arcTo(innerRect, -(startAngle + dAlpha), -(spanAngle - 360 - dAlpha));
                path.arcTo(innerRect, -startAngle - spanAngle + 360, -(360 - dAlpha));

                path.arcTo(outerRect, -startAngle - spanAngle + dAlpha, 360 - dAlpha);
                path.arcTo(outerRect, -startAngle - spanAngle + 360, spanAngle - 360 - dAlpha);
            } else {
                path.arcTo(innerRect, -(startAngle + dAlpha), -(spanAngle - dAlpha));
                path.arcTo(outerRect, -startAngle - spanAngle, spanAngle - dAlpha);
            }
            path.closeSubpath();
        } else {
            path.moveTo(outerRect.width()/2 * cos(startAngle / 180.0 * PI),
                outerRect.height()/2 * sin(startAngle / 180.0 * PI));

            if (moreThan360) {
                path.setFillRule(Qt::WindingFill);
                path.arcTo(outerRect, -startAngle, -(spanAngle - 360));
                path.arcTo(outerRect, -startAngle - (spanAngle - 360), -(360 - dAlpha));
            } else {
                path.arcTo(outerRect, -startAngle, -(spanAngle - dAlpha));
            }

            path.lineTo((outerRect.width()/2 + arrowHeightDelta) * cos((startAngle + spanAngle - dAlpha) / 180.0 * PI),
                    (outerRect.height()/2 + arrowHeightDelta) * sin((startAngle + spanAngle - dAlpha) / 180.0 * PI));
            path.lineTo(middleRect.width()/2 * cos((startAngle + spanAngle) / 180.0 * PI),
                    middleRect.height()/2 * sin((startAngle + spanAngle)/ 180.0 * PI));
            path.lineTo((innerRect.width()/2 - arrowHeightDelta) * cos((-startAngle - (spanAngle - dAlpha)) / 180.0 * PI),
                    (innerRect.height()/2 - arrowHeightDelta) * sin((startAngle + spanAngle - dAlpha) / 180.0 * PI));

            if (moreThan360) {
                path.arcTo(innerRect, -startAngle - spanAngle + dAlpha, 360 - dAlpha);
                path.arcTo(innerRect, -startAngle - spanAngle + 360, spanAngle - 360);

            } else {
                path.arcTo(innerRect, - startAngle - (spanAngle - dAlpha), spanAngle - dAlpha);
            }

            path.closeSubpath();
        }
    }
    return path;
}


void CircularViewRenderArea::buildAnnotationLabel( const QFont &font, const Annotation &a,
                                                   const AnnotationSettings *as, bool isAutoAnnotation) {
    const AnnotationData aData = a.getData( );
    if ( !as->visible ) {
        return;
    }

    if( !circItems.contains( a ) ) {
        return;
    }

    ADVSequenceObjectContext *ctx = view->getSequenceContext( );
    U2Region seqReg( 0, ctx->getSequenceLength( ) );

    int seqLen = seqReg.length;
    QVector<U2Region> location = aData.getRegions( );
    removeRegionsOutOfRange(location, seqLen);

    qStableSort(location.begin(), location.end(), isGreater);
    QPair<U2Region, U2Region> mergedRegions = mergeCircularJunctoinRegion(location, seqLen);

    for( int r = 0; r < location.count( ); r++ ) {
        CircularAnnotationLabel *label = new CircularAnnotationLabel( a, location,
                                                                      isAutoAnnotation, r,
                                                                      seqLen, font, this );
        labelList.append( label );
        CircularAnnotationRegionItem *ri = circItems[a]->getRegions( )[r];
        label->setAnnRegion(ri);
        ri->setLabel( label );
    }
}

U2Region CircularViewRenderArea::getAnnotationYRange( const Annotation &, int,
    const AnnotationSettings * ) const
{
    return U2Region(0,0);
}

void CircularViewRenderArea::resizeEvent( QResizeEvent *e ) {
    view->addUpdateFlags(GSLV_UF_ViewResized);
    QWidget::resizeEvent(e);
}

void CircularViewRenderArea::drawMarker(QPainter& p) {
    int yLevel = regionY.count() - 1;
    QPen markerPen;
    markerPen.setWidth(1);
    markerPen.setColor(Qt::gray);
    p.setPen(markerPen);

    QPainterPath arr1, arr2;

    arr1.moveTo((rulerEllipseSize/2.0 - MARKER_LEN)*cos(mouseAngle),
        (rulerEllipseSize/2.0 - MARKER_LEN)*sin(mouseAngle));
    QPointF point11((rulerEllipseSize/2.0 - NOTCH_SIZE)*cos(mouseAngle),
        (rulerEllipseSize/2.0 - NOTCH_SIZE)*sin(mouseAngle));
    arr1.lineTo(point11);
    arr1.lineTo(point11 - QPointF(ARR_LEN*sin(mouseAngle) + ARR_WIDTH/2*cos(mouseAngle),
        -ARR_LEN*cos(mouseAngle) + ARR_WIDTH/2*sin(mouseAngle)));
    arr1.moveTo(point11);
    arr1.lineTo(point11 + QPointF(ARR_LEN*sin(mouseAngle) - ARR_WIDTH/2*cos(mouseAngle),
        -ARR_LEN*cos(mouseAngle) - ARR_WIDTH/2*sin(mouseAngle)));

    arr2.moveTo((outerEllipseSize/2 + yLevel * ellipseDelta/2 + MARKER_LEN)*cos(mouseAngle),
        (outerEllipseSize/2 + yLevel * ellipseDelta/2 + MARKER_LEN)*sin(mouseAngle));
    QPointF point21((outerEllipseSize/2 + yLevel * ellipseDelta/2 + ARROW_HEIGHT_DELTA)*cos(mouseAngle),
        (outerEllipseSize/2 + yLevel * ellipseDelta/2 + ARROW_HEIGHT_DELTA)*sin(mouseAngle));
    arr2.lineTo(point21);
    arr2.lineTo(point21 + QPointF(ARR_LEN*sin(mouseAngle) + ARR_WIDTH/2*cos(mouseAngle),
        -ARR_LEN*cos(mouseAngle) + ARR_WIDTH/2*sin(mouseAngle)));
    arr2.moveTo(point21);
    arr2.lineTo(point21 + QPointF(-ARR_LEN*sin(mouseAngle) + ARR_WIDTH/2*cos(mouseAngle),
        ARR_LEN*cos(mouseAngle) + ARR_WIDTH/2*sin(mouseAngle)));

    p.drawPath(arr1);
    p.drawPath(arr2);
}

#define LABEL_PAD 30
void CircularViewRenderArea::evaluateLabelPositions() {
    labelEmptyPositions.clear();

    QFont f;
    QFontMetrics fm(f,this);
    int labelHeight = fm.height();
    int lvlsNum = regionY.count();
    int outerRadius = outerEllipseSize/2 + (lvlsNum-1)*ellipseDelta/2;

    int areaHeight = height();

    int z0 = -areaHeight/2 + labelHeight;
    int z1 = areaHeight/2 - labelHeight;
    if (currentScale != 0) {
        int wH = parentWidget()->height();
        if (verticalOffset>wH) {
            z0 = -outerRadius;
            z1 = -outerRadius*cos(getVisibleAngle());
        }
    }
    for(int zPos=z0; zPos<z1; zPos+=labelHeight) {
        int x = sqrt(float(outerRadius*outerRadius - zPos*zPos));
        if(width()/2-x>0) {
            QRect l_rect(-x - LABEL_PAD, zPos, width()/2-(x+LABEL_PAD), labelHeight);
            QRect r_rect(x + LABEL_PAD, zPos, width()/2-(x+LABEL_PAD), labelHeight);
            labelEmptyPositions << l_rect << r_rect;
        }
    }
}

CircularViewRenderArea::~CircularViewRenderArea() {
    qDeleteAll(circItems.values());
}

void CircularViewRenderArea::removeRegionsOutOfRange(QVector<U2Region> &location, int seqLen) const {
    for (int i = 0; i < location.size(); i++) {
        if (location[i].endPos() > seqLen) {
            location.remove(i);
            i--;
        }
    }
}

QPair<U2Region, U2Region> CircularViewRenderArea::mergeCircularJunctoinRegion(QVector<U2Region> &location, int seqLen) const {
    QPair<U2Region, U2Region> res;
    if (location.size() >= 2) {
        if (location.first().endPos() == seqLen && location.last().startPos == 0 && circularView->isCircularTopology()) {
            res.first = location.first();
            res.second = location.last();
            location.first().length += location.last().length;
            location.remove(location.size() - 1);
        }
    }
    return res;
}

}//namespace
