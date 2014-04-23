/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#ifndef _CIRCULAR_ITEMS_H_
#define _CIRCULAR_ITEMS_H_

#include <QtGui/QFont>
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QGraphicsPathItem>
#else
#include <QtWidgets/QGraphicsPathItem>
#endif

#include <U2Core/Annotation.h>

namespace U2 {

class AnnotationSettings;
class CircularAnnotationItem;
class CircularViewRenderArea;
class CircularAnnotationLabel;
class CircularAnnotationRegionItem;

/************************************************************************/
/* CircularAnnotationItem                                               */
/************************************************************************/

class CircularAnnotationItem : public QGraphicsItem {
    Q_DISABLE_COPY(CircularAnnotationItem)
friend class CircularAnnotationRegionItem;
friend class CircularAnnotationLabel;
public:
    CircularAnnotationItem( const Annotation &ann, CircularAnnotationRegionItem* region, CircularViewRenderArea* _ra);
    CircularAnnotationItem( const Annotation &ann, QList<CircularAnnotationRegionItem*>& regions, CircularViewRenderArea* _ra);
    ~CircularAnnotationItem();
    virtual void paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget);
    Annotation getAnnotation() const;
    virtual QRectF boundingRect() const;
    virtual bool contains(const QPointF &point);
    int containsRegion(const QPointF &point);
    void setSelected(bool isSelected);
    const QList<CircularAnnotationRegionItem*>& getRegions();
protected:
    bool isSelected;
    QColor color;
    Annotation annotation;
    QList<CircularAnnotationRegionItem*> regions;
    QRectF _boundingRect;
    CircularViewRenderArea* ra;
};

/************************************************************************/
/* CircurlarAnnotationRegionItem                                        */
/************************************************************************/
class CircularAnnotationRegionItem : public QGraphicsPathItem{
friend class CircularAnnotationItem;
public:
    CircularAnnotationRegionItem(const QPainterPath& path, bool isShort, int number );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */);
    CircularAnnotationLabel* getLabel() const;
    void setLabel(CircularAnnotationLabel* label);
protected:
    CircularAnnotationItem *parent;
    CircularAnnotationLabel *label;
    int number;
    bool isShort;
};

/************************************************************************/
/* CircularAnnotationLabel                                              */
/************************************************************************/
class CircularAnnotationLabel : public QGraphicsItem {
public:
    CircularAnnotationLabel( const Annotation &ann, int _region, int sequenceLength, const QFont& font, CircularViewRenderArea* renderArea );
    virtual void paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget);
    Annotation getAnnotation() const;
    virtual QRectF boundingRect() const;
    virtual bool contains(const QPointF &point);
    int getRegion() const;
    void setLabelPosition();
    static void prepareLabels(QList<CircularAnnotationLabel*>& labelItems);

    bool operator<(const CircularAnnotationLabel& a);
private:
    int findClosestPoint(const QPoint& targetPoint, const QVector<QRect>& rects, QVector<int> indexes);
    void getVacantPositions(const QVector<QRect>& rects, QVector<int>& result);
    void getVacantInnerPositions(const QVector<QRect>& rects, QVector<int>& result);
    void getConnectionPoint();

    Annotation annotation;
    float annotationAngle;
    QFont labelFont;
    QRectF midRect;
    QPointF midRegionPoint;
    int region;

    qreal startA;
    qreal endA;
    qreal spanA;
    QPoint labelPos;
    QPoint connectionPoint;
    CircularViewRenderArea* ra;
    bool hasPosition;
    QString labelText;
    bool inner;
    int seqLen;
};

} // namespace U2

#endif // _CIRCULAR_ITEMS_H_
