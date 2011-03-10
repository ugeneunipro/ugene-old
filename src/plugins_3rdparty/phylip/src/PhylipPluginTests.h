#ifndef _NEIGHBORJOINTESTS_H
#define _NEIGHBORJOINTESTS_H
#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <U2Algorithm/PhyTreeGeneratorTask.h>

#include <QtXml/QDomElement>
#include <QFileInfo>
#include <QList>

namespace U2{

class PhyTreeObject;
class MAlignmentObject;

class GTest_NeighborJoin : public GTest {
	Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_NeighborJoin, "test-neighbor-join");

    void prepare();
    Task::ReportResult report();
    void cleanup();

private:    
    QString inputDocCtxName;
    QString resultCtxName;
    Document* maDoc;
    Document* treeDoc;
    PhyTreeGeneratorTask* task;
    MAlignmentObject* input;
    PhyTreeObject* treeObjFromDoc;
};


class  PhylipPluginTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}

#endif