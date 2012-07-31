#ifdef _U2_REPTEST_H_

#include <gtest/gtest.h>

#include <QtCore/QString>


namespace U2 {

class TestData {
public:
    QString factoryId;
};

class RepeatTest : public ::testing::TestWithParam<TestData> {
protected:
    virtual void SetUp() { testData = GetParam(); }
    virtual void TearDown() {}
protected:
    TestData testData;
};

} // namespace

#endif
