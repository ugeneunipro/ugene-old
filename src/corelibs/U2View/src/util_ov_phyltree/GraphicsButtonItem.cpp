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

#include "GraphicsButtonItem.h"
#include "GraphicsBranchItem.h"
#include "GraphicsRectangularBranchItem.h"
#include <QtGui/QPainter>
#include <QtGui/QPen>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QList>
#include <QtGui/QMenu>
#include <QtGui/QGraphicsScene>

namespace U2 {

const qreal GraphicsButtonItem::radiusMin = 2.0;
const qreal GraphicsButtonItem::radiusMax = 5.0;

GraphicsButtonItem::GraphicsButtonItem() {
    setPen(QColor(0, 0, 0));
    setBrush(Qt::gray);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(2);
    setFlag(QGraphicsItem::ItemIsSelectable);
}

QRectF GraphicsButtonItem::boundingRect() const {
    return QRectF(-radiusMax, -radiusMax, 2 * radiusMax, 2 * radiusMax);
}

QPainterPath GraphicsButtonItem::shape() const {
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void GraphicsButtonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem*, QWidget*) {

    QBrush br(Qt::gray);

    int radius = radiusMin;
    bool mouseHovered = isUnderMouse();
    bool itemSelected = isSelected();

    if (mouseHovered || itemSelected) {
        radius = radiusMax;
        br = QBrush(QColor(0, 0, 255));
    }
    painter->setPen(pen());
    painter->setBrush(br);
    painter->drawEllipse(QPointF(0, 0), radius, radius);

}

void GraphicsButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    uiLog.trace("Tree button pressed");

    bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
    bool leftButton = e->button() == Qt::LeftButton;
    GraphicsBranchItem *p = dynamic_cast<GraphicsBranchItem*>(parentItem());
    if (leftButton && p!=NULL) {
        bool newSelection = true;
        if (shiftPressed) {
            newSelection = !isSelected();
        }

        p->setSelectedRecurs(newSelection, true);

        e->accept();
        update();
    }
}

void GraphicsButtonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
    uiLog.trace("Tree button double-clicked");
    collapse();
    QAbstractGraphicsShapeItem::mouseDoubleClickEvent(e);
}

void GraphicsButtonItem::collapse() {
    GraphicsBranchItem *p = dynamic_cast<GraphicsBranchItem*>(parentItem());
    Q_ASSERT(p);
    if (p) {
        p->collapse();
    }
}

void GraphicsButtonItem::swapSiblings() {
    uiLog.trace("Swapping siblings");

    GraphicsRectangularBranchItem *branchItem = dynamic_cast<GraphicsRectangularBranchItem*>(parentItem());
    if (!branchItem) {
        return;
    }

    branchItem->swapSiblings();
}

bool GraphicsButtonItem::isSelectedTop() {
    if (!isSelected()) {
        return false;
    }
    GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(parentItem());
    if (!branchItem) {
        return true;
    }
    GraphicsBranchItem *parentBranchItem = dynamic_cast<GraphicsBranchItem*>(branchItem->parentItem());
    if (!parentBranchItem) {
        return true;
    }
    bool parentBranchSelected = parentBranchItem->isSelected();
    return !parentBranchSelected;
}

bool GraphicsButtonItem::isCollapsed(){
    GraphicsBranchItem *p = dynamic_cast<GraphicsBranchItem*>(parentItem());
    Q_ASSERT(p);
    if (p) {
        return p->isCollapsed();
    }
    return false;
    
}

}//namespace
