#ifndef _U2_EDIT_SEQUENCE_TESTS_H_
#define _U2_EDIT_SEQUENCE_TESTS_H_

#include <U2Core/global.h>
#include <U2Core/U2Region.h>
#include <U2Core/GObject.h>

#include <U2Core/DNASequenceObject.h>

#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

#include <U2Core/RemovePartFromSequenceTask.h>
#include <U2Core/AddPartToSequenceTask.h>
#include <U2Core/ReplacePartOfSequenceTask.h>

namespace U2 {

class GTest_RemovePartFromSequenceTask : GTest {
Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_RemovePartFromSequenceTask, "Remove_Part_From_Sequence_Task", TaskFlags_NR_FOSCOE);

    ~GTest_RemovePartFromSequenceTask();

    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    int                 length;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};

class GTest_AddPartToSequenceTask : GTest {
Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_AddPartToSequenceTask, "Add_Part_To_Sequence_Task", TaskFlags_NR_FOSCOE);

    ~GTest_AddPartToSequenceTask();
    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    QString             insertedSequence;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};

class GTest_ReplacePartOfSequenceTask : GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_ReplacePartOfSequenceTask, "replace-part-of-sequence", TaskFlags_NR_FOSCOE);

    ~GTest_ReplacePartOfSequenceTask() {};
    void prepare();
    Task::ReportResult report();
private:
    U2AnnotationUtils::AnnotationStrategyForResize strat;
    QString             docName;
    QString             seqName;
    int                 startPos;
    int                 length;
    QString             insertedSequence;
    QString             annotationName;
    QString             expectedSequence;
    QVector<U2Region>      expectedRegions;
    DNASequenceObject*  dnaso;
};


class EditSequenceTests {
public:
    static QList< XMLTestFactory* > createTestFactories();

};
}//ns
#endif
