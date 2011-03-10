#ifndef _U2_LOAD_REMOTE_DOCUMENT_TEST_H_
#define _U2_LOAD_REMOTE_DOCUMENT_TEST_H_

#include <U2Test/GTest.h>
#include <U2Test/XMLTestUtils.h>

namespace U2 {

    class LoadRemoteDocumentTask;

    class GTest_LoadRemoteDocumentTask : public GTest {
        Q_OBJECT
    public:
        SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_LoadRemoteDocumentTask, "load-remote-document", TaskFlags_NR_FOSCOE);

        void prepare();
        void cleanup();
        ReportResult report();
    private:
        LoadRemoteDocumentTask *t;
        QString dbName;
        QString docId;
        QString expectedDoc;
    };

    class LoadRemoteDocumentTests {
    public:
        static QList<XMLTestFactory*> createTestFactories();
    };


} //namespace

#endif // _U2_LOAD_REMOTE_DOCUMENT_TEST_H_
