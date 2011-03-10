#ifndef _U2_KALIGN_TESTS_H_
#define _U2_KALIGN_TESTS_H_

#include "KalignTask.h"

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>

#include <QtXml/QDomElement>
#include <QtCore/QFileInfo>

namespace U2 {

class KalignGObjectTask;
class MAlignmentObject;
class LoadDocumentTask;
class MAlignment;

class  Kalign_Load_Align_Compare_Task : public Task {
	Q_OBJECT
public:
	Kalign_Load_Align_Compare_Task(QString inFileURL, QString patFileURL, KalignTaskSettings& config, 
		QString _name = QString("Kalign_Load_Align_Compare_Task"));
	~Kalign_Load_Align_Compare_Task() {cleanup();}
	void prepare();
	void run();
	QList<Task*> onSubTaskFinished(Task* subTask);
	ReportResult report();
	void cleanup();

	public slots:
		void sl_kalignProgressChg() {stateInfo.progress = kalignTask->getProgress();}

private:
	MAlignment dna_to_ma(QList<GObject*> dnaSeqs);
	QString str_inFileURL;
	QString str_patFileURL;
	LoadDocumentTask*           loadTask1;
	LoadDocumentTask*           loadTask2;
	Task*                       kalignTask; 
	KalignTaskSettings          config;
	MAlignmentObject*           ma1;
	MAlignmentObject*           ma2;
};

class GTest_Kalign_Load_Align_Compare: public GTest {
	Q_OBJECT
public:
	SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Kalign_Load_Align_Compare, "kalign-load-align-compare");
	~GTest_Kalign_Load_Align_Compare();
	void prepare();
	Task::ReportResult report();
private:
	QString inFileURL;
	QString patFileURL;
	Kalign_Load_Align_Compare_Task *worker;
};

class GTest_Kalign_Load_Align_QScore: public GTest {
	Q_OBJECT
public:
	SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Kalign_Load_Align_QScore, "kalign-load-align-qscore", TaskFlags_FOSCOE)
		//SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_Kalign_Load_Align_QScore, "kalign-load-align-qscore");
		~GTest_Kalign_Load_Align_QScore();
	void prepare();
	Task::ReportResult report();
	void run();
	QList<Task*> onSubTaskFinished(Task* subTask);
	MAlignment dna_to_ma(QList<GObject*> dnaSeqs);

	public slots:
		void sl_kalignProgressChg() {stateInfo.progress = kalignTask->getProgress();}

private:
	QString inFileURL;
	QString patFileURL;
	double qscore;
	double dqscore;
	LoadDocumentTask*           loadTask1;
	LoadDocumentTask*           loadTask2;
	Task*                       kalignTask; 
	KalignTaskSettings          config;
	MAlignmentObject*           ma1;
	MAlignmentObject*           ma2;
};

class KalignTests {
public:
	static QList<XMLTestFactory*> createTestFactories();
};
}//namespace
#endif

