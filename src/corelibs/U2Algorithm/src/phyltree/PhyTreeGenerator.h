#ifndef _PHYTREEGENERATOR
#define _PHYTREEGENERATOR


#include <U2Core/Task.h>
#include <U2Core/PhyTree.h>
#include <U2Core/MAlignment.h>
#include "CreatePhyTreeSettings.h"

#include <memory>



namespace U2{

class CreatePhyTreeDialogController;

class U2ALGORITHM_EXPORT PhyTreeGenerator{
public:
    virtual PhyTree calculatePhyTree(const MAlignment& ma, const CreatePhyTreeSettings& s, TaskStateInfo& ti) = 0;
    // TODO create separate class&registry
    virtual void setupCreatePhyTreeUI(CreatePhyTreeDialogController* c, const MAlignment& ma) = 0;
};



}
#endif
