#ifndef _U2_PWMatrix_TESTS_H_
#define _U2_PWMatrix_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

#include <U2Core/PFMatrix.h>
#include <U2Core/PWMatrix.h>

namespace U2 {

class Document;
class LoadDocumentTask;

//---------------------------------------------------------------------
class GTest_PFMtoPWMConvertTest : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PFMtoPWMConvertTest, "check-pfm2pwm-convert");
private:
    QString             objContextName;
    PWMatrixType        type;
public:
    ReportResult        report();
};

class GTest_PFMCreateTest : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PFMCreateTest, "check-pfm-create");
private:
    QString                 objContextName;
    QString                 objType;
    int                     size;
    int                     length;
    PFMatrixType            type;
    QVarLengthArray<int>    values[16];
public:
    ReportResult report();
};

class GTest_PWMCreateTest : public GTest {
    Q_OBJECT
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_PWMCreateTest, "check-pwm-create");

private:
    QString                 objContextName;
    QString                 objType;
    QString                 algo;
    int                     size;
    int                     length;
    PWMatrixType            type;
    QVarLengthArray<double> values[16];
public:
    ReportResult report();
};
//---------------------------------------------------------------------
class PWMatrixTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}//namespace
#endif
