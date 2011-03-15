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

#ifndef _U2_WORKFLOW_VIEW_ITEMS_H_
#define _U2_WORKFLOW_VIEW_ITEMS_H_

#include <U2Lang/ActorModel.h>

#include <QtGui/QAction>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsScene>
#include <QtGui/QGraphicsItem>

class QDomElement;

namespace U2 {
using namespace Workflow;
class WorkflowScene;
class ItemViewStyle;

typedef QString StyleId;

#define GRID_STEP 15
#define ANGLE_STEP 10

inline qreal round(qreal val, int step) {
    int tmp = int(val) + step /2;
    tmp -= tmp % step;
    return qreal(tmp);
}
inline qreal roundUp(qreal val, int step) {
    int tmp = int(val) + step /2;
    tmp -= tmp % step;
    if (tmp < int(val)) {
        tmp += step;
    }
    return qreal(tmp);
}


class ItemStyles {
public:
    static const StyleId SIMPLE;
    static const StyleId EXTENDED;
};

class StyledItem : public QGraphicsItem {
public:
    StyledItem(QGraphicsItem* p = 0) : QGraphicsItem(p) {}
    WorkflowScene* getWorkflowScene() const;
    virtual ~StyledItem(){}
    virtual void setStyle(StyleId) {}
    virtual StyleId getStyle() const {return ItemStyles::SIMPLE;}
    virtual QList<QAction*> getContextMenuActions() const {return QList<QAction*>();}
};

class WorkflowPortItem;
class WorkflowBusItem;
enum {
    WorkflowProcessItemType = QGraphicsItem::UserType + 1, 
    WorkflowPortItemType,
    WorkflowBusItemType
};

class WorkflowProcessItem : public QObject, public StyledItem {
    Q_OBJECT
public:
    WorkflowProcessItem(Actor* process);
    virtual ~WorkflowProcessItem();
    Actor* getProcess() const {return process;}
    WorkflowPortItem* getPort(const QString& id) const;
    QList<WorkflowPortItem*> getPortItems() const {return ports;}
    QRectF boundingRect(void) const;
    QPainterPath shape () const;

    virtual void setStyle(StyleId);
    virtual StyleId getStyle() const {return styles.key(currentStyle);}

    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    enum {Type = WorkflowProcessItemType};
    int type() const {return Type;}
    void prepareUpdate() {prepareGeometryChange();}

    virtual QList<QAction*> getContextMenuActions() const;

    void saveState(QDomElement& ) const;
    void loadState(QDomElement& );
    
    ItemViewStyle* getStyleByIdSafe(StyleId id) const;
    ItemViewStyle* getStyleById(const StyleId & id) const;
    bool containsStyle(const StyleId & id) const;
    
protected:
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
    virtual bool sceneEvent ( QEvent * event );
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
public slots:
    void sl_update();
private:
    void createPorts();
    QMap<StyleId, ItemViewStyle*> styles;
    ItemViewStyle* currentStyle;
    Actor* process;
    QList<WorkflowPortItem*> ports;
    QMap<QGraphicsItem *, QPointF> initialPositions;
};

class WorkflowPortItem : public StyledItem {
public:
    WorkflowPortItem(WorkflowProcessItem* owner, Port* port);
    virtual ~WorkflowPortItem();
    Port* getPort() const {return port;}
    WorkflowProcessItem* getOwner() const {return owner;}
    QList<WorkflowBusItem*> getDataFlows() const {return flows;}
    WorkflowBusItem* getDataFlow(const WorkflowPortItem* other) const;
    WorkflowPortItem* checkBindCandidate(const QGraphicsItem* it) const;
    WorkflowPortItem* findNearbyBindingCandidate(const QPointF& at) const;
    WorkflowBusItem* tryBind(WorkflowPortItem* otherPort);
    void removeDataFlow(WorkflowBusItem* flow);
    // position of the arrow tip in items coordinates 
    QPointF head(const QGraphicsItem* item) const;
    // position of the arrow tip in scene coordinates
    QPointF headToScene() const;
    // direction of the arrow in items coordinates 
    QLineF arrow(const QGraphicsItem* item) const;
    QRectF boundingRect(void) const;
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );

    qreal getOrientarion() const {return orientation;}
    void setOrientation(qreal);

    virtual void setStyle(StyleId);
    virtual StyleId getStyle() const {return currentStyle;}
    void adaptOwnerShape();


    void setHighlight(bool v) {highlight = v;}

    enum {Type = WorkflowPortItemType};
    int type() const {return Type;}
protected:
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
private:
    StyleId currentStyle;

    Port* port;
    WorkflowProcessItem* owner;
    qreal orientation;
    QList<WorkflowBusItem*> flows;
    bool dragging;
    bool rotating;
    bool sticky;
    bool highlight;
    QList<WorkflowPortItem*> bindCandidates;
    QPointF dragPoint;
};

class WorkflowBusItem : public QObject, public StyledItem {
    Q_OBJECT
    friend class WorkflowPortItem;
    friend class WorkflowProcessItem;
public:
    WorkflowBusItem(WorkflowPortItem* p1, WorkflowPortItem* p2);
    virtual ~WorkflowBusItem();
    WorkflowPortItem *getInPort() const {return dst;}
    WorkflowPortItem *getOutPort() const {return src;}
    Link* getBus() const {return bus;}
    QRectF boundingRect(void) const;
    QPainterPath shape () const;
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    enum {Type = WorkflowBusItemType};
    int type() const {return Type;}
    bool validate();

    void saveState(QDomElement& ) const;
    void loadState(QDomElement& );
    
    QGraphicsItem* getText() const {return text;}
    
protected:
    QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
    //void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
    //void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    //void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
    void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
    void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
private slots:
    void sl_update();

private:
    void updatePos();
    Link* bus;
    WorkflowPortItem *dst, *src;
    QGraphicsItem* text;
    //bool dragging;
    //QPointF dragPoint;
};

} // U2

Q_DECLARE_METATYPE(U2::StyleId);

#endif
