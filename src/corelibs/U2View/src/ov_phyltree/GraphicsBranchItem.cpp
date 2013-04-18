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

#include "GraphicsBranchItem.h"
#include "GraphicsButtonItem.h"
#include "TreeViewerUtils.h"

#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsScene>
#include <QtCore/QStack>
#include <U2Core/U2SafePoints.h>

namespace U2 {

QColor BranchSettings::defaultColor = QColor(0, 0, 0);
int BranchSettings::defaultThickness = 1;

BranchSettings::BranchSettings() {

    branchColor = defaultColor;
    branchThickness = defaultThickness;
}

const int GraphicsBranchItem::TextSpace = 8;
const int GraphicsBranchItem::SelectedPenWidth = 1;

void GraphicsBranchItem::updateSettings(const BranchSettings& branchSettings) {

    settings = branchSettings;

    int penWidth = settings.branchThickness;
    if (isSelected()) {
        penWidth += SelectedPenWidth;
    }

    QPen currentPen = this->pen();
    currentPen.setColor(settings.branchColor);
    currentPen.setWidth(penWidth);

    this->setPen(currentPen);
}

void GraphicsBranchItem::updateChildSettings(const BranchSettings& branchSettings) {
    foreach(QGraphicsItem* graphItem, this->childItems()) {
        GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);
        if (branchItem) {
            branchItem->updateSettings(branchSettings);
            branchItem->updateChildSettings(branchSettings);
        }
    }
}

void GraphicsBranchItem::updateTextSettings(const QFont& font, const QColor& color){
    if(distanceText){
        distanceText->setFont(font);
        distanceText->setBrush(color);
    }
    if(nameText){
        nameText->setFont(font);
        nameText->setBrush(color);
    }
}

void GraphicsBranchItem::collapse() {
    collapsed = !collapsed;
    QList<QGraphicsItem*> items = childItems();
    if (collapsed) {
        for (int i = 0, s = items.size(); i < s; ++i) {
            if (dynamic_cast<GraphicsBranchItem*>(items[i])) {
                items[i]->hide();
            }
        }

        int penWidth = settings.branchThickness;
        if (isSelected()) {
            penWidth += SelectedPenWidth;
        }

        QPen pen1(settings.branchColor);
        pen1.setWidth(penWidth);
        pen1.setCosmetic(true);
        QGraphicsRectItem *r = new QGraphicsRectItem(0, -4, 16, 8, this);
        r->setPen(pen1);
    } else {
        for (int i = 0, s = items.size(); i < s; ++i) {
            if (dynamic_cast<QGraphicsRectItem*>(items[i])) {
                items[i]->setParentItem(NULL);
                scene()->removeItem(items[i]);
            } else {
                if (items[i] != getDistanceText() && items[i] != getNameText()) {
                    items[i]->show();
                }
            }
        }
        setSelectedRecurs(true,true);
    }
}

void GraphicsBranchItem::setSelectedRecurs(bool sel, bool recursively) {

    int penWidth = settings.branchThickness;
    if (sel) {
        penWidth = settings.branchThickness + SelectedPenWidth;
    }

    QPen thisPen = this->pen();
    thisPen.setWidth(penWidth);
    setPen(thisPen);

    if (buttonItem) {
        buttonItem->setSelected(sel);
    }

    if (recursively) {
        foreach(QGraphicsItem* graphItem, this->childItems()) {
            GraphicsBranchItem *branchItem = dynamic_cast<GraphicsBranchItem*>(graphItem);

            if (branchItem) {
                 branchItem->setSelectedRecurs(sel, recursively);
            }
            
        }
    }
    this->setSelected(sel);
    scene()->update();
}

void GraphicsBranchItem::initText(qreal d) {
    QString str = QString::number(d, 'f', 3);
    int i = str.length() - 1;
    for (; i >= 0 && str[i] == '0'; --i) ;
    if (str[i] == '.')
        --i;
    str.truncate(i + 1);

    //it doesn't show zeroes by default. only in cladogramm mode
    if(str == "0"){
        str = "";
    }
    //test
    distanceText = new QGraphicsSimpleTextItem(str);
    distanceText->setFont(TreeViewerUtils::getFont());
    distanceText->setBrush(Qt::darkGray);
    QRectF rect = distanceText->boundingRect();
    distanceText->setPos(-rect.width(), 0);
    distanceText->setParentItem(this);
    distanceText->setZValue(1);
}

