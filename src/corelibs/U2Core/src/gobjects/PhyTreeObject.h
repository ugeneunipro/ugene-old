#ifndef _U2_PHYTREE_OBJECT_H_
#define _U2_PHYTREE_OBJECT_H_

#include "GObjectTypes.h"

#include <U2Core/GObject.h>
#include <U2Core/PhyTree.h>

namespace U2 {

class U2CORE_EXPORT PhyTreeObject : public GObject {
    Q_OBJECT
public:
    PhyTreeObject(const PhyTree& _tree, const QString& objectName, const QVariantMap& hintsMap = QVariantMap()) 
        : GObject(GObjectTypes::PHYLOGENETIC_TREE, objectName, hintsMap), tree(_tree){};

    virtual const PhyTree& getTree() const {return tree;}
    
    // Warning!
    // PhyBranches can be accessed and modified!
    // TODO: move branches to private data, add getters and setters
    const PhyNode* findPhyNodeByName(const QString& name);

    virtual GObject* clone() const;
    
    // Utility functions
    
    // Compares number of nodes and nodes with names (how many nodes etc.)
    static bool treesAreAlike(const PhyTree& tree1, const PhyTree& tree2);

protected:
    PhyTree tree;
};


}//namespace


#endif
