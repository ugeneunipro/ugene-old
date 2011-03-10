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

