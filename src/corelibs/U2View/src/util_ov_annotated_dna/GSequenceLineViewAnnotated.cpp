/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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

#include "GSequenceLineViewAnnotated.h"
#include "ADVSequenceObjectContext.h"

#include <U2Core/AppContext.h>
#include <U2Core/Timer.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Core/DNASequenceSelection.h>
#include <U2Gui/GUIUtils.h>
#include <U2Core/AnnotationData.h>
#include <U2Formats/GenbankFeatures.h>

#include <QtGui/QMenu>
#include <QtGui/QToolTip>
#include <QtGui/QPainterPath>
#include <QtGui/QApplication>

namespace U2 {

#define MIN_ANNOTATION_WIDTH 3
#define MIN_ANNOTATION_TEXT_WIDTH 5
#define MIN_SELECTED_ANNOTATION_WIDTH 4
#define MIN_WIDTH_TO_DRAW_EXTRA_FEATURES 10

GSequenceLineViewAnnotated::GSequenceLineViewAnnotated(QWidget* p, ADVSequenceObjectContext* ctx) 
: GSequenceLineView(p, ctx)
{
    const QSet<AnnotationTableObject*> aObjs = ctx->getAnnotationObjects(true);
    foreach(AnnotationTableObject* ao, aObjs) {
        connectAnnotationObject(ao);
    }
    connect(ctx->getAnnotationsSelection(),
        SIGNAL(si_selectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>& )),
        SLOT(sl_onAnnotationSelectionChanged(AnnotationSelection*, const QList<Annotation*>&, const QList<Annotation*>&)));

    connect(ctx, SIGNAL(si_annotationObjectAdded(AnnotationTableObject*)), SLOT(sl_onAnnotationObjectAdded(AnnotationTableObject*)));
    connect(ctx, SIGNAL(si_annotationObjectRemoved(AnnotationTableObject*)), SLOT(sl_onAnnotationObjectRemoved(AnnotationTableObject*)));
    
    connect(AppContext::getAnnotationsSettingsRegistry(),
        SIGNAL(si_annotationSettingsChanged(const QStringList&)),
        SLOT(sl_onAnnotationSettingsChanged(const QStringList&)));

}

void GSequenceLineViewAnnotated::connectAnnotationObject(AnnotationTableObject* ao) {
    connect(ao, SIGNAL(si_onAnnotationsAdded(const QList<Annotation*>&)), SLOT(sl_onAnnotationsAdded(const QList<Annotation*>&)));
    connect(ao, SIGNAL(si_onAnnotationsRemoved(const QList<Annotation*>&)), SLOT(sl_onAnnotationsRemoved(const QList<Annotation*>&)));
    connect(ao, SIGNAL(si_onAnnotationModified(const AnnotationModification&)), SLOT(sl_onAnnotationsModified(const AnnotationModification&)));
}

