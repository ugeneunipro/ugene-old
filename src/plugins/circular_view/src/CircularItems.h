#ifndef _CIRCULAR_ITEMS_H_
#define _CIRCULAR_ITEMS_H_

#include <QtGui/QGraphicsPathItem>
#include <QtGui/QFont>

namespace U2 {

class Annotation;
class AnnotationSettings;
class CircularAnnotationItem;
class CircularViewRenderArea;
class CircularAnnotationLabel;
class CircurlarAnnotationRegionItem;

/************************************************************************/
/* CircularAnnotationItem                                               */
/************************************************************************/

class CircularAnnotationItem : public QGraphicsItem {
friend class CircurlarAnnotationRegionItem;
friend class CircularAnnotationLabel;
public:
    CircularAnnotationItem(Annotation* ann, CircurlarAnnotationRegionItem* region, CircularViewRenderArea* _ra);
    CircularAnnotationItem(Annotation* ann, QList<CircurlarAnnotationRegionItem*>& regions, CircularViewRenderArea* _ra);
    ~CircularAnnotationItem();
    virtual void paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget);
    Annotation* getAnnotation() const;
    virtual QRectF boundingRect() const;
    virtual bool contains(const QPointF &point);
    int containsRegion(const QPointF &point);
    void setSelected(bool isSelected);
    const QList<CircurlarAnnotationRegionItem*>& getRegions();
protected:
    bool isSelected;
    QColor color;
    Annotation *annotation;
    QList<CircurlarAnnotationRegionItem*> regions;
    QRectF _boundingRect;
    CircularViewRenderArea* ra;
};

/************************************************************************/
/* CircurlarAnnotationRegionItem                                        */
/************************************************************************/
class CircurlarAnnotationRegionItem : public QGraphicsPathItem{
friend class CircularAnnotationItem;
public:
    CircurlarAnnotationRegionItem(const QPainterPath& path, bool isShort, int number );
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */);
    CircularAnnotationLabel* getLabel() const;
    void setLabel(CircularAnnotationLabel* label);
protected:
    CircularAnnotationItem *parent;
    CircularAnnotationLabel *label;
    int number;
    bool isShort;
};

/************************************************************************/
/* CircularAnnotationLabel                                              */
/************************************************************************/
class CircularAnnotationLabel : public QGraphicsItem {
public:
    CircularAnnotationLabel(Annotation* ann, int _region, int sequenceLength, const QFont& font, CircularViewRenderArea* renderArea );
    virtual void paint(QPainter *p,const QStyleOptionGraphicsItem *item,QWidget *widget);
    Annotation* getAnnotation() const;
    virtual QRectF boundingRect() const;
    virtual bool contains(const QPointF &point);
    int getRegion() const;
    void setLabelPosition();
    static void prepareLabels(QList<CircularAnnotationLabel*>& labelItems);

    bool operator<(const CircularAnnotationLabel& a);
private:
    int findClosestPoint(const QPoint& targetPoint, const QVector<QRect>& rects, QVector<int> indexes);
    void getVacantPositions(const QVector<QRect>& rects, QVector<int>& result);
    void getVacantInnerPositions(const QVector<QRect>& rects, QVector<int>& result);
    void getConnectionPoint();

    Annotation* annotation;
    float annotationAngle;
    QFont labelFont;
    QRectF midRect;
    QPointF midRegionPoint;
    int region;

    qreal startA;
    qreal endA;
    qreal spanA;
    QPoint labelPos;
    QPoint connectionPoint;
    CircularViewRenderArea* ra;
    bool hasPosition;
    QString labelText;
    bool inner;
    int seqLen;
};

} // namespace U2

#endif // _CIRCULAR_ITEMS_H_
