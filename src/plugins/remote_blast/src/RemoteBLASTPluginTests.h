#ifndef _U2_REMOTE_QUERY_PLUGIN_TEST_H_
#define _U2_REMOTE_QUERY_PLUGIN_TEST_H_

#include <U2Test/XMLTestUtils.h>
#include "RemoteBLASTTask.h"


namespace U2 {

//cppcheck-suppress noConstructor
class GTest_RemoteBLAST : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_RemoteBLAST, "plugin_remote-query");

    void prepare();
    void cleanup();
    Task::ReportResult report();
private:
    AnnotationTableObject *ao;
    RemoteBLASTToAnnotationsTask *task;
    int minLength;
    int maxLength;
    QString index;
    QString sequence;
    QString request;
    QString algoritm;
    QStringList expectedResults;
};

}//ns

#endif
