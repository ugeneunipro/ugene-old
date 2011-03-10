#ifndef _U2_GRAPHICS_BUTTON_ITEM_H_
#define _U2_GRAPHICS_BUTTON_ITEM_H_

#include <QtGui/QAbstractGraphicsShapeItem>

namespace U2 {

class GraphicsButtonItem: public QAbstractGraphicsShapeItem {
    static const qreal radiusMin, radiusMax;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *e);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e);
    
public:
    GraphicsButtonItem();
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    bool isSelectedTop();
    void collapse();
    void swapSiblings();
    bool isCollapsed();
};

}//namespace;

#endif
