/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2014 UniPro <ugene@unipro.ru>
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

#include "PhylipPlugin.h"
#include "PhylipPluginTests.h"

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/SaveDocumentTask.h>

#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/Log.h>

#include <U2Core/GObjectTypes.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/PhyTreeObject.h>

#include <QtCore/QDir>
#include <U2Core/AppContext.h>
#include "DistanceMatrix.h"
#include <U2Algorithm/CreatePhyTreeSettings.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>

namespace U2{
	
QList<XMLTestFactory*> PhylipPluginTests::createTestFactories(){
	QList<XMLTestFactory* > res;
	res.append(GTest_NeighborJoin::createFactory());
    return res;
}

void GTest_NeighborJoin::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    treeObjFromDoc = NULL;
	task = NULL;
    input = NULL;
    inputDocCtxName = el.attribute("in");
    if (inputDocCtxName.isEmpty()) {
        failMissingValue("in");
        return;
    }
    resultCtxName = el.attribute("sample");
    QString bootstrSeed;
    bootstrSeed = el.attribute("bootstrapSeed");
    if(bootstrSeed == NULL){
        bootStrapSeed = -1;
    }else{
        bootStrapSeed = bootstrSeed.toInt();
    }
}

void GTest_NeighborJoin::prepare() {

    maDoc = getContext<Document>(this, inputDocCtxName);
    if (maDoc == NULL) {
        stateInfo.setError(  QString("context not found %1").arg(inputDocCtxName) );
        return;
    }

    QList<GObject*> list = maDoc->findGObjectByType(GObjectTypes::MULTIPLE_ALIGNMENT);
    if (list.size() == 0) {
        stateInfo.setError(  QString("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT) );
        return;
    }

    GObject *obj = list.first();
    if(obj==NULL){
        stateInfo.setError(  QString("object with type \"%1\" not found").arg(GObjectTypes::MULTIPLE_ALIGNMENT) );
        return;
    }
    assert(obj!=NULL);
    MAlignmentObject* ma = qobject_cast<MAlignmentObject*>(obj);
    if(ma==NULL){
        stateInfo.setError(  QString("error can't cast to multiple alignment from GObject") );
        return;
    }

	input = ma;

	treeDoc = getContext<Document>(this, resultCtxName);
	if (treeDoc == NULL) {
		stateInfo.setError(  QString("context not found %1").arg(resultCtxName) );
		return;
	}

	QList<GObject*> list2 = treeDoc->findGObjectByType(GObjectTypes::PHYLOGENETIC_TREE);
	if (list2.size() == 0) {
		stateInfo.setError(  QString("container of object with type \"%1\" is empty").arg(GObjectTypes::MULTIPLE_ALIGNMENT) );
		return;
	}

	GObject *obj2 = list2.first();
	if(obj2==NULL){
		stateInfo.setError(  QString("object with type \"%1\" not found").arg(GObjectTypes::PHYLOGENETIC_TREE) );
		return;
	}
	
	treeObjFromDoc = qobject_cast<PhyTreeObject*>(obj2);
	
	if(treeObjFromDoc == NULL){
		stateInfo.setError(  QString("error can't cast to phylogenetic tree from GObject") );
		return;
	}
	assert( obj != NULL);

    CreatePhyTreeSettings settings;
    settings.algorithmId = PhylipPlugin::PHYLIP_NEIGHBOUR_JOIN;

    if(bootStrapSeed != -1){
        settings.bootstrap = true;
        settings.seed = bootStrapSeed;
        settings.consensusID =  "Majority Rule (extended)";
        settings.replicates = 100;
    }

    task = new PhyTreeGeneratorLauncherTask(input->getMAlignment(), settings);
    addSubTask(task);
}

Task::ReportResult GTest_NeighborJoin::report() {
    if (!task->hasError()) {
        const PhyTree computedTree = task->getResult();
        const PhyTree& treeFromDoc = treeObjFromDoc->getTree(); 
        bool same = PhyTreeObject::treesAreAlike(computedTree, treeFromDoc);
        if(!same){
 		    stateInfo.setError("Trees are not equal");
        }
    }
    
    return ReportResult_Finished;
}

void GTest_NeighborJoin::cleanup() {
	
}

}