GraphicsBranchItem::GraphicsBranchItem(bool withButton)
: correspondingItem(NULL), buttonItem(NULL), distanceText(NULL), nameText(NULL), collapsed(false), branchLength(0), lengthCoef(1), nameItemSelection(NULL) {

    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::NoButton);

    if (withButton) {
        buttonItem = new GraphicsButtonItem();
        buttonItem->setParentItem(this);
    }

    setBrush(settings.branchColor);
    QPen pen1(settings.branchColor);
    pen1.setCosmetic(true);
    setPen(pen1);
}

GraphicsBranchItem::GraphicsBranchItem(const QString& name)
: correspondingItem(NULL), buttonItem(NULL), distanceText(NULL), collapsed(false), branchLength(0), lengthCoef(1), nameItemSelection(NULL) {

    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::NoButton);

    QPen pen1(settings.branchColor);
    pen1.setStyle(Qt::DotLine);
    pen1.setCosmetic(true);
    setPen(pen1);

    width = 0;
    dist = 0;

    nameText = new QGraphicsSimpleTextItem(name);
    nameText->setFont(TreeViewerUtils::getFont());
    nameText->setBrush(Qt::darkGray);
    QRectF rect = nameText->boundingRect();
    nameText->setPos(GraphicsBranchItem::TextSpace, -rect.height() / 2);
    nameText->setParentItem(this);
    nameText->setZValue(1);
}

GraphicsBranchItem::GraphicsBranchItem(qreal d, bool withButton)
: correspondingItem(NULL), buttonItem(NULL), distanceText(NULL), nameText(NULL), collapsed(false), branchLength(0), lengthCoef(1), nameItemSelection(NULL) {

    setFlag(QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(false);
    setAcceptedMouseButtons(Qt::NoButton);

    if (withButton) {
        buttonItem = new GraphicsButtonItem();
        buttonItem->setParentItem(this);
    }

    initText(d);
    QPen pen1(settings.branchColor);
    pen1.setCosmetic(true);
    if (d < 0) {
        pen1.setStyle(Qt::DashLine);
    }
    setPen(pen1);
    setBrush(settings.branchColor);
}

void GraphicsBranchItem::setDistanceText(const QString& text){
    if(distanceText){
        distanceText->setText(text);
    }
}

void GraphicsBranchItem::setWidth(qreal w) {
    if (width == w) {
        return;
    }

    setPos(pos().x() - width + w, pos().y());
    if (getDistanceText() != NULL) {
        QPointF pos = getDistanceText()->pos();
        getDistanceText()->setPos(pos.x() + (width - w) * 0.5, pos.y());
    }

    prepareGeometryChange();
    width = w;
}

bool GraphicsBranchItem::isCollapsed(){
    return collapsed;
}

void GraphicsBranchItem::paint(QPainter* painter,const QStyleOptionGraphicsItem*, QWidget*) {
    CHECK(NULL != nameText,);
    if (isSelected()) {
            qreal radius = settings.branchThickness + 1.5;
            QRectF rect(-radius, -radius, radius*2, radius*2);
            painter->setBrush(settings.branchColor);
            if(NULL == nameItemSelection) {
                nameItemSelection = scene()->addEllipse(rect, QPen(settings.branchColor), QBrush(settings.branchColor));
                nameItemSelection->setParentItem(this);
                nameItemSelection->setFlag(QGraphicsItem::ItemIgnoresTransformations);
                nameItemSelection->setPen(QPen(Qt::gray));
                nameItemSelection->setBrush(QBrush(settings.branchColor));
            }
            else if (!nameItemSelection->isVisible()) {
                nameItemSelection->setRect(rect);
                nameItemSelection->show();
            }
    }
    else {
        if(NULL != nameItemSelection) {
        nameItemSelection->hide();
        }
    }
}
} //namespace
