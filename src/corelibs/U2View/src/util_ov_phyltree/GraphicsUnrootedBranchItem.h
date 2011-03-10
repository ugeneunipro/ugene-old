#ifndef _U2_GRAPHICS_UNROOTED_BRANCH_ITEM_H_
#define _U2_GRAPHICS_UNROOTED_BRANCH_ITEM_H_

#include <U2Core/Task.h>
#include "GraphicsBranchItem.h"

class QGraphicsItem;

namespace U2 {

class PhyNode;
class GraphicsButtonItem;
class GraphicsRectangularBranchItem;

class GraphicsUnrootedBranchItem: public GraphicsBranchItem {
public:
    GraphicsUnrootedBranchItem(QGraphicsItem* parent, qreal angle, GraphicsRectangularBranchItem* from);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
};

}//namespace;

#endif
