#ifndef _U2_GRAPHICS_CIRCULAR_BRANCH_ITEM_H_
#define _U2_GRAPHICS_CIRCULAR_BRANCH_ITEM_H_

#include <U2Core/Task.h>
#include "GraphicsBranchItem.h"

class QGraphicsItem;

namespace U2 {

class PhyNode;
class GraphicsButtonItem;
class GraphicsRectangularBranchItem;

class GraphicsCircularBranchItem: public GraphicsBranchItem {
    qreal height;
    Direction direction;
    bool visible;

public:
    GraphicsCircularBranchItem(QGraphicsItem* parent, qreal height, GraphicsRectangularBranchItem* from);

    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setVisibleW(bool v) { visible = v; }
};

}//namespace;

#endif
