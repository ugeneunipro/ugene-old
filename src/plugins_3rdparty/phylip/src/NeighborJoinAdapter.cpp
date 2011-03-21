/**
* UGENE - Integrated Bioinformatics Tools.
* Copyright (C) 2008-2011 UniPro <ugene@unipro.ru>
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
#include "NeighborJoinAdapter.h"
#include "DistanceMatrix.h"
#include "DistMatrixModelWidget.h"
#include "SeqBootModelWidget.h"
#include "SeqBootAdapter.h"


#include "neighbor.h"
#include "dnadist.h"
#include "protdist.h"

#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/Task.h>




//#include <QtGui/QPushButton>
#include <QtCore/QVector>
#include <QtCore/QString>

//#include <algorithm>
//#include <exception>
//#include <iostream>
//#include <memory>




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
            ti.setStateDesc("Generating trees");

            std::auto_ptr<SeqBoot> seqBoot(new SeqBoot);
            seqBoot->generateSequencesFromAlignment(ma,s);

            seqBoot->consInit();

            //std::auto_ptr<DistanceMatrix> distanceMatrix(new DistanceMatrix);
            bool initial = true;
            boolean haslengths;
            long nextnode;
            for (int i = 0; i < s.replicates; i++){
                const MAlignment& curMSA = seqBoot->getMSA(i);
                std::auto_ptr<DistanceMatrix> distanceMatrix(new DistanceMatrix);
                distanceMatrix->calculateOutOfAlignment(curMSA,s);

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

//                 if(i == 26){
//                     PhyNode* root = new PhyNode();
//                     bool njoin = true;
// 
//                     ti.progress = 99;
//                     createPhyTreeFromPhylipTree(ma, curTree->start, 0.43429448222, njoin, curTree->start, root);
// 
// 
//                     PhyTreeData* data = new PhyTreeData();
//                     data->rootNode = root;
// 
//                     phyTree = data;
//                 }
             
                for (int k = 0; k < maxgrp; k++) {
                    lengths[k] = -1;
                }
                nextnode = 0;
                haslengths = true;
                nodep_cons = (pointarray)Malloc(2*(1+spp)*sizeof(node *));
                for (int k  = 0; k < spp; k++) {
                    nodep_cons[k] = (node *)Malloc(sizeof(node));
                    for (int j = 0; j < MAXNCH; j++)
                        nodep_cons[k]->nayme[j] = '\0';
                    strncpy(nodep[k]->nayme, nayme[k], MAXNCH);
                }
                for (int k = spp; k < 2*(1+spp); k++)
                    nodep_cons[k] = NULL;
                treeread(intree, &root, treenode, &goteof, &firsttree, nodep, 
                    &nextnode, &haslengths, &grbg, initconsnode,true,-1);
                if (!initial) { 
                    //missingname(curTree->start);
                    reordertips();
                } else {
                    initial = false;
                    //dupname(curTree->start);
                    for (int k = 0; k < sz; k++) {
                        namesAdd(nayme[k]);
                    }
                    initreenode(curTree->start);
                }
                ntrees += trweight;
//                 if (noroot) {
//                     reroot(nodep[outgrno - 1], &nextnode);
//                     didreroot = outgropt;
//                 }
// 
//                 accumulate(curTree->start);
//                 gdispose(curTree->start);

                //store cloned tree and deleting used MSA from the vector

                neighbour_free_resources();
            }
            /*
            consensus(NULL, s.replicates);

            PhyNode* root1 = new PhyNode();
            bool njoin = true;

            ti.progress = 99;
            createPhyTreeFromPhylipTree(ma, root, 0.43429448222, njoin, root, root1);

            PhyTreeData* data = new PhyTreeData();
            data->rootNode = root1;

            phyTree = data;*/
            
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
     c->insertContrWidget(1,w);

//      CreatePhyTreeWidget* b = new SeqBootModelWidget(c, ma);
//      c->insertContrWidget(2, b);
     c->adjustSize();
}

} 
