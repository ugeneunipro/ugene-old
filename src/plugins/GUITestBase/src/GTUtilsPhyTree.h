/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2015 UniPro <ugene@unipro.ru>
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

#ifndef _U2_GT_UTILS_PHY_TREE_H_
#define _U2_GT_UTILS_PHY_TREE_H_

#include <U2View/TreeViewer.h>

#include "GTGlobals.h"

namespace U2 {

class GraphicsButtonItem;

class GTUtilsPhyTree {
public:
    static QList<QGraphicsItem *> getSelectedNodes(U2OpStatus& os);
    static QList<QGraphicsItem *> getUnselectedNodes(U2OpStatus& os);
    static QList<QGraphicsItem *> getNodes(U2OpStatus& os);

    static QList<QGraphicsSimpleTextItem*> getLabels(U2OpStatus& os, QGraphicsView* treeView = NULL);
    static QList<QGraphicsSimpleTextItem*> getVisiableLabels(U2OpStatus& os, QGraphicsView* treeView = NULL);
    static QList<QGraphicsSimpleTextItem*> getDistances(U2OpStatus& os, QGraphicsView* treeView = NULL);
    static QList<QGraphicsSimpleTextItem*> getVisiableDistances(U2OpStatus& os, QGraphicsView* treeView = NULL);
    static QStringList getLabelsText(U2OpStatus& os);
    static QList<double> getDistancesValues(U2OpStatus& os);
    static QPoint getGlobalCoord(U2OpStatus& os, QGraphicsItem* item);

    static void clickNode(U2OpStatus &os, GraphicsButtonItem *node);
    static qreal getNodeDistance(U2OpStatus &os, GraphicsButtonItem *node);

    static TreeViewerUI * getTreeViewerUi(U2OpStatus &os);

    /**
     * Branches are enumerated:
     * - right subtree is enumerated
     * - left subtree is enumerated
     * - root branch is enumerated
     *
     * An example:
     *        |-0--
     *    |-4--
     *    |   |   |-1--
     * -11-   |-3--
     *    |       |-2--
     *    |
     *    |       |-5--
     *    |   |-7--
     *    |   |   |-6--
     *    |-9--
     *    |   |-8--
     *    |
     *    |-10-
     *
     */
    static QList<GraphicsButtonItem *> getOrderedRectangularNodes(U2OpStatus &os);
    static QList<GraphicsRectangularBranchItem *> getOrderedRectangularBranches(U2OpStatus &os);
    static QList<qreal> getOrderedRectangularBranchesDistances(U2OpStatus &os);

    static GraphicsButtonItem * getRootRectangularNode(U2OpStatus &os);
    static GraphicsRectangularBranchItem * getRootRectangularBranch(U2OpStatus &os);

private:
    static QList<GraphicsRectangularBranchItem *> getSubtreeOrderedRectangularBranches(U2OpStatus &os, GraphicsRectangularBranchItem *rootBranch);
    static bool rectangularBranchLessThan(GraphicsRectangularBranchItem *first, GraphicsRectangularBranchItem *second);
    static QList<QGraphicsItem *> getNodes(U2OpStatus& os, int width);
};

}   // namespace U2

#endif // _U2_GT_UTILS_PHY_TREE_H_
