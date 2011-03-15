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


