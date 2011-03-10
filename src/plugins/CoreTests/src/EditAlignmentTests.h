#ifndef _U2_EDIT_ALIGNMENT_TESTS_H_
#define _U2_EDIT_ALIGNMENT_TESTS_H_

#include <U2Core/MAlignment.h>
#include <U2Core/GUrl.h>

#include <U2Test/GTest.h>
#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

#include <U2Algorithm/CreateSubalignmentTask.h>

namespace U2 {

class DNASequenceObject;

class GTest_CreateSubalignimentTask : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_CreateSubalignimentTask, "plugin_create-subalignment-task", TaskFlags_NR_FOSCOE);
    void prepare();
    Task::ReportResult report();   

private:
    MAlignmentObject *maobj;
    MAlignmentObject *expectedMaobj;
    QString docName;
    bool negativeTest;
    Task *t;
    QString expectedDocName;
    QStringList seqNames;
    U2Region window;
};

class GTest_RemoveAlignmentRegion : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_RemoveAlignmentRegion, "test-remove-alignment-region", TaskFlags_NR_FOSCOE);
    void prepare();
    Task::ReportResult report();   

private:
    MAlignmentObject *maobj;
    MAlignmentObject *expectedMaobj;
    QString docName;
    QString expectedDocName;
    int startBase, startSeq, width, height;
};


class GTest_AddSequenceToAlignment : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_AddSequenceToAlignment, "test-add-seq-to-alignment", TaskFlags_NR_FOSCOE);
    void prepare();
    Task::ReportResult report();   

private:
    MAlignmentObject *maobj;
    MAlignmentObject *expectedMaobj;
    QString docName;
    QString expectedDocName;
    QString seqFileName;
};

class CreateSubalignimentTests {
public:
    static QList< XMLTestFactory* > createTestFactories();
};


}

#endif //_U2_EDIT_ALIGNMENT_TESTS_H_
