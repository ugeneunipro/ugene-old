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

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QTemporaryFile>


namespace U2 {

QMutex NeighborJoinAdapter::runLock;

void createPhyTreeFromPhylipTree(const MAlignment &ma, node *p, double m, boolean njoin, node *start, PhyNode* root, int bootstrap_repl)
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
    if(p){
        if (p->tip) {
            if(bootstrap_repl != 0){
                current->name = QString::fromAscii(p->nayme);
            }else{
                assert(p->index - 1 < ma.getNumRows());
                current->name = ma.getRow(p->index - 1).getName();
            }
        } else {
            current->name = QString("node %1").arg(counter++);
            createPhyTreeFromPhylipTree(ma, p->next->back,  m, njoin, start, current, bootstrap_repl);
            createPhyTreeFromPhylipTree(ma, p->next->next->back, m, njoin, start, current, bootstrap_repl);
            if (p == start && njoin) {
                createPhyTreeFromPhylipTree(ma, p->back, m, njoin, start, current, bootstrap_repl);
            }
        }

        if (p == start) {
            counter = 0;
        } else {
            if(bootstrap_repl != 0){
                if(p->deltav == 0){
                    PhyNode::addBranch(root, current, bootstrap_repl);
                }else{
                    PhyNode::addBranch(root, current, p->deltav );
                }
            }else{
                PhyNode::addBranch(root, current, p->v);
            }
            
        }
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
            setBootstr(true);
            ti.setDescription("Generating sequences");

            std::auto_ptr<SeqBoot> seqBoot(new SeqBoot);

            QTemporaryFile tmpFile;
            QString path = seqBoot->getTmpFileTemplate();
            if(!path.isEmpty()){
                tmpFile.setFileTemplate(path);
            }
            if(!tmpFile.open()){
                ti.setError("Can't create temporary file");
                return phyTree;
            }

            seqBoot->generateSequencesFromAlignment(ma,s);

            ti.setDescription("Calculating trees");

            bool initial = true;
            for (int i = 0; i < s.replicates; i++){
                ti.progress = (int)(i/(float)s.replicates * 100);

                const MAlignment& curMSA = seqBoot->getMSA(i);
                std::auto_ptr<DistanceMatrix> distanceMatrix(new DistanceMatrix);
                distanceMatrix->calculateOutOfAlignment(curMSA,s);

                if (!distanceMatrix->isValid()) {
                    ti.setError("Calculated distance matrix is invalid");
                    return phyTree;
                }

                int sz = distanceMatrix->rawMatrix.count();

                // Allocate memory resources
                neighbour_init(sz, tmpFile.fileName());

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

                    for(int j = name.length(); j < nmlngth; j++){
                        nayme[i][j] = ' ';
                    }
                }

                // Calculate tree
                const tree* curTree = neighbour_calc_tree(); 

                neighbour_free_resources();
            }
            ti.progress = 99;
            ti.setDescription("Calculating consensus tree");

            if(s.consensusID == ConsensusModelTypes::Strict){
                consens_starter(tmpFile.fileName().toStdString().c_str(), s.fraction, true, false, false, false);
            }else if(s.consensusID == ConsensusModelTypes::MajorityRuleExt){
                consens_starter(tmpFile.fileName().toStdString().c_str(), s.fraction, false, true, false, false);
            }else if(s.consensusID == ConsensusModelTypes::MajorityRule){
                consens_starter(tmpFile.fileName().toStdString().c_str(), s.fraction, false, false, true, false);
            }else if(s.consensusID == ConsensusModelTypes::M1){
                consens_starter(tmpFile.fileName().toStdString().c_str(), s.fraction, false, false, false, true);
            }else{
                assert(0);
            }

            PhyNode* rootPhy = new PhyNode();
            bool njoin = true;

            createPhyTreeFromPhylipTree(ma, root, 0.43429448222, njoin, root, rootPhy, s.replicates);

            consens_free_res();

            PhyTreeData* data = new PhyTreeData();
            data->rootNode = rootPhy;

            phyTree = data;

         } catch (const char* message) {
            ti.setError(QString("Phylip error %1").arg(message));
        }
    }else{
    
        // Exceptions are used to avoid phylip exit(-1) error handling and canceling task 
        try {   
                    
            setTaskInfo(&ti);
            setBootstr(false);
            
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
            createPhyTreeFromPhylipTree(ma, curTree->start, 0.43429448222, njoin, curTree->start, root, 0);

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

      CreatePhyTreeWidget* b = new SeqBootModelWidget(c, ma);
      c->insertContrWidget(2, b);
     c->adjustSize();
}

} 
