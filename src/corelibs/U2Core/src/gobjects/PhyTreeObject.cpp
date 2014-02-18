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

#include <U2Core/DatatypeSerializeUtils.h>
#include <U2Core/RawDataUdrSchema.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SafePoints.h>

#include "GObjectTypes.h"

#include "PhyTreeObject.h"

namespace U2 {

PhyTreeObject * PhyTreeObject::createInstance(const PhyTree &tree, const QString &objectName, const U2DbiRef &dbiRef, U2OpStatus &os, const QVariantMap &hintsMap) {
    U2RawData object(dbiRef);
    object.url = objectName;
    object.serializer = NewickPhyTreeSerializer::ID;

    RawDataUdrSchema::createObject(dbiRef, object, os);
    CHECK_OP(os, NULL);

    U2EntityRef entRef(dbiRef, object.id);
    commit(tree, entRef, os);
    CHECK_OP(os, NULL);

    return new PhyTreeObject(tree, objectName, entRef, hintsMap);
}

void PhyTreeObject::commit(const PhyTree &tree, const U2EntityRef &treeRef, U2OpStatus &os) {
    CHECK_EXT(NULL != tree.data(), os.setError("NULL tree data"), );
    QByteArray data = NewickPhyTreeSerializer::serialize(tree);
    RawDataUdrSchema::writeContent(data, treeRef, os);
}

void PhyTreeObject::commit(const PhyTree &tree, const U2EntityRef &treeRef) {
    U2OpStatus2Log os;
    commit(tree, treeRef, os);
}

void PhyTreeObject::commit() {
    commit(tree, entityRef);
}

void PhyTreeObject::retrieve() {
    U2OpStatus2Log os;
    QString serializer = RawDataUdrSchema::getObject(entityRef, os).serializer;
    CHECK_OP(os, );
    SAFE_POINT(NewickPhyTreeSerializer::ID == serializer, "Unknown serializer id", );

    QByteArray data = RawDataUdrSchema::readAllContent(entityRef, os);
    CHECK_OP(os, );

    tree = NewickPhyTreeSerializer::deserialize(data, os);
}

PhyTreeObject::PhyTreeObject(const QString &objectName, const U2EntityRef &treeRef, const QVariantMap &hintsMap)
: GObject(GObjectTypes::PHYLOGENETIC_TREE, objectName, hintsMap)
{
    entityRef = treeRef;
    retrieve();
}

PhyTreeObject::PhyTreeObject(const PhyTree &tree, const QString &objectName, const U2EntityRef &treeRef, const QVariantMap &hintsMap)
: GObject(GObjectTypes::PHYLOGENETIC_TREE, objectName, hintsMap), tree(tree)
{
    entityRef = treeRef;
}

void PhyTreeObject::onTreeChanged() {
    commit();
    setModified(true);
}

GObject* PhyTreeObject::clone(const U2DbiRef &dstDbi, U2OpStatus &os) const {
    PhyTreeObject* cln = createInstance(tree, getGObjectName(), dstDbi, os, getGHintsMap());
    CHECK_OP(os, NULL);
    cln->setIndexInfo(getIndexInfo());
    return cln;
}

void PhyTreeObject::setTree(const PhyTree& _tree) {
    tree = _tree;
    onTreeChanged();
    emit si_phyTreeChanged();
}

void PhyTreeObject::rerootPhyTree(PhyNode* node) {
    PhyTreeUtils::rerootPhyTree(tree, node);
    onTreeChanged();
    emit si_phyTreeChanged();
}

bool PhyTreeObject::treesAreAlike( const PhyTree& tree1, const PhyTree& tree2 )
{
    
    QList<const PhyNode*> track1 =  tree1->collectNodes();
    QList<const PhyNode*> track2 =  tree2->collectNodes();
    if (track1.count() != track2.count()) {
        return false;
    }

    foreach (const PhyNode* n1, track1) {
        if (n1->getName().isEmpty()) {
            continue;
        } 
        foreach (const PhyNode* n2, track2) {
            if (n2->getName() != n1->getName()) {
                continue;
            }
            if (n1->branchCount() != n2->branchCount()) {
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
        if (node->getName() == name) {
            return node;
        }
    }
    return NULL;
}

}//namespace


