/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
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

#ifndef _U2_PHYTREE_H_
#define _U2_PHYTREE_H_

#include <U2Core/global.h>

#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QSharedData>

namespace U2 {

class PhyBranch;
class PhyNode;

class U2CORE_EXPORT PhyTreeData : public QSharedData {
public:
    PhyTreeData();
    PhyTreeData(const PhyTreeData& other);
    ~PhyTreeData();
    QList<const PhyNode*> collectNodes() const;
    void validate() const;
    void print() const;

    PhyNode* rootNode;
};
typedef QSharedDataPointer<PhyTreeData> PhyTree;

class U2CORE_EXPORT PhyNode {
public:
    PhyNode();
    
    void validate(QList<const PhyNode*>& track) const;
    bool isConnected(const PhyNode* node) const;

    static PhyBranch* addBranch(PhyNode* node1, PhyNode* node2, double distance);
    static void removeBranch(PhyNode* node1, PhyNode* node2);

    PhyNode* clone() const;
    void addToTrack(QSet<const PhyNode*>& track) const;

    QString             name;
    QList<PhyBranch*>   branches;
    bool equals(PhyNode* other);
    void print(QList<PhyNode*>& nodes, int distance, int tab);
    void dumpBranches() const;
    ~PhyNode();
};

class U2CORE_EXPORT PhyBranch {
public:
    PhyBranch();
    
    PhyNode* node1;
    PhyNode* node2;
    double   distance;
};

class U2CORE_EXPORT PhyTreeUtils {
public:
    static int getNumSeqsFromNode(const PhyNode *node, const QSet<QString>& names);
};

}//namespace

#endif

