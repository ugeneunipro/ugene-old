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

private:
    GraphicsBranchItem* correspondingItem;
    GraphicsButtonItem* buttonItem;
    void initText(qreal d);

protected:

    QGraphicsSimpleTextItem* distanceText;
    QGraphicsTextItem*       nameText;
    qreal width;
    qreal dist;
    bool collapsed;
    BranchSettings settings;

    GraphicsBranchItem(const QString& name);
    GraphicsBranchItem(qreal d, bool withButton = true);

public:
    GraphicsBranchItem(bool withButton = true);

    GraphicsButtonItem* getButton() const { return buttonItem; }
    QGraphicsSimpleTextItem* getDistanceText() const { return distanceText; }
    QGraphicsTextItem* getNameText() const { return nameText; }
    qreal getWidth() const { return width; }
    qreal getDist() const { return dist; }
    void setDistanceText(const QString& text);
    void setWidthW(qreal w) { width = w; }
    void setWidth(qreal w);
    void setDist (qreal d) { dist = d; }
    virtual void collapse();
    void setSelectedRecurs(bool sel, bool recursively);
    bool isCollapsed();

    void updateSettings(const BranchSettings& branchSettings);
    void updateChildSettings(const BranchSettings& branchSettings);
    void updateTextSettings(const QFont& font, const QColor& color);

    const BranchSettings& getBranchSettings() {return settings;}

    GraphicsBranchItem* getCorrespondingItem() {return correspondingItem;}
    void setCorrespondingItem(GraphicsBranchItem* cItem) {correspondingItem = cItem;}

    const QList<QGraphicsItem*> getChildItems() {return childItems();}
};

}//namespace;

#endif
