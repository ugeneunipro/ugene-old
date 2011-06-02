#ifndef _U2_API_TEST_H_
#define _U2_API_TEST_H_

#include <U2Test/XMLTestUtils.h>


namespace U2 {

class GTest_APITest : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_APITest, "gtest", TaskFlags_FOSCOE);

    void prepare();
    void run();
    void cleanup();

private:
    QString tcase;
    QStringList excluded;
    QStringList included;
};

} //namespace

#endif
