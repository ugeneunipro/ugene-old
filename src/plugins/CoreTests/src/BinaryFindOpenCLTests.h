#ifndef __BINARY_FIND_OPENCL_TESTS_H__
#define __BINARY_FIND_OPENCL_TESTS_H__

#include <QtCore/QVector>

#include <U2Algorithm/BinaryFindOpenCL.h>

#include <U2Test/XMLTestUtils.h>

namespace U2{

class GTest_BinaryFindOpenCL : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_BinaryFindOpenCL, "check-binary-find-opencl", TaskFlags_NR_FOSCOE);

    void prepare();
    Task::ReportResult report();

private:
    QVector<NumberType> numbers;
    QVector<NumberType> findNumbers;
    QVector<NumberType> expectedResults;
    NumberType* results;
};

class BinaryFindOpenCLTests {
public:
    static QList< XMLTestFactory* > createTestFactories();
};

} //namespace
#endif // __BINARY_FIND_OPENCL_TESTS_H__
