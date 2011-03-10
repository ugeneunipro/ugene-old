#ifndef _U2_CREATE_RECTANGULAR_BRANCHES_TASK_H_
#define _U2_CREATE_RECTANGULAR_BRANCHES_TASK_H_

#include "CreateBranchesTask.h"

namespace U2 {

class PhyNode;
class GraphicsRectangularBranchItem;

class CreateRectangularBranchesTask: public CreateBranchesTask {
    Q_OBJECT

    int size;
    int current;
    qreal scale;
    PhyNode* node;
    qreal minDistance, maxDistance;
    GraphicsRectangularBranchItem* getBranch(PhyNode *node);

public:
    CreateRectangularBranchesTask(PhyNode *n);
    void run();
    qreal getScale() { return scale; }
};

}//namespace;

#endif
