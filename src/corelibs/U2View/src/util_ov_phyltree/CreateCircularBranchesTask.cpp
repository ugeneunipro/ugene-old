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

#include "CreateCircularBranchesTask.h"
#include "GraphicsCircularBranchItem.h"
#include "GraphicsRectangularBranchItem.h"
#include <U2Core/PhyTreeObject.h>
#include "TreeViewerUtils.h"

namespace U2 {

CreateCircularBranchesTask::CreateCircularBranchesTask(GraphicsRectangularBranchItem *r): root1(r) {}

GraphicsCircularBranchItem* CreateCircularBranchesTask::getBranch(GraphicsRectangularBranchItem *from, GraphicsCircularBranchItem* parent) {
    GraphicsCircularBranchItem* res = new GraphicsCircularBranchItem(parent, coef * from->getHeight(), from);
    foreach (QGraphicsItem* item, from->childItems()) {
        GraphicsRectangularBranchItem* ri = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if (ri != NULL) {
            getBranch(ri, res);
        }
    }
    res->setCorrespondingItem(from);
    return res;
}

void CreateCircularBranchesTask::run() {
    coef = 6.0 / root1->childrenBoundingRect().height();
    root1->setWidthW(30);
    GraphicsCircularBranchItem* r = getBranch(root1, NULL);
    r->setVisibleW(false);
    root = r;
    root1->setWidthW(0);
}

}
