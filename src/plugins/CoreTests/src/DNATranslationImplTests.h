#ifndef _U2_DNATRANSLATION_IMPL_TESTS_H_
#define _U2_DNATRANSLATION_IMPL_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>
#include <U2Core/DNAAlphabet.h>

namespace U2 {

class Document;
class LoadDocumentTask;

//---------------------------------------------------------------------
class GTest_DNATranslation3to1Test : public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_DNATranslation3to1Test, "check-translation");

    ReportResult report();

    QString		objContextName;

    int strFrom;
    int strTo;
    QString		stringValue;
};
//---------------------------------------------------------------------
class DNATranslationImplTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

}//namespace
#endif
