#ifndef DBITEST_H
#define DBITEST_H

#include <U2Core/U2Dbi.h>
#include <U2Test/TestRunnerSettings.h>
#include <gtest/gtest.h>
#include <memory>


namespace U2 {

class BaseDbiTest : public ::testing::TestWithParam<APITestData> {
public:
    static const QString DB_URL;

protected:
    virtual void SetUp();
    virtual void TearDown();

protected:
    std::auto_ptr<U2Dbi> dbi;
    QHash<QString, QString> properties;
    APITestData testData;
};

} // namespace U2

#endif // DBITEST_H
