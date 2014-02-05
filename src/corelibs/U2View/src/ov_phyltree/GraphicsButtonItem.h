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

#ifndef _U2_GRAPHICS_BUTTON_ITEM_H_
#define _U2_GRAPHICS_BUTTON_ITEM_H_

#include <QtGui/QGraphicsEllipseItem>
#include <QtGui/QBrush>

namespace U2 {

class PhyTreeObject;

class GraphicsButtonItem: public QGraphicsEllipseItem {
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    
public:
    GraphicsButtonItem();

    bool isSelectedTop();
    void collapse();
    void swapSiblings();
    bool isCollapsed();

    void setSelected(bool selected);

    void rerootTree(PhyTreeObject* treeObject);
private:
    void setHighlighting(bool enabled);

    static const qreal radiusMin;
    static const qreal radiusMax;
    static const QBrush highlightingBrush;
    static const QBrush ordinaryBrush;
    bool isSelected;
};

}//namespace;

#endif
