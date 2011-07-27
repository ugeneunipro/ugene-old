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

#include "CircularItems.h"
#include "CircularView.h"
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/AnnotationSettings.h>
#include <U2Gui/GraphUtils.h>
#include <U2Core/AppContext.h>
#include <QtGui/QGraphicsLineItem>

namespace U2 {

/************************************************************************/
/* CircularAnnotationItem                                               */
/************************************************************************/

CircularAnnotationItem::CircularAnnotationItem(Annotation* ann, CircurlarAnnotationRegionItem* region, CircularViewRenderArea* _ra) : ra(_ra) {
    assert(region->parent == NULL);
    region->parent = this;
    regions.append(region);
    this->annotation = ann;
    _boundingRect = region->boundingRect();
    isSelected = false;
}

CircularAnnotationItem::CircularAnnotationItem(Annotation* ann, QList<CircurlarAnnotationRegionItem*>& _regions, CircularViewRenderArea* _ra) : regions(_regions), ra(_ra) {
    this->annotation = ann;
    isSelected = false;
    QPainterPath path;
    foreach(CircurlarAnnotationRegionItem* item, regions) {
        assert(item->parent == NULL);
        item->parent = this;
        path = path.united(item->path());
    }
    _boundingRect = path.boundingRect();
}


CircularAnnotationItem::~CircularAnnotationItem()
{
    foreach(CircurlarAnnotationRegionItem* item, regions) {
        delete item->label;
        delete item;
    }
    regions.clear();
}

void CircularAnnotationItem::paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget) {
    Q_UNUSED(item);
    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* as = asr->getAnnotationSettings(annotation->getAnnotationName());
    this->color = as->color;
    foreach(CircurlarAnnotationRegionItem* item, regions) {
        item->paint(p, NULL, widget);
    }
}

QRectF CircularAnnotationItem::boundingRect() const{
    return _boundingRect;
}

Annotation* CircularAnnotationItem::getAnnotation() const
{
    return annotation;
}

bool CircularAnnotationItem::contains(const QPointF &point) {
    if(_boundingRect.contains(point)) {
        foreach(CircurlarAnnotationRegionItem* item, regions)
            if(item->contains(point)) {
                return true;
            }
    }
    return false;
}

int CircularAnnotationItem::containsRegion(const QPointF &point) {
    if(_boundingRect.contains(point)) {
        foreach(CircurlarAnnotationRegionItem* item, regions) {
            if(item->contains(point)) {
                return item->number;
            }
        }
    }
    return -1;
}

void CircularAnnotationItem::setSelected( bool isSelected ) {
    this->isSelected = isSelected;
}

const QList<CircurlarAnnotationRegionItem*>& CircularAnnotationItem::getRegions() {
    return regions;
}

/************************************************************************/
/* CircurlarAnnotationRegionItem                                        */
/************************************************************************/

CircurlarAnnotationRegionItem::CircurlarAnnotationRegionItem(const QPainterPath& path, bool _isShort, int _number ) 
    :QGraphicsPathItem(path), parent(NULL), label(NULL), number(_number), isShort(_isShort) {
}

void CircurlarAnnotationRegionItem::paint( QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */ ) {
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
    QRadialGradient radialGrad(QPointF(0, 0), outerRadius );
    radialGrad.setColorAt(1, color);
    radialGrad.setColorAt((float)innerRadius / (float)outerRadius, QColor(color.red()*0.7,color.green()*0.7,color.blue()*0.7));
    radialGrad.setColorAt(0, Qt::black);

    p->fillPath(path(), radialGrad);

    if(!isShort || parent->isSelected) {
        p->drawPath(path());
    }
}

CircularAnnotationLabel* CircurlarAnnotationRegionItem::getLabel() const {
    return label;
}

void CircurlarAnnotationRegionItem::setLabel( CircularAnnotationLabel* label ) {
    this->label = label;
}

/************************************************************************/
/* CircularAnnotationLabel                                              */
/************************************************************************/

static bool labelLengthLessThan(CircularAnnotationLabel* l1, CircularAnnotationLabel* l2) {
    int length1 =  l1->getAnnotation()->getRegions()[l1->getRegion()].length;
    int length2 =  l2->getAnnotation()->getRegions()[l2->getRegion()].length;
    return length1 < length2;
}

