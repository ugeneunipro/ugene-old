#include <QtCore/QStack>

#include "CreateUnrootedBranchesTask.h"
#include "GraphicsUnrootedBranchItem.h"
#include "GraphicsRectangularBranchItem.h"
#include <U2Core/PhyTreeObject.h>
#include "TreeViewerUtils.h"

namespace U2 {

CreateUnrootedBranchesTask::CreateUnrootedBranchesTask(GraphicsRectangularBranchItem *r): root1(r) {}

GraphicsUnrootedBranchItem* CreateUnrootedBranchesTask::getBranch(GraphicsRectangularBranchItem *from, GraphicsUnrootedBranchItem* parent) {
    GraphicsUnrootedBranchItem* res = new GraphicsUnrootedBranchItem(parent, coef * from->getHeight(), from);
    foreach (QGraphicsItem* item, from->childItems()) {
        GraphicsRectangularBranchItem* ri = dynamic_cast<GraphicsRectangularBranchItem*>(item);
        if (ri != NULL) {
            getBranch(ri, res);
        }
    }
    return res;
}

void CreateUnrootedBranchesTask::run() {
    coef = 360.0 / root1->childrenBoundingRect().height();
    root = getBranch(root1, NULL);
}

}
