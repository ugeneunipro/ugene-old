#ifndef _U2_GRAPHICS_BRANCH_ITEM_H_
#define _U2_GRAPHICS_BRANCH_ITEM_H_

#include <QtGui/QAbstractGraphicsShapeItem>

namespace U2 {

class PhyNode;
class GraphicsButtonItem;

class BranchSettings {
public:
    BranchSettings();

    QColor branchColor;
    int branchThickness;

    static QColor defaultColor;
    static int defaultThickness;
};

class GraphicsBranchItem: public QAbstractGraphicsShapeItem {
public:
    enum Direction { up, down };

    static const int TextSpace;
    static const int SelectedPenWidth;

    BranchSettings settings;

private:
    GraphicsButtonItem* buttonItem;
    void initText(qreal d);

protected:

    QGraphicsSimpleTextItem* distanceText;
    QGraphicsSimpleTextItem* nameText;
    qreal width;
    qreal dist;
    bool collapsed;

    GraphicsBranchItem(const QString& name);
    GraphicsBranchItem(qreal d, bool withButton = true);


public:
    GraphicsBranchItem(bool withButton = true);

    GraphicsButtonItem* getButton() const { return buttonItem; }
    QGraphicsSimpleTextItem* getDistanceText() const { return distanceText; }
    QGraphicsSimpleTextItem* getNameText() const { return nameText; }
    qreal getWidth() const { return width; }
    qreal getDist() const { return dist; }
    void setWidthW(qreal w) { width = w; }
    void setWidth(qreal w);
    void setDist (qreal d) { dist = d; }
    virtual void collapse();
    void setSelectedRecurs(bool sel, bool recursively);
    bool isCollapsed();

    void updateSettings(const BranchSettings& branchSettings);
    void updateTextSettings(const QFont& font, const QColor& color);
};

}//namespace;

#endif
