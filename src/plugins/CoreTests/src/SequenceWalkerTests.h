#ifndef _U2_SEQUENCEWALKER_TESTS_H_
#define _U2_SEQUENCEWALKER_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include <U2Core/SequenceWalkerTask.h>

namespace U2 {

class GTest_SW_CheckRegion : public GTest, public SequenceWalkerCallback {
    Q_OBJECT
public:

    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SW_CheckRegion, "sw-check-region");

    ReportResult report();

    void onRegion(SequenceWalkerSubtask* t,TaskStateInfo& ti) {Q_UNUSED(t); Q_UNUSED(ti);}

private:    
    int chunkSize;
    int overlap;
    int extraLen;
    U2Region region;
    bool reventNorm;
    QVector<U2Region> result;

protected:
};

class SequenceWalkerTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};
}//namespace
#endif
