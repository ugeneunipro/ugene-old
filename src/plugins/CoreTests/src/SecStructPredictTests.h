#ifndef _U2_SEC_STRUCT_PREDICT_TESTS_H_
#define _U2_SEC_STRUCT_PREDICT_TESTS_H_

#include <U2Test/XMLTestUtils.h>


namespace U2 {

class SecStructPredictTask;
class AnnotationTableObject;


class GTest_SecStructPredictAlgorithm : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SecStructPredictAlgorithm, "test-sec-struct-predict-algorithm");
    void prepare();
    Task::ReportResult report();
private:
    SecStructPredictTask* task;
    QString algName;
    QString inputSeq;
    QString outputSeq;
};


class GTest_SecStructPredictTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_SecStructPredictTask, "predict-sec-structure-and-save-results");
    void prepare();
    void cleanup();
    Task::ReportResult report();
private:
    bool contextAdded;
    SecStructPredictTask* task;
    AnnotationTableObject* aObj;
    QString seqName;
    QString algName;
    QString resultsTableContextName;
};


class SecStructPredictTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};




} //namespace
#endif

