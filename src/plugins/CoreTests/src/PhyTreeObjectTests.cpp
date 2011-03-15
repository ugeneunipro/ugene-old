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

#include <QtCore/QDir>
#include <QtCore/QFileInfoList>


#include <U2Core/AppContext.h>
#include <U2Core/PhyTreeObject.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Algorithm/PhyTreeGeneratorRegistry.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/DocumentModel.h>

#include "PhyTreeObjectTests.h"

namespace U2 { 

#define ALGORITHM_ATTR      "algorithm"
#define INDEX_ATTR          "index"
#define OBJ_ATTR            "obj"
#define DOC_ATTR            "doc"
#define PHYNODE_ATTR        "phynode"
#define SIBLING_ATTR        "sibling"
#define VALUE_ATTR          "value"

#define EPS 0.0001


void GTest_CalculateTreeFromAligment::init(XMLTestFormat *tf, const QDomElement& el)  {
    Q_UNUSED(tf);

    task = NULL; 
    objContextName = el.attribute(OBJ_ATTR);
    if (objContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    treeObjContextName = el.attribute(INDEX_ATTR);
    if (treeObjContextName.isEmpty()) {
        failMissingValue(INDEX_ATTR);
        return;
    }

    algName = el.attribute(ALGORITHM_ATTR);
    if (algName.isEmpty()) {
        failMissingValue(ALGORITHM_ATTR);
        return;
    }

}
void GTest_CalculateTreeFromAligment::prepare() {

    GObject *obj = getContext<GObject>(this, objContextName);
    if(obj==NULL){
        stateInfo.setError(QString("wrong value: %1").arg(objContextName));
        return;  
    }

    MAlignmentObject* maObj = qobject_cast<MAlignmentObject*>(obj);
    if(maObj==NULL){
        stateInfo.setError(QString("can't cast to multimple alignment object from: %1").arg(obj->getGObjectName()));
        return;
    }

    CreatePhyTreeSettings settings;
    settings.algorithmId = algName;

    task = new PhyTreeGeneratorTask(maObj->getMAlignment(), settings);

    if (task == NULL) {
        stateInfo.setError(QString("Algorithm %1 not found").arg(algName));
        return;
    }
    addSubTask(task);   
}


Task::ReportResult GTest_CalculateTreeFromAligment::report()
{
    if (task == NULL) {
        return ReportResult_Finished;
    }

    if (!task->hasErrors()) {
        PhyTree tree = task->getResult();
        PhyTreeObject* obj = new PhyTreeObject(tree, treeObjContextName);
        addContext(treeObjContextName,obj);
    }        

    return ReportResult_Finished;
}



///////////////////////////////////////////////////////////////////////////////////////////

void GTest_CheckPhyNodeHasSibling::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    treeContextName = el.attribute(OBJ_ATTR);
    if (treeContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    nodeName = el.attribute(PHYNODE_ATTR);
    if (nodeName.isEmpty()) {
        failMissingValue(PHYNODE_ATTR);
        return;
    }

    siblingName = el.attribute(SIBLING_ATTR);
    if (siblingName.isEmpty()) {
        failMissingValue(SIBLING_ATTR);
        return;
    } 


}

Task::ReportResult GTest_CheckPhyNodeHasSibling::report()
{

    PhyTreeObject *treeObj = getContext<PhyTreeObject>(this, treeContextName);
    if(NULL == treeObj){
        stateInfo.setError(QString("wrong value: %1").arg(treeContextName));
        return ReportResult_Finished;
    }

    const PhyNode* node = treeObj->findPhyNodeByName(nodeName);
    if (NULL == node) {
        stateInfo.setError(QString("Node %1 not found in tree").arg(nodeName));
        return ReportResult_Finished;
    }
    
    bool foundSibling = false;
    
    const QList<PhyBranch*> branches = node->branches;
    assert(branches.count() == 1);
    const PhyBranch* parentBranch = branches.at(0);
    const PhyNode* parent = parentBranch->node1 == node ? parentBranch->node2 : parentBranch->node1;
    
    foreach(const PhyBranch* branch, parent->branches) {
        if ( (parent == branch->node1 && branch->node2->name == siblingName) ||
             ((branch->node1->name == siblingName) && (node == branch->node1)) )    
        {
                foundSibling = true;
                break;
        }
    }
    
    if (!foundSibling) {
        stateInfo.setError(QString("Node %1 doesn't have sibling %2").arg(nodeName).arg(siblingName));
        return ReportResult_Finished;
    }

    return ReportResult_Finished;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void GTest_CheckPhyNodeBranchDistance::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    treeContextName = el.attribute(OBJ_ATTR);
    if (treeContextName.isEmpty()) {
        failMissingValue(OBJ_ATTR);
        return;
    }

    nodeName = el.attribute(PHYNODE_ATTR);
    if (nodeName.isEmpty()) {
        failMissingValue(PHYNODE_ATTR);
        return;
    } 

    QString v = el.attribute(VALUE_ATTR);
    if (v.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    }    

    bool ok = false;
    distance = v.toDouble(&ok);
    if (!ok) {
        failMissingValue(VALUE_ATTR);
    }

}

Task::ReportResult GTest_CheckPhyNodeBranchDistance::report()
{
    if (stateInfo.hasErrors()) {
        return ReportResult_Finished;
    }

    PhyTreeObject *treeObj = getContext<PhyTreeObject>(this, treeContextName);
    if(NULL == treeObj){
        stateInfo.setError(QString("wrong value: %1").arg(treeContextName));
        return ReportResult_Finished;  
    }

    const PhyNode* node = treeObj->findPhyNodeByName(nodeName);
    if (NULL == node) {
        stateInfo.setError(QString("Node %1 not found in tree").arg(nodeName));
        return ReportResult_Finished;
    }

    const QList<PhyBranch*> branches = node->branches;
    assert(branches.count() == 1);
    const PhyBranch* parentBranch = branches.at(0);
    double chkDistance = parentBranch->distance;
    if (distance - chkDistance > EPS) {
        stateInfo.setError(QString("Distances don't match! Expected %1, real dist is %2").arg(distance).arg(chkDistance));
    }
    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////////////////////

void GTest_CompareTreesInTwoObjects::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    docContextName = el.attribute(DOC_ATTR);
    if (docContextName.isEmpty()) {
        failMissingValue(DOC_ATTR);
        return;
    }

    secondDocContextName = el.attribute(VALUE_ATTR);
    if (secondDocContextName.isEmpty()) {
        failMissingValue(VALUE_ATTR);
        return;
    } 
}

Task::ReportResult GTest_CompareTreesInTwoObjects::report() {
    Document* doc = getContext<Document>(this, docContextName);
    if (doc == NULL) {
        stateInfo.setError(QString("document not found %1").arg(docContextName));
        return ReportResult_Finished;
    }
    Document* doc2 = getContext<Document>(this, secondDocContextName);
    if (doc2 == NULL) {
        stateInfo.setError(QString("document not found %1").arg(secondDocContextName));
        return ReportResult_Finished;
    }

    const QList<GObject*>& objs = doc->getObjects();
    const QList<GObject*>& objs2 = doc2->getObjects();
    for (int i = 0; i < objs.size() && i < objs2.size(); ++i){
        GObject *obj = objs.at(i), *obj2 = objs2.at(i);
        PhyTreeObject *treeObj = NULL, *treeObj2 = NULL;
        if (obj->getGObjectType() == GObjectTypes::PHYLOGENETIC_TREE){
            treeObj = qobject_cast<PhyTreeObject*>(obj);
        }
        if (obj2->getGObjectType() == GObjectTypes::PHYLOGENETIC_TREE){
            treeObj2 = qobject_cast<PhyTreeObject*>(obj2);
        }
        if (treeObj == NULL){
            stateInfo.setError(QString("can't cast to tree from: %1 in position %2").arg(obj->getGObjectName()).arg(i));
            return ReportResult_Finished;
        }
        if (treeObj2 == NULL){
            stateInfo.setError(QString("can't cast to tree from: %1 in position %2").arg(obj2->getGObjectName()).arg(i));
            return ReportResult_Finished;
        }

        if (!PhyTreeObject::treesAreAlike(treeObj->getTree(), treeObj2->getTree())) {
            stateInfo.setError(QString("trees in position %1 are different").arg(i));
            return ReportResult_Finished;
        }
    }

    return ReportResult_Finished;
}

///////////////////////////////////////////////////////////////////////////////////////////

QList<XMLTestFactory*> PhyTreeObjectTests::createTestFactories()
{
    QList<XMLTestFactory*> res;
    res.append(GTest_CalculateTreeFromAligment::createFactory());
    res.append(GTest_CheckPhyNodeHasSibling::createFactory());
    res.append(GTest_CheckPhyNodeBranchDistance::createFactory());
    res.append(GTest_CompareTreesInTwoObjects::createFactory());
    
    return res;

}


} //namespace
