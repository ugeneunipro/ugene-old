#ifndef _U2_SUBST_MATRIX_TESTS_H_
#define _U2_SUBST_MATRIX_TESTS_H_

#include <U2Test/XMLTestUtils.h>

namespace U2{

class GTest_SubstMatrix : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_SubstMatrix, "check-subst-matrix", TaskFlags_NR_FOSCOE);

    Task::ReportResult report();

private:
    QString file;
    char col, row;
    float expectedVal;
    QString expectedAlphabetId;
};

class SMatrixTests {
public:
    static QList< XMLTestFactory* > createTestFactories();
};

}

#endif

