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

#include <QtCore/qglobal.h>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QGraphicsLineItem>
#else
#include <QtWidgets/QGraphicsLineItem>
#endif
#include <math.h>

#include <U2Core/AnnotationSettings.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/U2SafePoints.h>

#include <U2Gui/GraphUtils.h>

#include "CircularItems.h"
#include "CircularView.h"
#include "CircularViewPlugin.h"

namespace U2 {

/************************************************************************/
/* CircularAnnotationItem                                               */
/************************************************************************/

CircularAnnotationItem::CircularAnnotationItem(Annotation *ann, CircularAnnotationRegionItem* region, CircularViewRenderArea* _ra)
    : annotation(ann), ra(_ra)
{
    assert(region->parent == NULL);
    region->parent = this;
    regions.append(region);
    _boundingRect = region->boundingRect();
    isSelected = false;
}

CircularAnnotationItem::CircularAnnotationItem(Annotation *ann, QList<CircularAnnotationRegionItem*>& _regions, CircularViewRenderArea* _ra)
    : annotation(ann), regions(_regions), ra(_ra)
{
    isSelected = false;
    QPainterPath path;
    foreach(CircularAnnotationRegionItem* item, regions) {
        assert(item->parent == NULL);
        item->parent = this;
        path = path.united(item->path());
    }
    _boundingRect = path.boundingRect();
}


CircularAnnotationItem::~CircularAnnotationItem()
{
    foreach(CircularAnnotationRegionItem* item, regions) {
        delete item->label;
        delete item;
    }
    regions.clear();
}

void CircularAnnotationItem::paint(QPainter *p,const QStyleOptionGraphicsItem *item, QWidget *widget) {
    Q_UNUSED(item);
    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings *as = asr->getAnnotationSettings(annotation->getData());
    this->color = as->color;
    foreach (CircularAnnotationRegionItem *item, regions) {
        item->paint(p, NULL, widget);
    }
}

QRectF CircularAnnotationItem::boundingRect() const {
    return _boundingRect;
}

Annotation * CircularAnnotationItem::getAnnotation() const {
    return annotation;
}

bool CircularAnnotationItem::contains(const QPointF &point) const {
    if(_boundingRect.contains(point)) {
        foreach(CircularAnnotationRegionItem* item, regions)
            if(item->contains(point)) {
                return true;
            }
    }
    return false;
}

int CircularAnnotationItem::containsRegion(const QPointF &point) {
    CircularAnnotationRegionItem* r = getContainingRegion(point);
    return (r == NULL) ? -1 : r->number;
}

CircularAnnotationRegionItem* CircularAnnotationItem::getContainingRegion(const QPointF &point) {
    if(_boundingRect.contains(point)) {
        foreach(CircularAnnotationRegionItem* item, regions) {
            if(item->contains(point)) {
                return item;
            }
        }
    }
    return NULL;
}

void CircularAnnotationItem::setSelected(bool isSelected) {
    this->isSelected = isSelected;
}

const QList<CircularAnnotationRegionItem*>& CircularAnnotationItem::getRegions() {
    return regions;
}

/************************************************************************/
/* CircularAnnotationRegionItem                                        */
/************************************************************************/

CircularAnnotationRegionItem::CircularAnnotationRegionItem(const QPainterPath& path, bool _isShort, int _number)
    : QGraphicsPathItem(path),
      parent(NULL),
      label(NULL),
      number(_number),
      isShort(_isShort),
      arrowCenterPercentage(0)
{}

void CircularAnnotationRegionItem::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */) {
    Q_UNUSED(widget);
    Q_UNUSED(option);
    assert(parent!=NULL);
    QPen pen(Qt::black);
    pen.setWidth(1);
    if(parent->isSelected) {
        pen.setWidth(2);
    }

    p->setPen(pen);

    CircularViewRenderArea* renderArea = parent->ra;

    int yLevel = renderArea->annotationYLevel[parent->annotation];
    int innerRadius = renderArea->innerEllipseSize/2 + yLevel * renderArea->ellipseDelta/2;
    int outerRadius = renderArea->outerEllipseSize/2 + yLevel * renderArea->ellipseDelta/2;
    QColor &color(parent->color);
    QRadialGradient radialGrad(QPointF(0, 0), outerRadius);
    radialGrad.setColorAt(1, color);
    radialGrad.setColorAt((float)innerRadius / (float)outerRadius, QColor(color.red()*0.7,color.green()*0.7,color.blue()*0.7));
    radialGrad.setColorAt(0, Qt::black);

    p->fillPath(path(), radialGrad);

    if(!isShort || parent->isSelected) {
        p->drawPath(path());
    }
}

CircularAnnotationLabel* CircularAnnotationRegionItem::getLabel() const {
    return label;
}

void CircularAnnotationRegionItem::setLabel(CircularAnnotationLabel* label) {
    this->label = label;
}

