#ifndef _GB2_QD_TESTS_H_
#define _GB2_QD_TESTS_H_

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>


namespace U2 {

class AnnotationTableObject;
class DNASequenceObject;
class QDScheduler;
class QDScheme;

//cppcheck-suppress noConstructor
class GTest_QDSchedulerTest : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_QDSchedulerTest, "qd_search", TaskFlags_NR_FOSCOE);
    ~GTest_QDSchedulerTest();

    virtual void prepare();
    virtual QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QString seqName;
    QString schemaUri;
    QString expectedResName;
    DNASequenceObject* seqObj;
    AnnotationTableObject* expectedResult;
    AnnotationTableObject* result;
    QDScheme* schema;
    QDScheduler* sched;
};

class QDTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};

class AnnotationGroup;

class CompareATObjectsTask : public Task {
    Q_OBJECT
public:
    CompareATObjectsTask(const QList<AnnotationGroup*>& _grp1,
        const QList<AnnotationGroup*>& _grp2)
        : Task(tr("Compare annotation tables task"), TaskFlag_None),
        grps1(_grp1), grps2(_grp2), equal(false) {}

    virtual void run();
    bool areEqual() const { assert(isFinished()); return equal; }
private:
    QList<AnnotationGroup*> grps1, grps2;
    bool equal;
};

}//namespace

#endif
