#ifndef _U2_GRAPHICS_RECTANGULAR_BRANCH_ITEM_H_
#define _U2_GRAPHICS_RECTANGULAR_BRANCH_ITEM_H_

#include <U2Core/Task.h>
#include "GraphicsBranchItem.h"

namespace U2 {

class PhyNode;
class PhyBranch;
class GraphicsButtonItem;

class GraphicsRectangularBranchItem: public GraphicsBranchItem {
public:
    static const qreal DEFAULT_WIDTH;
    static const qreal MAXIMUM_WIDTH;
    static const int DEFAULT_HEIGHT;
    
    GraphicsRectangularBranchItem();
    GraphicsRectangularBranchItem(const QString& name, GraphicsRectangularBranchItem* pitem);
    GraphicsRectangularBranchItem(qreal d);
    GraphicsRectangularBranchItem(qreal x, qreal y, const QString& name, qreal d);
    GraphicsRectangularBranchItem(qreal x, qreal y, const QString& name);

    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setParentItem(QGraphicsItem *item);

    Direction getDirection() const { return direction; }
    qreal getHeight() const { return height; }
    void setHeightW(qreal h) { height = h; }
    void setHeight(qreal h);
    void setHeightCoef(int coef);
    void setHeightCoefW(int coef) {cur_height_coef = coef;}
    void setDirection(Direction d);

    void collapse();
    void swapSiblings();
    void redrawBranches(int& current, qreal& minDistance, qreal& maxDistance, PhyNode* root);

    void setPhyBranch(PhyBranch* p) {phyBranch = p;}
    const PhyBranch* getPhyBranch() const {return phyBranch;}
private:
    qreal height;
    int cur_height_coef;
    Direction direction;
    PhyBranch* phyBranch;

    GraphicsRectangularBranchItem* getChildItemByPhyBranch(const PhyBranch* branch);
};

}//namespace;

#endif
