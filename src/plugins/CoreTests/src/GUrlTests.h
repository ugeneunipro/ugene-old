#ifndef _U2_GURL_TESTS_H_
#define _U2_GURL_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <QtXml/QDomElement>

namespace U2 {
    class GTest_ConvertPath : public GTest {
        Q_OBJECT
    public:
        SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_ConvertPath, "convert-path");

        ReportResult report();

        virtual void cleanup() {};

    private:
        QString             originalUrl, result, expectedResult, platform;
        bool                isFileUrl, runThisTest;
    };

    class GUrlTests {
    public:
        static QList<XMLTestFactory*> createTestFactories();
    };
}//namespace
#endif
