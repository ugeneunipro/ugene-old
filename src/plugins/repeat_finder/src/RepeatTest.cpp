#ifdef _U2_REPTEST_H_
#include "RepeatTest.h"

#include <U2Core/AppContext.h>
#include <U2Algorithm/RepeatFinderTaskFactoryRegistry.h>


namespace U2 {

static TestData createData() {
    TestData d;
    d.factoryId = "";
    return d;
}

TEST_P(RepeatTest, t01) {
    RepeatFinderTaskFactoryRegistry* r = AppContext::getRepeatFinderTaskFactoryRegistry();
    ASSERT_NE(r, (RepeatFinderTaskFactoryRegistry*)NULL);
    RepeatFinderTaskFactory* t = AppContext::getRepeatFinderTaskFactoryRegistry()->getFactory(testData.factoryId);
    ASSERT_NE(t, (RepeatFinderTaskFactory*)NULL);
}

INSTANTIATE_TEST_CASE_P(
                        RepeatTestInstance,
                        RepeatTest,
                        ::testing::Values(createData()));

} //namespace
#endif