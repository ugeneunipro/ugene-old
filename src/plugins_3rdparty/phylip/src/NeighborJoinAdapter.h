#ifndef _U2_NEIGHBORJOIN_ADAPTER_H_
#define _U2_NEIGHBORJOIN_ADAPTER_H_

#include <QtCore/QObject>

#include <U2Core/PhyTree.h>
#include <U2Algorithm/SubstMatrixRegistry.h>
#include <U2View/CreatePhyTreeDialogController.h>
#include <U2Algorithm/PhyTreeGenerator.h>

namespace U2 { 

	class MAlignment;
	class TaskStateInfo;

	class NeighborJoinAdapter : public PhyTreeGenerator {
	public:
        PhyTree calculatePhyTree(const MAlignment& ma, const CreatePhyTreeSettings& s, TaskStateInfo& ti);
        virtual void setupCreatePhyTreeUI(CreatePhyTreeDialogController* c, const MAlignment& ma);
    private:
        static QMutex runLock;
	};

}//namespace

#endif