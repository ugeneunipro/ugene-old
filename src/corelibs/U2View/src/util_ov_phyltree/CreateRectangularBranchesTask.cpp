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

#include <QtCore/QStack>

#include "CreateRectangularBranchesTask.h"
#include "GraphicsRectangularBranchItem.h"
#include <U2Core/PhyTreeObject.h>
#include "TreeViewerUtils.h"

namespace U2 {

CreateRectangularBranchesTask::CreateRectangularBranchesTask(PhyNode *n): size(0), current(0), node(n) {}

GraphicsRectangularBranchItem* CreateRectangularBranchesTask::getBranch(PhyNode *node) {
    if (isCanceled() || stateInfo.hasError())
        return NULL;


    int branches = node->branches.size();
    if (branches == 1 && (node->name=="" || node->name=="ROOT")) {
        assert(node != node->branches[0]->node2);
        return getBranch(node->branches[0]->node2);
    }
    if (branches > 1) {
        stateInfo.progress = 100 * ++size / 100; // <- number of sequences
        QList<GraphicsRectangularBranchItem*> items;
        int ind = -1;
        for (int i = 0; i < branches; ++i) {
            if (node->branches[i]->node2 != node) {
                GraphicsRectangularBranchItem *item = getBranch(node->branches[i]->node2);
                items.append(item);
            } else {
                items.append(NULL);
                ind = i;
            }
        }

        GraphicsRectangularBranchItem *item;
        if (ind<0) {
            item = new GraphicsRectangularBranchItem();
        }
        else {
            item = new GraphicsRectangularBranchItem(node->branches[ind]->distance);
            item->setPhyBranch(node->branches[ind]);
        }
        int size = items.size();
        assert(size > 0);

        {
            int xmin = 0, ymin = items[0] ? items[0]->pos().y() : items[1]->pos().y(), ymax = 0;
            for (int i = 0; i < size; ++i) {
                if (items[i] == NULL) {
                    continue;
                }
                QPointF pos1 = items[i]->pos();
                if (pos1.x() < xmin)
                    xmin = pos1.x();
                if (pos1.y() < ymin)
                    ymin = pos1.y();
                if (pos1.y() > ymax)
                    ymax = pos1.y();
            }
            xmin -= GraphicsRectangularBranchItem::DEFAULT_WIDTH;

            int y = (ymax + ymin) / 2;
            item->setPos(xmin, y);

            for (int i = 0; i < size; ++i) {
                if (items[i] == NULL) {
                    continue;
                }
                qreal dist = qAbs(node->branches[i]->distance);
                if (minDistance > -1) {
                    minDistance = qMin(minDistance, dist);
                } else {
                    minDistance = dist;
                }
                maxDistance = qMax(maxDistance, dist);
                items[i]->setDirection(items[i]->pos().y() > y ? GraphicsRectangularBranchItem::up : GraphicsRectangularBranchItem::down);
                items[i]->setWidthW(dist);
                items[i]->setDist(dist);
                items[i]->setParentItem(item);
                QRectF rect = items[i]->getDistanceText()->boundingRect();
                items[i]->getDistanceText()->setPos(-(items[i]->getWidth() + rect.width()) / 2, 0);
            }
        }
        return item;
    } else {
        int y = (current++ + 0.5) * GraphicsRectangularBranchItem::DEFAULT_HEIGHT;
        GraphicsRectangularBranchItem *item = NULL;
        if(branches != 1){
            item = new GraphicsRectangularBranchItem(0, y, node->name);
        }else{
            item = new GraphicsRectangularBranchItem(0, y, node->name, node->branches[0]->distance);
            item->setPhyBranch(node->branches[0]);
        }
        
        return item;
    }
}

void CreateRectangularBranchesTask::run() {
    minDistance = -2;
    maxDistance = 0;
    GraphicsRectangularBranchItem* item = getBranch(node); // modifies minDistance and maxDistance
    item->setWidthW(0);
    item->setDist(0);
    item->setHeightW(0);
    root = item;

    qreal minDistScale = GraphicsRectangularBranchItem::DEFAULT_WIDTH / (qreal)minDistance;
    qreal maxDistScale = GraphicsRectangularBranchItem::MAXIMUM_WIDTH / (qreal)maxDistance;

    scale = qMin(minDistScale, maxDistScale);

    QStack<GraphicsRectangularBranchItem*> stack;
    stack.push(item);
    while (!stack.empty()) {
        GraphicsRectangularBranchItem *item = stack.pop();
        item->setWidth(item->getWidth() * scale);
        foreach (QGraphicsItem* ci, item->childItems()) {
            GraphicsRectangularBranchItem* gbi = dynamic_cast<GraphicsRectangularBranchItem*>(ci);
            if (gbi != NULL) {
                stack.push(gbi);
            }
        }
    }
}

}
