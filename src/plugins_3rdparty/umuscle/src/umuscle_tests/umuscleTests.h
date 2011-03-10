#ifndef _U2_MUSCLE_TESTS_H_
#define _U2_MUSCLE_TESTS_H_

#include <U2Core/GObject.h>
#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>
#include <QtCore/QFileInfo>


#include "MuscleTask.h"

namespace U2 {

class MuscleGObjectTask;
class MAlignmentObject;
class LoadDocumentTask;
class MAlignment;

class GTest_uMuscle : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uMuscle, "umuscle");
    
    void prepare();
    Task::ReportResult report();
    void cleanup();

private:    
    QString inputDocCtxName;
    QString resultCtxName;
    Document* doc;
    MuscleGObjectTask* mTask;
    MAlignmentObject *ma_result;
    bool ctxAdded;
    bool refineOnly;
    int maxIters;
    bool alignRegion;
    U2Region region;
    bool stable;
};

class GTest_CompareMAlignment : public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_CompareMAlignment, "compare-malignment");

    Task::ReportResult report();

private:    
    QString doc1CtxName;
    QString doc2CtxName;
};

class GTest_uMuscleAddUnalignedSequenceToProfile: public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uMuscleAddUnalignedSequenceToProfile, "umuscle-add-unaligned-seq-to-profile");

    void prepare();
    Task::ReportResult report();

private:    
    QString                 aliDocName;
    QString                 seqDocName;
    int                     resultAliLen;
    QList< QList<int> >     gapPositionsForSeqs;
    MAlignmentObject*       aliObj;
    int                     origAliSeqs;
    int                     resultAliSeqs;
};

class GTest_Muscle_Load_Align_QScore: public GTest {
    Q_OBJECT
public:
	SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Muscle_Load_Align_QScore, "umuscle-load-align-qscore", TaskFlags_FOSCOE)
    //SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Muscle_Load_Align_QScore, "umuscle-load-align-qscore");
    ~GTest_Muscle_Load_Align_QScore();
    void prepare();
    Task::ReportResult report();
	void run();
	QList<Task*> onSubTaskFinished(Task* subTask);
	MAlignment dna_to_ma(QList<GObject*> dnaSeqs);

public slots:
		void sl_muscleProgressChg() {stateInfo.progress = muscleTask->getProgress();}

private:
    QString inFileURL;
    QString patFileURL;
	double qscore;
	double dqscore;
	LoadDocumentTask*           loadTask1;
	LoadDocumentTask*           loadTask2;
	Task*                       muscleTask; 
	MuscleTaskSettings          config;
	MAlignmentObject*           ma1;
	MAlignmentObject*           ma2;
};

class  Muscle_Load_Align_Compare_Task : public Task {
    Q_OBJECT
public:
    Muscle_Load_Align_Compare_Task(QString inFileURL, QString patFileURL, MuscleTaskSettings& config, 
        QString _name = QString("Muscle_Load_Align_Compare_Task"));
    ~Muscle_Load_Align_Compare_Task() {cleanup();}
    void prepare();
    void run();
    QList<Task*> onSubTaskFinished(Task* subTask);
    ReportResult report();
    void cleanup();

public slots:
    void sl_muscleProgressChg() {stateInfo.progress = muscleTask->getProgress();}

private:
    MAlignment dna_to_ma(QList<GObject*> dnaSeqs);
    QString str_inFileURL;
    QString str_patFileURL;
    LoadDocumentTask*           loadTask1;
    LoadDocumentTask*           loadTask2;
    Task*                       muscleTask; 
    MuscleTaskSettings          config;
    MAlignmentObject*           ma1;
    MAlignmentObject*           ma2;
};

class GTest_Muscle_Load_Align_Compare: public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Muscle_Load_Align_Compare, "umuscle-load-align-compare");
    ~GTest_Muscle_Load_Align_Compare();
    void prepare();
    Task::ReportResult report();
private:
    QString inFileURL;
    QString patFileURL;
    Muscle_Load_Align_Compare_Task *worker;
};

class GTest_uMusclePacketTest: public GTest {
    Q_OBJECT
public:
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uMusclePacketTest, "umuscle-packet-test");

    void prepare();
    Task::ReportResult report();

private:    
    QString                 inDirName;
    QString                 patDirName;
    QTime                   timer;
};

class UMUSCLETests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};
}//namespace
#endif
