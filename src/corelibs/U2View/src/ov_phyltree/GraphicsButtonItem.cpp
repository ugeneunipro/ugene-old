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

#include "GraphicsButtonItem.h"
#include "GraphicsBranchItem.h"
#include "GraphicsRectangularBranchItem.h"
#include <U2Core/U2SafePoints.h>
#include <U2Core/PhyTreeObject.h>
#include <QtGui/QPainter> 
#include <QtGui/QPen>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtCore/QList>
#include <QtGui/QMenu>
#include <QtGui/QGraphicsScene>

namespace U2 {

const qreal GraphicsButtonItem::radiusMin = 3.0;
const qreal GraphicsButtonItem::radiusMax = 4.0;
const QBrush GraphicsButtonItem::highlightingBrush = QBrush(QColor(170, 170, 230));
const QBrush GraphicsButtonItem::ordinaryBrush = QBrush(Qt::gray);


GraphicsButtonItem::GraphicsButtonItem() 
    : QGraphicsEllipseItem(QRectF(-radiusMin, -radiusMin, 2 * radiusMin, 2 * radiusMin)), isSelected(false) {
    setPen(QColor(0, 0, 0));
    setBrush(ordinaryBrush);
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton);
    setZValue(2);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setToolTip(QObject::tr("Left click to select the branch\nDouble-click to collapse the branch"));
}

void GraphicsButtonItem::mousePressEvent(QGraphicsSceneMouseEvent *e) {
    uiLog.trace("Tree button pressed");

    bool shiftPressed = e->modifiers() & Qt::ShiftModifier;
    bool leftButton = e->button() == Qt::LeftButton;
    GraphicsBranchItem *p = dynamic_cast<GraphicsBranchItem*>(parentItem());
    if (leftButton && p!=NULL) {
        bool newSelection = true;
        if (shiftPressed) {
            newSelection = !isSelected;
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

void GraphicsButtonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    if(isSelected) {
        return;
    }
    QGraphicsItem::hoverEnterEvent(event);
    setHighlighting(true);
}
void GraphicsButtonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    if(isSelected) {
        return;
    }
    QGraphicsItem::hoverLeaveEvent(event);
    setHighlighting(false);
}

void GraphicsButtonItem::setSelected(bool selected){
    isSelected = selected;
    if(selected) {
        setHighlighting(true);
    }
    else {
        setHighlighting(false);
    }
}

void GraphicsButtonItem::setHighlighting(bool enabled) {
    if(enabled) {
        setRect(QRectF(-radiusMax, -radiusMax, 2 * radiusMax, 2 * radiusMax));
        setBrush(highlightingBrush);
    }
    else {
        setRect(QRectF(-radiusMin, -radiusMin, 2 * radiusMin, 2 * radiusMin));
        setBrush(ordinaryBrush);
    }
    update();
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

    GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(parentItem());
    if (!branchItem) {
        return;
    }

    GraphicsRectangularBranchItem *rectBranchItem = dynamic_cast<GraphicsRectangularBranchItem*>(branchItem);
    if(!rectBranchItem){
        if(!branchItem->getCorrespondingItem()){
            return;
        }
        
        rectBranchItem = dynamic_cast<GraphicsRectangularBranchItem*>(branchItem->getCorrespondingItem());
        if(!rectBranchItem){
            return;
        }
    }

    rectBranchItem->swapSiblings();
}

bool GraphicsButtonItem::isSelectedTop() {
    if (!isSelected) {
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

void GraphicsButtonItem::rerootTree(PhyTreeObject* treeObject) {
    uiLog.trace("Rerooting of the PhyTree");
    SAFE_POINT(NULL != treeObject, "Null pointer argument 'treeObject' was passed to 'PhyTreeUtils::rerootPhyTree' function",);

    GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(parentItem());
    CHECK(NULL != branchItem , );

    GraphicsRectangularBranchItem *rectBranchItem = dynamic_cast<GraphicsRectangularBranchItem*>(branchItem);
    if(NULL == rectBranchItem){
        CHECK(branchItem->getCorrespondingItem(), );

        rectBranchItem = dynamic_cast<GraphicsRectangularBranchItem*>(branchItem->getCorrespondingItem());
        CHECK(NULL != rectBranchItem,);
    }

    const PhyBranch* nodeBranch = rectBranchItem->getPhyBranch();
    CHECK(NULL != nodeBranch, );
    PhyNode* newRoot = nodeBranch->node2;
    CHECK(NULL != newRoot, );

    treeObject->rerootPhyTree(newRoot);
}

}//namespace
