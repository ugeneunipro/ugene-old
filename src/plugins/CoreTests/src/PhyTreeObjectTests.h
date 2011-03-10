#ifndef _U2_PHYLTREE_OBJECT_TESTS_H_
#define _U2_PHYLTREE_OBJECT_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <QtXml/QDomElement>
#include <QtCore/QHash>

namespace U2 {

class Document;
class PhyTreeGeneratorTask;

class GTest_CalculateTreeFromAligment : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CalculateTreeFromAligment, "calc-tree-from-alignment");

    ReportResult report();
    void prepare();

    PhyTreeGeneratorTask* task;
    QString objContextName;
    QString treeObjContextName;
    QString algName;
};

class GTest_CheckPhyNodeHasSibling : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckPhyNodeHasSibling, "check-phynode-has-sibling");

    ReportResult report();

    QString treeContextName;
    QString nodeName;
    QString siblingName;
    
};

class GTest_CheckPhyNodeBranchDistance : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CheckPhyNodeBranchDistance, "check-phynode-branch-distance");
 
    ReportResult report();
 
    QString treeContextName;
    QString nodeName;
    double distance;

};

class GTest_CompareTreesInTwoObjects : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CompareTreesInTwoObjects, "compare-trees-in-two-objects");

    ReportResult report();
    QString docContextName;
    QString secondDocContextName;
};

class PhyTreeObjectTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};



}//namespace

#endif //_U2_BIOSTRUCT3D_OBJECT_TESTS_H_
