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
    void setTree(const PhyTree& _tree);

    void rerootPhyTree(PhyNode* node);
    // Warning!
    // PhyBranches can be accessed and modified!
    // TODO: move branches to private data, add getters and setters
    const PhyNode* findPhyNodeByName(const QString& name);

    virtual GObject* clone(const U2DbiRef&, U2OpStatus&) const;
    PhyTreeObject* clone() const;

    void update(){ tree->update();}
    
    // Utility functions
    
    // Compares number of nodes and nodes with names (how many nodes etc.)
    static bool treesAreAlike(const PhyTree& tree1, const PhyTree& tree2);
signals:
    void si_phyTreeChanged();
protected:
    PhyTree tree;
};


}//namespace


#endif
