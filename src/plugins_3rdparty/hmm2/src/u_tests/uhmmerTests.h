#ifndef _U2_UHMMER_TESTS_H_
#define _U2_UHMMER_TESTS_H_

#include <U2Core/GObject.h>
#include <U2Test/XMLTestUtils.h>

#include <QtXml/QDomElement>

namespace U2 {

class Document;
class GTest_LoadDocument;
class HMMCalibrateToFileTask;
class HMMBuildToFileTask;
class CreateAnnotationModel;
struct plan7_s;

class GTest_uHMMERSearch : public GTest {
    Q_OBJECT
public:

    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uHMMERSearch, "uhmmer-search");

	~GTest_uHMMERSearch();
	void cleanup();
	ReportResult report();
	void prepare();

private:    
	Task* searchTask;
	Task* saveTask;
    QString		hmmFileName;
	QString		seqDocCtxName;
	QString		resultDocName;
	QString		resultDocContextName;
	bool expertOptions;
	int number_of_seq;
	int hmmSearchChunk;
	bool customHmmSearchChunk;
	bool parallel_flag;
	float evalueCutoff;
	float domEvalueCutoff;
	float minScoreCutoff;
	Document* aDoc;
	
protected:
	virtual QList<Task*> onSubTaskFinished(Task* subTask);
	
};

class GTest_uHMMERBuild: public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uHMMERBuild, "uhmmer-build");
    ReportResult report();
	void cleanup();
private:
	QString outFile;
	HMMBuildToFileTask *buildTask;
	bool deleteTempFile;
};

class GTest_hmmCompare: public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_hmmCompare, "hmm-compare");
    ReportResult report();
private:
	QString file1Name;
	QString file2Name;
};
class GTest_uHMMERCalibrate: public GTest {
    Q_OBJECT
    SIMPLE_XML_TEST_BODY_WITH_FACTORY(GTest_uHMMERCalibrate, "uhmmer-calibrate");
    ReportResult report();
	void cleanup();
private:
	class GTest_uHMMERCalibrateSubtask: public Task {
	public:
		GTest_uHMMERCalibrateSubtask(HMMCalibrateToFileTask **calibrateTask, int n);
	};
	HMMCalibrateToFileTask **calibrateTask;
	float mu;
	float lambda;
	int nCalibrates;
};

class UHMMERTests {
public:
    static QList<XMLTestFactory*> createTestFactories();
};
}//namespace
#endif
