#include "PhyTreeObject.h"

namespace U2 {

GObject* PhyTreeObject::clone() const {
    PhyTreeObject* cln = new PhyTreeObject(tree, getGObjectName(), getGHintsMap());
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

bool PhyTreeObject::treesAreAlike( const PhyTree& tree1, const PhyTree& tree2 )
{
    
    QList<const PhyNode*> track1 =  tree1->collectNodes();
    QList<const PhyNode*> track2 =  tree2->collectNodes();
    if (track1.count() != track2.count()) {
        return false;
    }

    foreach (const PhyNode* n1, track1) {
        if (n1->name.isEmpty()) {
            continue;
        } 
        foreach (const PhyNode* n2, track2) {
            if (n2->name != n1->name) {
                continue;
            }
            if (n1->branches.count() != n2->branches.count()) {
                return false;
            }
        }


    }

    return true;
}

const PhyNode* PhyTreeObject::findPhyNodeByName( const QString& name )
{
    QList<const PhyNode*> nodes = tree.constData()->collectNodes();
    foreach (const PhyNode* node, nodes) {
        if (node->name == name) {
            return node;
        }
    }
    return NULL;
}

}//namespace