/************************************************************************/
/* CircularAnnotationLabel                                              */
/************************************************************************/

static bool labelLengthLessThan(CircularAnnotationLabel* l1, CircularAnnotationLabel* l2) {
    int length1 = l1->getAnnotation()->getRegions()[l1->getRegion()].length;
    int length2 =  l2->getAnnotation()->getRegions()[l2->getRegion()].length;
    return length1 < length2;
}

CircularAnnotationLabel::CircularAnnotationLabel(Annotation *ann, const QVector<U2Region> &annLocation, bool isAutoAnnotation,
    int _region, int sequenceLength, const QFont &font, CircularViewRenderArea *renderArea)
    : annotation(ann),
      location(annLocation),
      isAutoAnnotation(isAutoAnnotation),
      labelFont(font),
      region(_region),
      ra(renderArea),
      regionItem(NULL),
      hasPosition(false),
      inner(false),
      seqLen(sequenceLength)
{
    const SharedAnnotationData &aData = annotation->getData();
    SAFE_POINT(0 <= region && region < location.size(), "Invalid location index", );
    const U2Region &r = location[region];
    const qreal startAngle = renderArea->rotationDegree + (float)r.startPos / (float)sequenceLength * 360;
    const qreal spanAngle = (float) r.length / (float) sequenceLength * 360;

    const float middleAngle = 360 - (startAngle + spanAngle / 2.0);
    annotationAngle = middleAngle;
    if (annotationAngle < 0) {
        annotationAngle += 360;
    }

    startA = startAngle * PI / 180;
    endA = spanAngle * PI / 180 + startA;
    spanA = spanAngle * PI / 180;
    if ( startA > 2 * PI) {
        startA -= 2 * PI;
    }
    else if (startA < 0) {
        startA += 2 * PI;
    }
    if (endA > 2 * PI) {
        endA -= 2 * PI;
    }
    else if (endA < 0) {
        endA += 2 * PI;
    }

    const int yLevel = renderArea->annotationYLevel[annotation];

    midRect = QRectF(-renderArea->middleEllipseSize / 2 - yLevel * renderArea->ellipseDelta / 2,
        -renderArea->middleEllipseSize / 2 - yLevel * renderArea->ellipseDelta / 2,
        renderArea->middleEllipseSize + yLevel * renderArea->ellipseDelta,
        renderArea->middleEllipseSize + yLevel * renderArea->ellipseDelta);
    setVisible(false);

    AnnotationSettingsRegistry *asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings *as = asr->getAnnotationSettings(aData);
    labelText = GSequenceLineViewAnnotated::prepareAnnotationText(aData, as);
}

void CircularAnnotationLabel::setAnnRegion(CircularAnnotationRegionItem *region) {
    regionItem = region;
}

bool CircularAnnotationLabel::canFitToTheRegion() const {
    CHECK(regionItem != NULL, false);

    QPainterPath path = regionItem->path();
    QFontMetrics metrics(labelFont, ra);
    qreal labelTextPixSize = metrics.width(labelText);
    // (can fit in width) && (can fit in height)
    if ((regionItem->getArrowCenterPercentage() * path.length() > labelTextPixSize / 2)
        && (metrics.height() + ra->FREE_SPACE_HEIGHT_FOR_INTERNAL_LABELS < ra->circularView->CV_REGION_ITEM_WIDTH))
    {
        return true;
    }
    return false;
}

void CircularAnnotationLabel::drawLabelInsideRegion(QPainter *p, bool canFit) {
    QPainterPath path = regionItem->path();
    SAFE_POINT(path.length() != 0, "Region path has zero length",);
    QFontMetrics metrics(labelFont,ra);

    if (metrics.height() + ra->FREE_SPACE_HEIGHT_FOR_INTERNAL_LABELS
            > ra->circularView->CV_REGION_ITEM_WIDTH) {
        // The font is too big for curent region width
        return;
    }

    QString labelText = this->labelText;
    qreal labelCenter = regionItem->getArrowCenterPercentage();
    if (!canFit) {
        qreal arrowShaft = path.length() * 2 * labelCenter;
        labelText = metrics.elidedText(labelText, Qt::ElideRight, arrowShaft);
    }

    int textWidth = metrics.size(Qt::TextSingleLine, labelText).width();
    qreal labelPercentage = textWidth / path.length();

    qreal percent = 0;
    qreal percentIncreaseSgn = -1;
    qreal arrowWidthPercentage = (ra->outerEllipseSize - ra->innerEllipseSize + 5) / (2 * path.length());

    const SharedAnnotationData &aData = annotation->getData();
    bool isDirect = aData->getStrand().isDirect();
    // percent of annotation arrow path where label start position is located
    if (isDirect) {
        qreal a = path.angleAtPercent(labelCenter);
        if (a < 90 || a > 270) {
            // top
            percent = 1 - labelCenter - arrowWidthPercentage + labelPercentage / 2;
        } else {
            // bottom
            percent = labelCenter + labelPercentage / 2;
        }
    } else {
        qreal a = path.angleAtPercent(1 - labelCenter);
        if (a < 90 || a > 270) {
            // bottom
            percent = 1 - labelCenter - labelPercentage / 2;
        } else {
            // top
            percent = 1 - labelCenter * 3 - arrowWidthPercentage - labelPercentage / 2;
        }
        percentIncreaseSgn = 1;
    }


    for (int i = 0; i < labelText.size(); i++) {
        QPointF point = path.pointAtPercent(percent);
        qreal angle = path.angleAtPercent(percent);   // Clockwise is negative

        p->save();
        // Move the virtual origin to the point on the curve
        p->translate(point);
        // Rotate to match the angle of the curve
        // Clockwise is positive so we negate the angle from above
        p->rotate(-angle + isDirect*180);
        // Draw a line width above the origin to move the text above the line
        // and let Qt do the transformations
        p->drawText(QPoint(0, - p->pen().width()), QString(labelText[i]));
        p->restore();

        percent += percentIncreaseSgn * metrics.width(labelText[i]) / path.length();
    }
}

