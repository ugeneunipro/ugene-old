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
    setWidthW(w);
    setDist(from->getDist());
    setPos(w, 0);
    angle = from->getDirection() == GraphicsBranchItem::up ? angle : -angle;
    setTransform(QTransform().translate(-w, 0).rotate(angle).translate(w, 0));
//    setTransformOriginPoint(-w, 0);
//    setRotation(angle);

    QPen pen1;
    pen1.setCosmetic(true);
    if (from->getNameText() != NULL) {
        nameText = new QGraphicsSimpleTextItem(from->getNameText()->text(), this);
        QRectF rect = nameText->boundingRect();
        qreal h = rect.height();
        nameText->setPos(GraphicsBranchItem::TextSpace, -h * 0.5);
        if (nameText->scenePos().x() < 0.0) {
            QPointF p = rect.center();
            nameText->setTransform(QTransform().translate(p.x(), p.y()).rotate(180).translate(-p.x(), -p.y()));
//            nameText->setTransformOriginPoint(rect.center());
//            nameText->setRotation(180);
        }

        pen1.setStyle(Qt::DotLine);
        pen1.setColor(Qt::darkGray);
        QBrush brush1 = nameText->brush();
        brush1.setColor(Qt::darkGray);
        nameText->setBrush(brush1);
    }
    if (from->getDistanceText() != NULL) {
        distanceText = new QGraphicsSimpleTextItem(from->getDistanceText()->text(), this);
        QRectF rect = distanceText->boundingRect();
        if (distanceText->scenePos().x() < 0) {
            QPointF p(rect.center().x(), rect.height());
            distanceText->setTransform(QTransform().translate(p.x(), p.y()).rotate(180).translate(-p.x(), -p.y()));
//            distanceText->setTransformOriginPoint(rect.center().x(), rect.height());
//            distanceText->setRotation(180);
        }
        distanceText->setPos(-0.5 * (w + rect.width()), -rect.height());

        QBrush brush1 = distanceText->brush();
        brush1.setColor(Qt::darkGray);
        distanceText->setBrush(brush1);
    }
    setPen(pen1);
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
