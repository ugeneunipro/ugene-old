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
