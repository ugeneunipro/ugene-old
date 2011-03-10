#ifndef _U2_CREATE_BRANCHES_TASK_H_
#define _U2_CREATE_BRANCHES_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class GraphicsBranchItem;

class CreateBranchesTask: public Task {
    Q_OBJECT
protected:
    GraphicsBranchItem* root;
public:
    CreateBranchesTask(): Task(QObject::tr("Generating tree view"), TaskFlag_None), root(NULL) {}
    GraphicsBranchItem* getResult() { return root; }
};

}//namespace;

#endif
