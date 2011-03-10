#ifndef _U2_CREATE_CIRCULAR_BRANCHES_TASK_H_
#define _U2_CREATE_CIRCULAR_BRANCHES_TASK_H_

#include "CreateBranchesTask.h"

namespace U2 {

class PhyNode;
class GraphicsRectangularBranchItem;
class GraphicsCircularBranchItem;

class CreateCircularBranchesTask: public CreateBranchesTask {
    Q_OBJECT

    qreal coef;
    GraphicsRectangularBranchItem* root1;
    GraphicsCircularBranchItem* getBranch(GraphicsRectangularBranchItem *r, GraphicsCircularBranchItem* parent);

public:
    CreateCircularBranchesTask(GraphicsRectangularBranchItem *r);
    void run();
};

}//namespace;

#endif