void CircularAnnotationLabel::drawLabelOutsideRegion(QPainter *p, QPen& pen) {
    QRectF bound = boundingRect();
    p->fillRect(bound, Qt::white);
    p->drawText(bound, labelText);

    pen.setStyle(Qt::DotLine);
    p->setPen(pen);

    p->drawLine(labelPos, connectionPoint);
    pen.setStyle(Qt::SolidLine);
    p->setPen(pen);
}

void CircularAnnotationLabel::prepareLabels(QList<CircularAnnotationLabel*>& labelItems) {
    qSort<QList<CircularAnnotationLabel*>::iterator>(labelItems.begin(), labelItems.end(), labelLengthLessThan);
    foreach(CircularAnnotationLabel* l, labelItems) {
        l->setVisible(true);
    }
}

void CircularAnnotationLabel::setLabelPosition() {
    ////find closest empty label position
    QPointF midRegionPoint = QPointF (midRect.width()/2 * cos(annotationAngle / 180.0 * PI),
        -midRect.height()/2 * sin(annotationAngle / 180.0 * PI)).toPoint();

    QVector<QRect>& outerRects = ra->labelEmptyPositions;
    QVector<int> indexes;//nearby points
    getVacantPositions(outerRects, indexes);
    int closest = findClosestPoint(midRegionPoint.toPoint(), outerRects, indexes);

    if(-1==closest) {
        return;
    }
    else {
        QRect r = outerRects.at(closest);
        labelPos = QPoint(r.topLeft());
        outerRects.remove(closest);
    }

    getConnectionPoint();
    hasPosition=true;
}

int CircularAnnotationLabel::findClosestPoint(const QPoint& targetPoint, const QVector<QRect>& rects, QVector<int> indexes) {
    foreach(int idx, indexes) {
        assert(idx>=0 && idx<rects.size());
        if(boundingRect().width()>rects.at(idx).width()) {
            int i = indexes.indexOf(idx);
            indexes.remove(i);
        }
    }

    int size = indexes.size();
    if(0==size) {
        return -1;
    }

    int index = indexes.at(0);
    assert(index>=0&&index<rects.size());

    if(1==size) {
        return index;
    }

    QPoint rectTopLeft = rects.at(index).topLeft();
    QPoint diff = rectTopLeft - targetPoint;
    int minDistance = diff.x() * diff.x() + diff.y() * diff.y();
    int closest = index;

    foreach(int idx, indexes) {
        assert(idx>=0&&idx<rects.size());
        rectTopLeft = rects.at(idx).topLeft();
        diff = rectTopLeft - targetPoint;
        int currDistance = diff.x() * diff.x() + diff.y() * diff.y();
        if(minDistance>currDistance) {
            minDistance = currDistance;
            closest=idx;
        }
    }
    return closest;
}

void CircularAnnotationLabel::getVacantPositions(const QVector<QRect>& rects, QVector<int>& result) {
    foreach(QRect rect, rects) {
        qreal arcsin = CircularView::coordToAngle(rect.topLeft());
        if(startA>endA) { //region contains 2*PI angle
            if(arcsin>startA - PI/16 || arcsin<endA + PI/16) {
                result << rects.indexOf(rect);
            }
        } else {
            if(arcsin>startA - PI/16 && arcsin < endA + PI/16) {
                result << rects.indexOf(rect);
            }
        }
    }
}

