#include "NeighborJoinAdapter.h"
#include "DistMatrixModelWidget.h"
#include "SeqBootModelWidget.h"
#include "SeqBootAdapter.h"

#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>

#include <QtGui/QPushButton>
#include <QtCore/QVector>
#include <QtCore/QString>

//#include <algorithm>
//#include <exception>
//#include <iostream>
//#include <memory>

#include "DistanceMatrix.h"
#include "neighbor.h"
#include "dnadist.h"
#include "protdist.h"


namespace U2 {

QMutex NeighborJoinAdapter::runLock;

void createPhyTreeFromPhylipTree(const MAlignment &ma, node *p, double m, boolean njoin, node *start, PhyNode* root)
{
    /* used in fitch & neighbor */
    long i=0;
    naym* nayme = getNayme();
    static int counter = 0;

    PhyNode* current = NULL;

    if (p == start) {
        current = root;
    } else {
        current = new PhyNode;
    }

    if (p->tip) {
        assert(p->index - 1 < ma.getNumRows());
        current->name = ma.getRow(p->index - 1).getName();
    } else {
        current->name = QString("node %1").arg(counter++);
        createPhyTreeFromPhylipTree(ma, p->next->back,  m, njoin, start, current);
        createPhyTreeFromPhylipTree(ma, p->next->next->back, m, njoin, start, current);
        if (p == start && njoin) {
            createPhyTreeFromPhylipTree(ma, p->back, m, njoin, start, current);
        }
    }

    if (p == start) {
        counter = 0;
    } else {
        PhyNode::addBranch(root, current, p->v);
    }
}  

void replacePhylipRestrictedSymbols(QByteArray& name) {
    static const char badSymbols[] = {',',':','[',']','(',')',';' };
    static int sz = sizeof (badSymbols) / sizeof(char);
    for (int i = 0; i < sz; ++i) {
        name.replace(badSymbols[i], ' ');
    }
}


PhyTree NeighborJoinAdapter::calculatePhyTree( const MAlignment& ma, const CreatePhyTreeSettings& s, TaskStateInfo& ti)
{
    QMutexLocker runLocker( &runLock );

    GCOUNTER(cvar,tvar, "PhylipNeigborJoin" );


    PhyTree phyTree(NULL);

    if (ma.getNumRows() < 3) {
        ti.setError("Neighbor-Joining runs must have at least 3 species");
        return phyTree;
    }

    if(s.bootstrap){ //bootstrapping and creating a consensus tree
        try {
            setTaskInfo(&ti);

            std::auto_ptr<SeqBoot> seqBoot(new SeqBoot);
            seqBoot->generateSequencesFromAlignment(ma,s);

            ti.progress = 99;

        } catch (const char* message) {
            ti.setError(QString("Phylip error %1").arg(message));
        }
    }else{
    
        // Exceptions are used to avoid phylip exit(-1) error handling and canceling task 
        try {   
            
            setTaskInfo(&ti);
            
            std::auto_ptr<DistanceMatrix> distanceMatrix(new DistanceMatrix);
            distanceMatrix->calculateOutOfAlignment(ma,s);

            if (!distanceMatrix->isValid()) {
                ti.setError("Calculated distance matrix is invalid");
                return phyTree;
            }

            int sz = distanceMatrix->rawMatrix.count();

            // Allocate memory resources
            neighbour_init(sz);

            // Fill data
            vector* m = getMtx();
            for (int i = 0; i < sz; ++i) {
                for (int j = 0; j < sz; ++j) {
                    m[i][j] = distanceMatrix->rawMatrix[i][j];
                }
            }

            naym* nayme = getNayme();
            for (int i = 0; i < sz; ++i) {
                const MAlignmentRow& row = ma.getRow(i);
                QByteArray name = row.getName().toAscii();
                replacePhylipRestrictedSymbols(name);
                qstrncpy(nayme[i], name.constData(), sizeof(naym));
            }

            // Calculate tree
            const tree* curTree = neighbour_calc_tree();
            PhyNode* root = new PhyNode();
            bool njoin = true;

            ti.progress = 99;
            createPhyTreeFromPhylipTree(ma, curTree->start, 0.43429448222, njoin, curTree->start, root);

            neighbour_free_resources();

            PhyTreeData* data = new PhyTreeData();
            data->rootNode = root;

            phyTree = data;
        } catch (const char* message) {
            ti.setError(QString("Phylip error %1").arg(message));
        }
    
    }

    return phyTree;

}

void NeighborJoinAdapter::setupCreatePhyTreeUI( CreatePhyTreeDialogController* c, const MAlignment& ma )
{
     CreatePhyTreeWidget* w = new DistMatrixModelWidget(c, ma);
     c->insertWidget(1,w);

     //CreatePhyTreeWidget* b = new SeqBootModelWidget(c, ma);
     //c->insertWidget(2, b);
     c->adjustSize();
}

} 