CircularAnnotationLabel::CircularAnnotationLabel( Annotation* ann, int _region, int sequenceLength, const QFont& font, CircularViewRenderArea* renderArea ) : 
annotation(ann), labelFont(font), region(_region), ra(renderArea), hasPosition(false), inner(false), seqLen(sequenceLength)
{
    const U2Region& r = annotation->getRegions()[region];
    qreal startAngle = (float)r.startPos / (float)sequenceLength * 360;
    qreal spanAngle = qMin((float)r.length / (float)sequenceLength * 360, float(360 - startAngle));
    startAngle+=renderArea->rotationDegree;
    float middleAngle = 360 - (startAngle + spanAngle /2.0);
    annotationAngle = middleAngle;
    if (annotationAngle<0) {
        annotationAngle+=360;
    }

    startA = startAngle*PI/180;
    endA = spanAngle*PI/180 + startA;
    spanA = spanAngle*PI/180;
    if(startA>2*PI) {
        startA -=2*PI;
    }
    else if(startA<0) {
        startA +=2*PI;
    }
    if(endA>2*PI) {
        endA -=2*PI;
    }
    else if(endA<0) {
        endA +=2*PI;
    }

    const int yLevel = renderArea->annotationYLevel[annotation];
    //const int count = renderArea->regionY.count();

    midRect = QRectF(-renderArea->middleEllipseSize/2 - yLevel * renderArea->ellipseDelta/2,
        -renderArea->middleEllipseSize/2 - yLevel * renderArea->ellipseDelta/2, 
        renderArea->middleEllipseSize + yLevel * renderArea->ellipseDelta,
        renderArea->middleEllipseSize + yLevel * renderArea->ellipseDelta);
    setVisible(false);

    AnnotationSettingsRegistry* asr = AppContext::getAnnotationsSettingsRegistry();
    AnnotationSettings* as = asr->getAnnotationSettings(annotation->getAnnotationName());
    labelText=GSequenceLineViewAnnotated::prepareAnnotationText(annotation, as);
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
        //inner positions
        QVector<QRect>& rects = ra->labelEmptyInnerPositions;
        QVector<int> indexes2;
        getVacantInnerPositions(rects, indexes2);
        closest = findClosestPoint(midRegionPoint.toPoint(), rects, indexes2);

        if(-1==closest) {
            return;
        }
        QRect rect = rects.at(closest);
        labelPos = QPoint(rect.topLeft());
        rects.remove(closest);
        inner=true;
        //assume innerPositions have predefined order: pos[i].x>0, pos[i+1]<0
        if(labelPos.x()<0) {
            assert(rects.size()>closest-1);
            int prevWidth = rects[closest-1].width();
            rects[closest-1].setWidth(prevWidth - rect.width());
        }
        else {
            assert(rects.size()>closest);
            int prevWidth = rects[closest].width();
            rects[closest].setWidth(prevWidth - rect.width());
        }
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

void CircularAnnotationLabel::paint( QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget )
{
    Q_UNUSED(item);
    Q_UNUSED(widget);

    if(!isVisible() || !hasPosition) {
        return;
    }

    CircularAnnotationItem* ai= ra->circItems.value(annotation);

    QPen pen(Qt::black);
    pen.setWidth(1);
    labelFont.setBold(false);
    if(ai->isSelected) {
        labelFont.setBold(true);
        p->setFont(labelFont);
        pen.setWidth(2);
    }
    p->setPen(pen);

    QRectF bound = boundingRect();

    p->fillRect(bound, Qt::white);
    p->drawText(bound, labelText);
    pen.setStyle(Qt::DotLine);
    p->setPen(pen);
    p->drawLine(labelPos, connectionPoint);
    pen.setStyle(Qt::SolidLine);
    p->setPen(pen);
}

Annotation* CircularAnnotationLabel::getAnnotation() const {
    return annotation;
}

QRectF CircularAnnotationLabel::boundingRect() const {
    QFontMetrics fm(labelFont);
    QRectF bound = fm.boundingRect(labelText+' ');
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

bool CircularAnnotationLabel::contains( const QPointF &point ) {
    return boundingRect().contains(point) || QGraphicsLineItem(QLine(labelPos, connectionPoint)).contains(point);
}

int CircularAnnotationLabel::getRegion() const {
    return region;
}

} // namespace U2