void GSequenceLineViewAnnotated::sl_onAnnotationSettingsChanged(const QStringList&) {
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationObjectAdded(AnnotationTableObject* o) {
    connectAnnotationObject(o);
    sl_onAnnotationsAdded(o->getAnnotations());
}

void GSequenceLineViewAnnotated::sl_onAnnotationObjectRemoved(AnnotationTableObject* o) {
    o->disconnect(this);
    sl_onAnnotationsRemoved(o->getAnnotations());
}


void GSequenceLineViewAnnotated::sl_onAnnotationsAdded(const QList<Annotation*>& l) {
    GTIMER(c2,t2,"GSequenceLineViewAnnotated::sl_onAnnotationsAdded");
    registerAnnotations(l);
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationsRemoved(const QList<Annotation*>& l) {
    unregisterAnnotations(l);
    addUpdateFlags(GSLV_UF_AnnotationsChanged);
    update();
}

void GSequenceLineViewAnnotated::sl_onAnnotationSelectionChanged(AnnotationSelection* as, const QList<Annotation*>& _added, const QList<Annotation*>& _removed) {
    const QSet<AnnotationTableObject*> aos = ctx->getAnnotationObjects(true);

    bool changed = false;
    QList<Annotation*> added = ctx->selectRelatedAnnotations(_added);
    QList<Annotation*> removed = ctx->selectRelatedAnnotations(_removed);

    if (added.size() == 1) {
        Annotation* a = added.first();
        if (aos.contains(a->getGObject())) {
            const AnnotationSelectionData* asd = as->getAnnotationData(a);
            ensureVisible(a, asd->locationIdx);
            changed = true;
        }
    }

    if (!changed) {
        foreach(Annotation* a, added) {
            if (aos.contains(a->getGObject()) && isAnnotationVisible(a)) {
                changed = true;
                break;
            }
        }
        if (!changed) {
            foreach(Annotation* a, removed) {
                if (aos.contains(a->getGObject()) && isAnnotationVisible(a)) {
                    changed = true;
                    break;
                }
            }
        }
    }

    if (changed) {
        addUpdateFlags(GSLV_UF_SelectionChanged);
        update();
    }
}

bool GSequenceLineViewAnnotated::isAnnotationVisible(Annotation* a) const  {
    foreach(const U2Region& r, a->getRegions()) {
        if (visibleRange.intersects(r)) {
            return true;
        }
    }
    return false;
}

QList<AnnotationSelectionData> GSequenceLineViewAnnotated::selectAnnotationByCoord(const QPoint& p) const {
    QList<AnnotationSelectionData> res;
    GSequenceLineViewAnnotatedRenderArea* ra = (GSequenceLineViewAnnotatedRenderArea*)renderArea;
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    int pos = ra->coordToPos(p.x());
    int dPos = 0;
    if (visibleRange.length > renderArea->width()) {
        float scale = renderArea->getCurrentScale();
        dPos = (int) ( 1 / (scale) );
        assert(dPos < seqLen);
    }
    U2Region reg(pos-dPos, 1 + 2*dPos);
    const QSet<AnnotationTableObject*> aObjs = ctx->getAnnotationObjects(true);
    foreach(AnnotationTableObject* ao, aObjs ) {
        foreach(Annotation* a, ao->getAnnotations()) {
            QVector<U2Region> location = a->getRegions();
            for (int i = 0, n = location.size(); i < n; i++) {
                const U2Region& l = location[i];
                if (l.intersects(reg) || l.endPos() == reg.startPos) {
                    bool ok = true;
                    if (l.endPos() == pos || pos == l.startPos) {//now check pixel precise coords for boundaries
                        int x1 = ra->posToCoord(l.startPos, true);
                        int x2 = ra->posToCoord(l.endPos(), true);
                        ok = p.x() <= x2 && p.x() >= x1;
                    }
                    if (ok) {
                        AnnotationSettings* as = asr->getAnnotationSettings(a);
                        if (as->visible) {
                            U2Region ry = ra->getAnnotationYRange(a, i, as);
                            if (ry.contains(p.y())) {
                                res.append(AnnotationSelectionData(a, i));
                            }
                        }
                    }
                }
            }
        }
    }
    return res;
}

void GSequenceLineViewAnnotated::mousePressEvent(QMouseEvent * me) {
    setFocus();
    QPoint p = toRenderAreaPoint(me->pos());
    Qt::KeyboardModifiers km = QApplication::keyboardModifiers();
    bool singleBaseSelectionMode = km.testFlag(Qt::AltModifier);
    bool annotationEvent = false; // true if mouse pressed in some annotation area
    if (renderArea->rect().contains(p) && me->button()== Qt::LeftButton && !singleBaseSelectionMode) {
        bool expandAnnotationSelectionToSequence = me->modifiers().testFlag(Qt::ControlModifier);
        if (!me->modifiers().testFlag(Qt::ShiftModifier)) {
            ctx->getAnnotationsSelection()->clear();
        }
        QList<AnnotationSelectionData> selected = selectAnnotationByCoord(p);
        annotationEvent = !selected.isEmpty();
        if (annotationEvent) {
            AnnotationSelectionData* asd = &selected.first();
            if (selected.size() > 1) {
                AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
                QMenu popup;
                foreach(const AnnotationSelectionData& as, selected) {
                    QVector<U2Region> location = as.annotation->getRegions();
                    const U2Region& r = location[qMax(0, as.locationIdx)];
                    QString text = as.annotation->getAnnotationName()
                                   + QString(" [%1, %2]").arg(QString::number(r.startPos+1)).arg(QString::number(r.endPos()));
                    AnnotationSettings* asettings = asr->getAnnotationSettings(as.annotation->getAnnotationName());
                    QIcon icon = GUIUtils::createSquareIcon(asettings->color, 10);
                    popup.addAction(icon, text);
                }
                QAction* a = popup.exec(QCursor::pos());
                if (a == NULL) {
                    asd = NULL;
                } else {
                    int idx = popup.actions().indexOf(a);
                    asd = &selected[idx];
                }
            }
            if (asd != NULL) { //add to annotation selection
                AnnotationSelection* asel = ctx->getAnnotationsSelection();
                if (asel->contains(asd->annotation, asd->locationIdx)) {
                    ctx->getAnnotationsSelection()->removeFromSelection(asd->annotation, asd->locationIdx);
                } else {
                    ctx->getAnnotationsSelection()->addToSelection(asd->annotation, asd->locationIdx);
                }
                //select region
                if (expandAnnotationSelectionToSequence) {
                    ctx->getSequenceSelection()->clear();
                    U2Region regionToSelect;
                    foreach(const AnnotationSelectionData& asd, asel->getSelection()) {
                        AnnotationTableObject* aobj = asd.annotation->getGObject();
                        const QSet<AnnotationTableObject*> aObjs = ctx->getAnnotationObjects(true);
                        if (!aObjs.contains(aobj)) {
                            continue;
                        }
                        U2Region aregion;
                        if (asd.locationIdx == -1) {
                            aregion = U2Region::containingRegion(asd.annotation->getRegions());
                        } else {
                            aregion = asd.annotation->getRegions().at(asd.locationIdx);
                        }
                        regionToSelect = regionToSelect.length == 0 ? aregion : U2Region::containingRegion(regionToSelect, aregion);
                    }
                    ctx->getSequenceSelection()->addRegion(regionToSelect);
                }
            }
        }
    }
    
    // a hint to parent class: if mouse action leads to annotation selection -> skip selection handling for mouse press
    ignoreMouseSelectionEvents = annotationEvent;
    GSequenceLineView::mousePressEvent(me);
    ignoreMouseSelectionEvents = false;
}

QString GSequenceLineViewAnnotated::prepareAnnotationText(const Annotation* a, const AnnotationSettings* as) {
    if (as->nameQuals.isEmpty()) {
        return a->getAnnotationName();
    }
    QVector<U2Qualifier> qs;
    foreach(const QString& qn, as->nameQuals) {
        qs.clear();
        a->findQualifiers(qn, qs);
        if (!qs.isEmpty()) {
            QString res = qs[0].value;
            return res;
        }
    }
    return a->getAnnotationName();
}

QList<Annotation*> GSequenceLineViewAnnotated::findAnnotationsInRange(const U2Region& range) {
    QList<Annotation*> result;
    const QSet<AnnotationTableObject*> aObjs = ctx->getAnnotationObjects(true);
    foreach(AnnotationTableObject* ao, aObjs) {
        foreach (Annotation* a, ao->getAnnotations()) {
            if (range.intersects(a->getRegions())) {
                result.append(a);
            }
        }
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////
/// Renderer

GSequenceLineViewAnnotatedRenderArea::GSequenceLineViewAnnotatedRenderArea(GSequenceLineViewAnnotated* d, bool overlap) : GSequenceLineViewRenderArea(d)
{
    annotationsCanOverlap = overlap;
    afNormal = new QFont("Courier", 10);
    afSmall = new QFont("Arial", 8);

    afmNormal = new QFontMetrics(*afNormal);
    afmSmall = new QFontMetrics(*afSmall);

    afNormalCharWidth  = afmNormal->width('w');
    afSmallCharWidth   = afmSmall->width('w');

    QLinearGradient gradient(0, 0, 0, 1); //vertical
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
    gradient.setColorAt(0.00, QColor(255, 255, 255, 120));
    gradient.setColorAt(0.50, QColor(  0,   0,   0,   0));
    gradient.setColorAt(0.70, QColor(  0,   0,   0,   0));
    gradient.setColorAt(1.00, QColor(  0,   0,   0,  70));
    gradientMaskBrush = QBrush(gradient);
}

GSequenceLineViewAnnotatedRenderArea::~GSequenceLineViewAnnotatedRenderArea() {
    delete afmNormal;
    delete afNormal;
    delete afmSmall;
    delete afSmall;
}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotations(QPainter& p) {
    GTIMER(c2,t2,"GSequenceLineViewAnnotatedRenderArea::drawAnnotations");
    ADVSequenceObjectContext* ctx = view->getSequenceContext();

    QPen pen1(Qt::SolidLine);
    pen1.setWidth(1);

    foreach(AnnotationTableObject* ao, ctx->getAnnotationObjects(true)) {
        foreach(Annotation* a, ao->getAnnotations()) {
            AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
            AnnotationSettings* as = asr->getAnnotationSettings(a->getAnnotationName());
            drawAnnotation(p, DrawAnnotationPass_DrawFill, a, pen1, false, as);
            drawAnnotation(p, DrawAnnotationPass_DrawBorder, a, pen1, false, as);
        }
    }
}

#define FEATURE_ARROW_HLEN 3
#define FEATURE_ARROW_VLEN 3
void static addArrowPath(QPainterPath& path, const QRect& rect, bool leftArrow) {
    if (rect.width() <= FEATURE_ARROW_HLEN || rect.height() <= 0) {
        return;
    }
    int x = leftArrow ? rect.left() : rect.right();
    int dx = leftArrow ? -FEATURE_ARROW_HLEN : FEATURE_ARROW_HLEN;
    
    QPolygon arr;
    arr << QPoint(x - dx, rect.top()    - FEATURE_ARROW_VLEN);
    arr << QPoint(x + dx, rect.top()    + rect.height() / 2);
    arr << QPoint(x - dx, rect.bottom() + FEATURE_ARROW_VLEN);
    arr << QPoint(x - dx, rect.top()    - FEATURE_ARROW_VLEN);
    QPainterPath arrowPath;
    arrowPath.addPolygon(arr);

    QPainterPath dRectPath;
    dRectPath.addRect(leftArrow ? x : x - (FEATURE_ARROW_HLEN - 1), rect.top(), FEATURE_ARROW_HLEN, rect.height());

    path = path.subtracted(dRectPath);
    path = path.united(arrowPath);
}

/*#define FEATURE_TRIM_HLEN 5
void static addTrimPath(QPainterPath& path, const QRect& rect, bool leftTrim) {
    //TODO: normalize line line by 4
    if (rect.width() <= FEATURE_TRIM_HLEN || rect.height() <= 0) {
        return;
    }
    int x = leftTrim ? rect.left() - 1: rect.right() + 1;
    int dx = leftTrim ? -FEATURE_TRIM_HLEN : FEATURE_TRIM_HLEN;
    float dy = rect.height() / 4.0f;

    QPolygon arr;
    arr << QPoint(x     , rect.top()-1);
    arr << QPoint(x - dx, rect.top()-1);
    arr << QPoint(x     , qRound(rect.top() + dy));
    arr << QPoint(x - dx, qRound(rect.top() + dy * 2));
    arr << QPoint(x     , qRound(rect.top() + dy * 3));
    arr << QPoint(x - dx, rect.bottom()+1);
    arr << QPoint(x     , rect.bottom()+1);
    arr << QPoint(x     , rect.top());
    QPainterPath trimPath;
    trimPath.addPolygon(arr);

    path = path.subtracted(trimPath);
}*/


void GSequenceLineViewAnnotatedRenderArea::drawAnnotation(QPainter& p, DrawAnnotationPass pass, Annotation* a, const QPen& borderPen, bool selected, 
                                                          const AnnotationSettings* as, U2Region predefinedy) {
    if (as == NULL) {
        AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
        as = asr->getAnnotationSettings(a->getAnnotationName());
    }
    if (!as->visible && (pass == DrawAnnotationPass_DrawFill || !selected)) {
        return;
    }
    const GSequenceLineViewAnnotated::DrawSettings& drawSettings = getGSequenceLineViewAnnotated()->getDrawSettings();
    const U2Region& vr = view->getVisibleRange();

    QVector<U2Region> location = a->getRegions();
    bool simple = location.size() == 1;
    for (int ri = 0, ln = location.size(); ri < ln; ri++) {
        const U2Region& r = location.at(ri);
        if (!r.intersects(vr) || predefinedy.startPos < 0) {
            continue;
        }
        U2Region visibleLocation = r.intersect(vr);
        U2Region y = predefinedy.isEmpty() ? getAnnotationYRange(a, ri, as) : predefinedy;
        if (y.startPos <  0){
            continue;
        }
        float x1f = posToCoordF(visibleLocation.startPos);
        float x2f = posToCoordF(visibleLocation.endPos());
        assert(x2f >= x1f);

        int rw = qMax(selected ? MIN_SELECTED_ANNOTATION_WIDTH : MIN_ANNOTATION_WIDTH, qRound(x2f - x1f));
        int x1 = qRound(x1f);
        
        QRect annotationRect(x1, y.startPos, rw, y.length);
        QPainterPath rectPath;
        rectPath.addRect(x1, y.startPos, rw, y.length);
        bool leftTrim  = visibleLocation.startPos != r.startPos;
        bool rightTrim = visibleLocation.endPos()!= r.endPos();
        if (drawSettings.drawAnnotationArrows && (!leftTrim && !rightTrim)) {
            addArrowPath(rectPath, annotationRect, a->getStrand().isCompementary());
        } /* else  {
            if (leftTrim) {
                addTrimPath(rectPath, annotationRect, true);
            }
            if (rightTrim) {
                addTrimPath(rectPath, annotationRect, false);
            }
        } */
        if (pass == DrawAnnotationPass_DrawFill) {
            rectPath.setFillRule(Qt::WindingFill);
            p.fillPath(rectPath, as->color);
            p.fillPath(rectPath, gradientMaskBrush);
        } else {
            assert(pass == DrawAnnotationPass_DrawBorder);
            p.setPen(borderPen);
            if (rw > MIN_ANNOTATION_WIDTH) {
                p.drawPath(rectPath);
                if (drawSettings.drawAnnotationNames &&  annotationRect.width() >= MIN_ANNOTATION_TEXT_WIDTH) {
                    QString aText = GSequenceLineViewAnnotated::prepareAnnotationText(a, as);
                    drawBoundedText(p, annotationRect, aText);
                }
                if (simple && annotationRect.width() > MIN_WIDTH_TO_DRAW_EXTRA_FEATURES) {
                    if (drawSettings.drawCutSites) {
                        QString cutStr = a->findFirstQualifierValue(GBFeatureUtils::QUALIFIER_CUT);
                        bool hasD = false, hasC = false;
                        int cutD = 0, cutC = 0;
                        if (!cutStr.isEmpty()) {
                            int complSplit = cutStr.indexOf('/');
                            if (complSplit != -1) {
                                cutD = cutStr.left(complSplit).toInt(&hasD);
                                cutC = cutStr.mid(qMin(cutStr.length(), complSplit+1)).toInt(&hasC);
                            } else {
                                cutD = cutStr.toInt(&hasD);
                                cutC = cutD;
                                hasC = hasD;
                            }
                        }
                        if (hasD) {
                            drawCutSite(p, annotationRect, r.startPos + cutD, true);
                        }
                        if (hasC) {
                            drawCutSite(p, annotationRect, r.endPos() - cutC, false);
                        }
                    }
                }
            }
            drawAnnotationConnections(p, a, as);
        } 
    } 
}

void GSequenceLineViewAnnotatedRenderArea::drawAnnotationsSelection(QPainter& p) {
    ADVSequenceObjectContext* ctx = view->getSequenceContext();

    QPen pen1(Qt::SolidLine);
    pen1.setWidth(1);

    QPen pen2(Qt::SolidLine);
    pen2.setWidth(2);

    foreach(const AnnotationSelectionData& asd, ctx->getAnnotationsSelection()->getSelection()) {
        AnnotationTableObject* o = asd.annotation->getGObject();
        if (ctx->getAnnotationObjects(true).contains(o)) {
            if (annotationsCanOverlap) {
                drawAnnotation(p, DrawAnnotationPass_DrawFill,   asd.annotation, pen1, true);
            }
            drawAnnotation(p, DrawAnnotationPass_DrawBorder, asd.annotation, pen2, true);
        }
    }
}


void GSequenceLineViewAnnotatedRenderArea::drawBoundedText(QPainter& p, const QRect& r, const QString& text) const {
    if (afSmallCharWidth > r.width()) {
        return;
    }
    QFont *font = afNormal;
    QFontMetrics* fm = afmNormal;
    if (fm->width(text) > r.width()) {
        font = afSmall;
        fm = afmSmall;
    }
    p.setFont(*font);

    int len = text.length();
    int textWidth = 0, prefixLen = 0;
    do {
        int cw = fm->width(text[prefixLen]);
        if (textWidth + cw > r.width()) {
            break;
        }
        textWidth+=cw;
    } while (++prefixLen < len);
    
    if (prefixLen == 0) {
        return;
    }
    p.drawText(r, Qt::TextSingleLine | Qt::AlignCenter, text.left(prefixLen));
}


#define CUT_SITE_HALF_WIDTH  4
#define CUT_SITE_HALF_HEIGHT 2
void GSequenceLineViewAnnotatedRenderArea::drawCutSite(QPainter& p, const QRect& rect, int pos, bool direct) {
    int xCenter = posToCoord(pos, true);
    if (xCenter < rect.left() || xCenter > rect.right()) {
        return; // drawing cut sites out of the annotation region is not supported
    }
    int xLeft = xCenter - CUT_SITE_HALF_WIDTH;
    int xRight= xCenter + CUT_SITE_HALF_WIDTH;
    int yFlat = direct ? rect.top() - CUT_SITE_HALF_HEIGHT : rect.bottom() + CUT_SITE_HALF_HEIGHT;
    int yPeak = direct ? rect.top() + CUT_SITE_HALF_HEIGHT : rect.bottom() - CUT_SITE_HALF_HEIGHT;
    
    p.drawLine(xLeft, yFlat, xRight, yFlat);
    p.drawLine(xLeft, yFlat, xCenter, yPeak);
    p.drawLine(xRight, yFlat, xCenter, yPeak);
}


#define MAX_VIRTUAL_RANGE 10000

void GSequenceLineViewAnnotatedRenderArea::drawAnnotationConnections(QPainter& p, Annotation* a, const AnnotationSettings* as) {
    if (a->getLocation()->isSingleRegion()) {
        return;
    }
    const GSequenceLineViewAnnotated::DrawSettings& drawSettings = getGSequenceLineViewAnnotated()->getDrawSettings();
    const U2Region& visibleRange = view->getVisibleRange();
    int dx1 = 0;
    int dx2 = 0;
    if (drawSettings.drawAnnotationArrows) {
        if (a->getStrand().isCompementary()) {
            dx2 = - FEATURE_ARROW_HLEN;
        } else {
            dx1 = FEATURE_ARROW_HLEN;
        }
    }
    QVector<U2Region> location = a->getRegions();
    for (int ri = 0, ln = location.size(); ri < ln; ri++) {
        const U2Region& r = location.at(ri);
        U2Region y = getAnnotationYRange(a, ri, as);
        if (ri > 0) {
            U2Region prev = location.at(ri - 1);
            int prevPos = prev.endPos();
            int pos = r.startPos;
            int min = qMin(prevPos, pos);
            int max = qMax(prevPos, pos);
            if (visibleRange.intersects(U2Region(min, max-min))) {
                int x1 = posToCoord(prevPos, true) + dx1;
                int x2 = posToCoord(pos, true) + dx2;
                if (qAbs(x2-x1) > 1) {
                    x1 = qBound(-MAX_VIRTUAL_RANGE, x1, MAX_VIRTUAL_RANGE); //qt4.4 crashes in line clipping alg for extremely large X values
                    x2 = qBound(-MAX_VIRTUAL_RANGE, x2, MAX_VIRTUAL_RANGE);
                    int midX = (x1 + x2) / 2;
                    U2Region pyr = getAnnotationYRange(a, ri - 1, as);
                    U2Region yr = getAnnotationYRange(a, ri, as);
                    int y1 = pyr.startPos;
                    int dy1 = pyr.length/2;
                    int y2 = yr.startPos;
                    int dy2 = yr.length/2;
                    int midY = qMin(y1, y2);
                    p.drawLine(x1, y1+dy1, midX, midY);
                    p.drawLine(midX, midY, x2, y2+dy2);
                }
            }
        }
    }
}

void GSequenceLineViewAnnotated::ensureVisible(Annotation* a, int locationIdx) {
    QVector<U2Region> location = a->getRegions();
    assert(locationIdx < location.size());
    if (locationIdx == -1) {
        foreach(const U2Region& r, location) {
            if (visibleRange.intersects(r)) {
                return;
            }
        }
    }
    const U2Region& region = location[qMax(0, locationIdx)];
    if (!visibleRange.intersects(region)) {
        qint64 pos = a->getStrand().isCompementary() ? region.endPos() : region.startPos;
        setCenterPos(qBound(qint64(0), pos, seqLen-1));
    }
}

bool GSequenceLineViewAnnotated::event(QEvent* e) {
    if (e->type() == QEvent::ToolTip) {
        QHelpEvent* he = static_cast<QHelpEvent*>(e);
        QString tip = createToolTip(he);
        if (!tip.isEmpty()) {
            QToolTip::showText(he->globalPos(), tip);
        }
        return true;
    }
    return GSequenceLineView::event(e);
}

QString GSequenceLineViewAnnotated::createToolTip(QHelpEvent* e) {
    const int ROWS_LIMIT = 25;
    QList<AnnotationSelectionData> la = selectAnnotationByCoord(e->pos());
    if (la.isEmpty()) return QString();
    QString tip = "<table>";
    int rows = 0;
    if (la.size() > 1) {
        foreach(AnnotationSelectionData ad, la) {
            rows += ad.annotation->getQualifiers().size() + 1;
        }
    }
    bool skipDetails = rows > ROWS_LIMIT;
    rows = 0;
    foreach(AnnotationSelectionData ad, la) {
        if (++rows > ROWS_LIMIT) break;
        QString aname = ad.annotation->getAnnotationName();
        QColor acl = AppContext::getAnnotationsSettingsRegistry()->getAnnotationSettings(aname)->color;
        tip += "<tr><td bgcolor="+acl.name()+" bordercolor=black width=15></td><td><big>"+aname+"</big></td></tr>";
        //if (ad.annotation->getQualifiers().size() !=0)
        {
            if (skipDetails) {
                tip += "<tr><td/><td>...</td>";
                rows++;
            } else {
                tip += "<tr><td/><td>";
                tip += ad.annotation->getQualifiersTip(ROWS_LIMIT - rows, getSequenceObject(), ctx->getComplementTT(), ctx->getAminoTT());
                tip += "</td></tr>";
                rows += ad.annotation->getQualifiers().size();
            }
        }
    }
    tip += "</table>";
    if (rows > ROWS_LIMIT) {
        tip += "<hr> <div align=center>"+tr("etc ...")+"</div>"; 
    }
    return tip;
}

void GSequenceLineViewAnnotated::sl_onAnnotationsModified( const AnnotationModification& md )
{
    if (md.type == AnnotationModification_LocationChanged || md.type == AnnotationModification_NameChanged ) {
        addUpdateFlags(GSLV_UF_AnnotationsChanged);
        update();
    }
}
bool GSequenceLineViewAnnotatedRenderArea::isAnnotationSelectionInVisibleRange() const {
    const QSet<AnnotationTableObject*> aos = view->getSequenceContext()->getAnnotationObjects(true);
    AnnotationSelection* as = view->getSequenceContext()->getAnnotationsSelection();
    foreach(const AnnotationSelectionData& asd, as->getSelection()) {
        if (!aos.contains(asd.annotation->getGObject())) {
            continue;
        }
        if (getGSequenceLineViewAnnotated()->isAnnotationVisible(asd.annotation)) {
            return true;
        }
    }
    return false;
}


} // namespace
