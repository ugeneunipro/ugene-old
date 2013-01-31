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

#include "GraphicsUnrootedBranchItem.h"
#include "GraphicsRectangularBranchItem.h"
#include "GraphicsButtonItem.h"
#include "TreeViewerUtils.h"

#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtCore/QStack>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsView>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/AppContext.h>
#include <qmath.h>

namespace U2 {

GraphicsUnrootedBranchItem::GraphicsUnrootedBranchItem(QGraphicsItem* parent, qreal angle, GraphicsRectangularBranchItem* from): GraphicsBranchItem(true) {
    setParentItem(parent);
    qreal w = from->getWidth();
    settings = from->settings;
    setWidthW(w);
    setDist(from->getDist());
    setPos(w, 0);
    angle = from->getDirection() == GraphicsBranchItem::up ? angle : -angle;
    setTransform(QTransform().translate(-w, 0).rotate(angle).translate(w, 0));
//    setTransformOriginPoint(-w, 0);
//    setRotation(angle);

    if (from->getNameText() != NULL) {
        nameText = new QGraphicsSimpleTextItem(from->getNameText()->text(), this);
        nameText->setFont(from->getNameText()->font());
        QRectF rect = nameText->boundingRect();
        qreal h = rect.height();
        nameText->setPos(GraphicsBranchItem::TextSpace, -h * 0.5);
        if (nameText->scenePos().x() < 0.0) {
            QPointF p = rect.center();
            nameText->setTransform(QTransform().translate(p.x(), p.y()).rotate(180).translate(-p.x(), -p.y()));
//            nameText->setTransformOriginPoint(rect.center());
//            nameText->setRotation(180);
        }

        nameText->setBrush(from->getNameText()->brush());
    }
    if (from->getDistanceText() != NULL) {
        distanceText = new QGraphicsSimpleTextItem(from->getDistanceText()->text(), this);
        distanceText->setFont(from->getDistanceText()->font());
        QRectF rect = distanceText->boundingRect();
        if (distanceText->scenePos().x() < 0) {
            QPointF p(rect.center().x(), rect.height());
            distanceText->setTransform(QTransform().translate(p.x(), p.y()).rotate(180).translate(-p.x(), -p.y()));
//            distanceText->setTransformOriginPoint(rect.center().x(), rect.height());
//            distanceText->setRotation(180);
        }
        distanceText->setPos(-0.5 * (w + rect.width()), -rect.height());

        distanceText->setBrush(from->getDistanceText()->brush());
    }
    setPen(from->pen());
}

QRectF GraphicsUnrootedBranchItem::boundingRect() const {
    qreal penWidth = 1;
    return QRectF(-width, -penWidth * 0.5, width, penWidth);
}

void GraphicsUnrootedBranchItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {
    painter->setPen(pen());
    painter->drawLine(0, 0, -width, 0);
}

}//namespace
