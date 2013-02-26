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

#ifndef _U2_PHYTREE_H_
#define _U2_PHYTREE_H_

#include <U2Core/global.h>

#include <QtCore/QSet>
#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QSharedData>

namespace U2 {

class PhyNode;
class PhyBranch;

class U2CORE_EXPORT PhyTreeData : public QSharedData {
public:
    PhyTreeData();
    PhyTreeData(const PhyTreeData& other);
    ~PhyTreeData();
    QList<const PhyNode*> collectNodes() const;
    void validate() const;
    void print() const;
    void update(){}

    void setRootNode(PhyNode* _rootNode) {rootNode = _rootNode;}
    PhyNode* getRootNode() const { return rootNode;}

    static PhyBranch* addBranch(PhyNode* node1, PhyNode* node2, double distance);
    static void removeBranch(PhyNode* node1, PhyNode* node2);

private:
    PhyNode* rootNode;
};
typedef QSharedDataPointer<PhyTreeData> PhyTree;

class U2CORE_EXPORT PhyBranch : public QObject{
    Q_OBJECT
public:
    PhyBranch();

    void movingToAnotherAddress(PhyBranch* newAdress);

    PhyNode* node1;
    PhyNode* node2;
    double   distance;
signals:
    void si_onAdressChanged(PhyBranch* );
};

class U2CORE_EXPORT PhyNode {
    friend class PhyTreeUtils;
    friend class PhyTreeData;
public:
    PhyNode();
    
    void validate(QList<const PhyNode*>& track) const;
    bool isConnected(const PhyNode* node) const;

    PhyNode* getParentNode();

    void swapBranches(int firstBrunch, int secondBranch){branches.swap(firstBrunch, secondBranch);}

    int getNumberOfBranches() const {return branches.size();}

    PhyNode* getFirstNodeOfBranch(int branchNumber) const {return branches.at(branchNumber)->node1;}
    PhyNode* getSecondNodeOfBranch(int branchNumber) const {return branches.at(branchNumber)->node2;}

    double getBranchesDistance(int branchNumber) const { return branches.at(branchNumber)->distance;}
    void setBranchesDistance(int branchNumber, double _distance) { branches.at(branchNumber)->distance = _distance;}

    void removeBrunch(PhyBranch* brunch) {branches.removeAll(brunch);}

    PhyNode* clone() const;
    void addToTrack(QSet<const PhyNode*>& track) const;

    const QString& getName() const { return name; } 
    void setName(const QString& _name){ name = _name; }

    bool equals(PhyNode* other);
    void print(QList<PhyNode*>& nodes, int distance, int tab);
    void dumpBranches() const;
    ~PhyNode();

    QList<PhyBranch*>   branches;
private:
    QString             name;
};

class U2CORE_EXPORT PhyTreeUtils {
public:
    static int getNumSeqsFromNode(const PhyNode *node, const QSet<QString>& names);
};

}//namespace

#endif