void CircularAnnotationLabel::getVacantInnerPositions(const QVector<QRect>& rects, QVector<int>& result) {
    QPointF midRegionPoint = QPointF (midRect.width()/2 * cos(annotationAngle / 180.0 * PI),
        -midRect.height()/2 * sin(annotationAngle / 180.0 * PI)).toPoint();
    foreach(QRect r, rects) {
        if(midRegionPoint.y()>0) {
            qreal delta = qAbs(midRegionPoint.y() - r.bottom());
            if(delta<ra->height()/5.0) {
                result << rects.indexOf(r);
            }
        }
        else {
            qreal delta = qAbs(midRegionPoint.y() - r.top());
            if(delta<ra->height()/5.0) {
                result << rects.indexOf(r);
            }
        }
    }
}
#define CONNECTION_POINT_PAD PI/36
void CircularAnnotationLabel::getConnectionPoint() {
    qreal arcsin = CircularView::coordToAngle(labelPos);

    if(spanA<CONNECTION_POINT_PAD*2) {
        qreal midAngle = startA + spanA/2;
        connectionPoint.rx() = midRect.width()/2 * cos(midAngle);
        connectionPoint.ry() = midRect.width()/2 * sin(midAngle);
        return;
    }

    if(startA>endA) { //region contains 2*PI angle
        if(arcsin>startA+CONNECTION_POINT_PAD || arcsin<endA-CONNECTION_POINT_PAD) {
            connectionPoint = QPoint(midRect.width()/2 * cos(arcsin),
                midRect.width()/2 * sin(arcsin));
            return;
        }
    }
    else {
        if(arcsin>startA+CONNECTION_POINT_PAD && arcsin < endA-CONNECTION_POINT_PAD) {
            connectionPoint = QPoint(midRect.width()/2 * cos(arcsin),
                midRect.width()/2 * sin(arcsin));
            return;
        }
    }

    if(abs(startA-arcsin)<abs(endA-arcsin)) {
        connectionPoint = QPoint(midRect.width()/2 * cos(startA + CONNECTION_POINT_PAD),
            midRect.width()/2 * sin(startA + CONNECTION_POINT_PAD));
    }
    else {
        connectionPoint = QPoint(midRect.width()/2 * cos(endA - CONNECTION_POINT_PAD),
            midRect.width()/2 * sin(endA - CONNECTION_POINT_PAD));
    }
}

void CircularAnnotationLabel::paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget) {
    Q_UNUSED(item);
    Q_UNUSED(widget);

    if (!isVisible()) {
        return;
    }

    CircularAnnotationItem* ai= ra->circItems.value(annotation);

    p->save();
    QPen pen(Qt::black);
    pen.setWidth(1);
    labelFont.setBold(false);
    if(ai->isSelected) {
        labelFont.setBold(true);
        pen.setWidth(2);
    }
    p->setFont(labelFont);
    p->setPen(pen);

    bool canFit = canFitToTheRegion();

    SAFE_POINT(ra->settings != NULL, "Circular view settings is NULL",);
    if (ra->settings->labelMode == CircularViewSettings::Inside
        || (ra->settings->labelMode == CircularViewSettings::Mixed && canFit && !isAutoAnnotation))
    {
        drawLabelInsideRegion(p, canFit);
    }
    if (hasPosition && (ra->settings->labelMode == CircularViewSettings::Outside ||
            (ra->settings->labelMode == CircularViewSettings::Mixed && (!canFit || isAutoAnnotation)))) {
        drawLabelOutsideRegion(p, pen);
    }
    p->restore();
}

Annotation * CircularAnnotationLabel::getAnnotation() const {
    return annotation;
}

QRectF CircularAnnotationLabel::boundingRect() const {
    QFontMetrics fm(labelFont);
    QRectF bound = fm.boundingRect(" " + labelText);
    if(!inner) {
        if(labelPos.x()>0 && labelPos.y()>=0) {
            bound.moveTopLeft(labelPos);
        }
        else if(labelPos.x()<0 && labelPos.y()>=0) {
            bound.moveTopRight(labelPos);
        }
        else if(labelPos.x()<0 && labelPos.y()<0) {
            bound.moveBottomRight(labelPos);
        }
        else {
            bound.moveBottomLeft(labelPos);
        }
    }
    else {
        if(labelPos.x()>0 && labelPos.y()>=0) {
            bound.moveBottomRight(labelPos);
        }
        else if(labelPos.x()<0 && labelPos.y()>=0) {
            bound.moveBottomLeft(labelPos);
        }
        else if(labelPos.x()<0 && labelPos.y()<0) {
            bound.moveTopLeft(labelPos);
        }
        else {
            bound.moveTopRight(labelPos);
        }
    }
    return bound;
}

bool CircularAnnotationLabel::contains(const QPointF &point) const {
    return boundingRect().contains(point) || QGraphicsLineItem(QLine(labelPos, connectionPoint)).contains(point);
}

int CircularAnnotationLabel::getRegion() const {
    return region;
}

} // namespace U2
