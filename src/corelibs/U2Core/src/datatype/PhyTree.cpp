/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
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

#include "PhyTree.h"
#include <iostream>

namespace U2 {

PhyTreeData::PhyTreeData() : rootNode(NULL)  {
}

PhyTreeData::PhyTreeData(const PhyTreeData& other) : QSharedData(other) {
    rootNode = other.rootNode == NULL ? NULL : other.rootNode->clone();
}

PhyTreeData::~PhyTreeData() {
    if (rootNode != NULL){
        delete rootNode;
        rootNode = NULL;
    }    
}

PhyNode::PhyNode() {
}

PhyBranch::PhyBranch() : node1(NULL), node2(NULL), distance(0) {
}

void PhyTreeData::print() const{
    QList<PhyNode* > nodes;
    int tab = 0;
    int distance = 0;
    rootNode->print(nodes,distance, tab);
}

void PhyTreeData::validate() const {
#ifdef _DEBUG
    if (rootNode != NULL) {
        QList<const PhyNode*> track;
        rootNode->validate(track);
    }
#endif
}

QList<const PhyNode*> PhyTreeData::collectNodes() const
{
    QList<const PhyNode*> track;

    if (rootNode != NULL) {
        rootNode->validate(track);
    }
    return track;

}
void PhyNode::validate(QList<const PhyNode*>& track) const {
    if ( track.contains(this) )
        return;
    track.append(this);
    foreach(PhyBranch* b, branches) {
        assert(b->node1 != NULL && b->node2!=NULL);
        if (b->node1 != this) {
            b->node1->validate(track);
        } else if (b->node2 != this) {
            b->node2->validate(track);
        }
    }    
}

bool PhyNode::isConnected(const PhyNode* node) const {
    foreach(PhyBranch* b, branches) {
        if (b->node1 == node || b->node2 == node) {
            return true;
        }
    }
    return false;
}

PhyBranch* PhyNode::addBranch(PhyNode* node1, PhyNode* node2, double distance) {
    assert(!node1->isConnected(node2));

    PhyBranch* b = new PhyBranch();
    b->distance = distance;
    b->node1 = node1;
    b->node2 = node2;
    
    node1->branches.append(b);
    node2->branches.append(b);
    
    return b;
}

void PhyNode::removeBranch(PhyNode* node1, PhyNode* node2) {
    foreach(PhyBranch* b, node1->branches) {
        if (b->node1 == node1 && b->node2 == node2) {
            node1->branches.removeAll(b);
            node2->branches.removeAll(b);
            delete b;
            return;
        }
    }
    assert(0);
}

void PhyNode::dumpBranches() const{
    std::cout<<"Branches are: ";
    for(int i =0; i<branches.size(); i++){
        QString node1name = "null";
        if(branches[i]->node1!=0){
            node1name = branches[i]->node1->name;
        }

        QString node2name = "null";
        if(branches[i]->node2!=0){
            node2name = branches[i]->node2->name;
        }
        std::cout<<"branch from node "<<node1name.toAscii().constData()
            <<" to "<<node2name.toAscii().constData()
            <<" with distance "<<branches[i]->distance<<std::endl;
    }

}

PhyNode::~PhyNode(){
    for (int i = 0, s = branches.size(); i < s; ++i) {
        if (branches[i]->node2 != this) {
            delete branches[i]->node2;
            delete branches[i];
        }
    }
}

PhyNode* PhyNode::clone() const {
    QSet<const PhyNode*> track;
    addToTrack(track);

    QSet<PhyBranch*> allBranches;
    QMap<const PhyNode*, PhyNode*> nodeTable;
    foreach(const PhyNode* n, track) {
        PhyNode* n2 = new PhyNode();
        n2->name = n->name;
        nodeTable[n] = n2;
        foreach(PhyBranch* b, n->branches) {
            allBranches.insert(b);
        }
    }
    foreach(PhyBranch* b, allBranches) {
        PhyNode* node1 = nodeTable[b->node1];
        PhyNode* node2 = nodeTable[b->node2];
        assert(node1!=NULL && node2!=NULL);
        PhyNode::addBranch(node1, node2, b->distance);
    }
    PhyNode* myClone = nodeTable.value(this);
    assert(myClone!=NULL);
    return myClone;
}

void PhyNode::print(QList<PhyNode*>& nodes, int tab, int distance){
    if(nodes.contains(this)){
        return;
    }
    nodes.append(this);
    for(int i=0; i<tab; i++){
        std::cout<<" ";
    }
    tab++;
    std::cout<<"name: "<<this->name.toAscii().constData() <<" distance: "<<distance<<std::endl;
    QList<PhyBranch* > blist = this->branches;
    int s = blist.size();
    for(int i=0; i<s; i++){
        if(blist[i]->node2!=0){
            int d = blist[i]->distance;
            blist[i]->node2->print(nodes, tab, d);
        }
    }
}


void PhyNode::addToTrack(QSet<const PhyNode*>& track) const {
    if (track.contains(this)) {
        return;
    }
    track.insert(this);
    foreach(PhyBranch* b, branches) {
        b->node1->addToTrack(track);
        b->node2->addToTrack(track);
    }
}

int PhyTreeUtils::getNumSeqsFromNode(const PhyNode *node, const QSet<QString> &names) {
    int size = node->branches.size();
    if (size > 1) {
        int s = 0;
        for (int i = 0; i < size; ++i) {
            if (node->branches[i]->node2 != node) {
                int num = getNumSeqsFromNode(node->branches[i]->node2, names);
                if (!num) {
                    return 0;
                }
                s += num;
            }
        }
        return s;
    } else {
        QString str = node->name;
        return names.contains(str.replace('_', ' ')) ? 1 : 0;
    }
}

}//namespace
