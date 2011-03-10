#ifndef _U2_BOWTIE_TESTS_H_
#define _U2_BOWTIE_TESTS_H_

#include <U2Test/XMLTestUtils.h>
#include <U2Core/GObject.h>
#include <QtXml/QDomElement>
#include <QFileInfo>

#include "BowtieTask.h"

namespace U2 {

class BowtieGObjectTask;
class DnaAssemblyMultiTask;
class MAlignmentObject;
class LoadDocumentTask;
class MAlignment;

class GTest_Bowtie : public GTest {
    Q_OBJECT
public:
	SIMPLE_XML_TEST_BODY_WITH_FACTORY_EXT(GTest_Bowtie, "bowtie", TaskFlag_FailOnSubtaskCancel);
	~GTest_Bowtie();
    void prepare();
	void run();
    Task::ReportResult report();
    void cleanup();
    QString getTempDataDir();
	QList<Task*> onSubTaskFinished(Task* subTask);
	
private:    
	QList<DNASequence> dnaObjList_to_dnaList(QList<GObject*> dnaSeqs);
	void parseBowtieOutput(MAlignment& result, QString text);
	DnaAssemblyToRefTaskSettings config;
    QString readsFileName;
    GUrl readsFileUrl;
	QString indexName;
	QString patternFileName;
	QString negativeError;
	bool usePrebuildIndex;
	bool subTaskFailed;
	LoadDocumentTask* resultLoadTask;
	LoadDocumentTask* patternLoadTask;
	BowtieTask* bowtieTask;
	MAlignmentObject* ma1;
	MAlignmentObject* ma2;
	DocumentFormatId format;
	DocumentFormatId patternFormat;
};

class BowtieTests {
public:
	static QList<XMLTestFactory*> createTestFactories();
};
} //namespace U2

#endif //_U2_BOWTIE_TESTS_H_
