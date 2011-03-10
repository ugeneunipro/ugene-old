#ifndef _U2_CREATE_UNROOTED_BRANCHES_TASK_H_
#define _U2_CREATE_UNROOTED_BRANCHES_TASK_H_

#include "CreateBranchesTask.h"

namespace U2 {

class PhyNode;
class GraphicsRectangularBranchItem;
class GraphicsUnrootedBranchItem;

class CreateUnrootedBranchesTask: public CreateBranchesTask {
    Q_OBJECT

    qreal coef;
    GraphicsRectangularBranchItem* root1;
    GraphicsUnrootedBranchItem* getBranch(GraphicsRectangularBranchItem *r, GraphicsUnrootedBranchItem* parent);

public:
    CreateUnrootedBranchesTask(GraphicsRectangularBranchItem *r);
    void run();
};

}//namespace;

#endif
