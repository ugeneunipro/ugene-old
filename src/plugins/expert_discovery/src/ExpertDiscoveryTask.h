#pragma once

#include "DDisc/Extractor.h"
#include "DDisc/Sequence.h"
#include "DDisc/MetaInfo.h"
#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryCSUtil.h"
#include "ExpertDiscoveryView.h"

#include <U2Core/AnnotationData.h>
#include <U2Core/AppContext.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/BackgroundTaskRunner.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObject.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/U2Region.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/Task.h>

#include <U2Gui/ObjectViewTasks.h>

#include <U2View/AnnotatedDNAView.h>

#include <QtCore/QMutex>

namespace U2 {

class Document;

class ExpertDiscoveryLoadPosNegTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadPosNegTask(QString firstF, QString secondF, bool generateNeg, int negPerPositive);

    void run(){};
    void prepare();

    QList<Document*> getDocuments() const {return docs;}

    bool isGenerateNeg() {return generateNeg;}

private:
    QString firstFile, secondFile;
    bool generateNeg;
    int negPerPositive;
    QList<Document*> docs;

    Document* loadFile(QString inFile);

    QList<DNASequence> sequencesGenerator(const QList<GObject*> &);
    QByteArray generateRandomSequence(const int* acgtContent, int seqLen);
    void calculateACGTContent(const DNASequence& seq, int* acgtContent);

protected slots:
    void sl_generateNegativeSample(Task* task);

signals:
    void si_stateChanged(Task* task);
};

class ExpertDiscoveryLoadControlTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadControlTask(QString firstF);

    void run(){};
    void prepare();

    QList<Document*> getDocuments() const {return docs;}

private:
    QString firstFile;
    QList<Document*> docs;

    Document* loadFile(QString inFile);

signals:
    void si_stateChanged(Task* task);
};

class ExpertDiscoveryLoadPosNegMrkTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadPosNegMrkTask(QString firstF, QString secondF, QString thirdF, bool generateDescr, bool appendToCurrentMrk, bool isLettersMarkup, ExpertDiscoveryData& edData);

    void run();
    void prepare();
    ReportResult report();

    static bool loadAnnotationFromUgeneDocument(MarkingBase& base, const SequenceBase& seqBase, Document* doc);

private:
    QString firstFile, secondFile, thirdFile;
    bool generateDescr;
    bool appendToCurrent;
    bool nucleotidesMarkup;

    ExpertDiscoveryData& edData;
    Document* posDoc;
    Document* negDoc;

signals:
    void si_stateChanged(Task* task);
};

class ExpertDiscoveryLoadControlMrkTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadControlMrkTask(QString firstF, ExpertDiscoveryData& edD );

    void run(){};
    void prepare();
    ReportResult report();

private:
    QString firstFile;
    ExpertDiscoveryData& edData;
    Document* conDoc;

signals:
    void si_stateChanged(Task* task);
};

class ExpertDiscoverySignalExtractorTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoverySignalExtractorTask(ExpertDiscoveryData* d);
    ~ExpertDiscoverySignalExtractorTask();

    void run();
    void prepare();

private:
    DDisc::Extractor *extractor;
    CSFolder* folder;
    ExpertDiscoveryData* data;

    bool performNextStep();

protected slots:
    void sl_newFolder(const QString& folderName);

signals:
    void si_newSignalReady(DDisc::Signal* signal, CSFolder* folder);
    void si_newFolder(const QString& folderName);
};

// error messages and dialogs
class ExpertDiscoveryErrors: QObject {
    Q_OBJECT
public:
    enum Errors {ErrorOpen, NoErrors};

    static void fileOpenError(const QString &filename = "");
    static void markupLoadError();
};

class ExpertDiscoveryCreateViewTask: public ObjectViewTask{
    Q_OBJECT
public:
    //opens a single view for all sequence object in the list of sequence objects related to the objects in the list
    ExpertDiscoveryCreateViewTask(const QList<GObject*>& objects);

    virtual void open();
    AnnotatedDNAView* getADV(){return adv;}

private:
    QList<GObjectReference> sequenceObjectRefs;
    AnnotatedDNAView* adv;
};

class ExpertDiscoverySignalsAutoAnnotationUpdater : public AutoAnnotationsUpdater{
    Q_OBJECT
public:
    ExpertDiscoverySignalsAutoAnnotationUpdater();
    Task* createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa);
    bool checkConstraints(const AutoAnnotationConstraints& constraints);

    void setEDData(ExpertDiscoveryData* d){edData = d;}
    void setEDProcSignals(const EDProcessedSignal* ps){curPS = ps;}
    void setEDMutex(QMutex* mut){mutex = mut;}
private:
    ExpertDiscoveryData* edData;
    const EDProcessedSignal* curPS;
    QMutex* mutex;

};
class ExpertDiscoveryGetRecognitionDataTask :public Task{
    Q_OBJECT
public:
    ExpertDiscoveryGetRecognitionDataTask(ExpertDiscoveryData& data, RecognizationData& _recData, Sequence& seq);
    void run();
    bool hasRecData(){return isRecData;}
private:
    bool isRecData;
    Sequence curSequence;
    ExpertDiscoveryData& edData;
    RecognizationData& recData;

};

class ExpertDiscoveryToAnnotationTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoveryToAnnotationTask(FeaturesTableObject* aobj, const DNASequence& seq, ExpertDiscoveryData* d, const EDProcessedSignal* ps, QMutex& mut);
    void prepare();
    void run();
    ReportResult report();
private:
    QList<Task*> onSubTaskFinished(Task* subTask);
    void recDataToAnnotation();
    void csToAnnotation(int seqNumberm, unsigned int seqLen);

    QString                                 curDnaName;
    ExpertDiscoveryGetRecognitionDataTask*  recDataTask;
    const DNASequence&                      dna;
    ExpertDiscoveryData*                    edData;
    const EDProcessedSignal*                curPS;
    QList<AnnotationData>                   resultList;
    U2Region                                seqRange;
    QPointer<FeaturesTableObject>           aObj;
    RecognizationData                       recData;
    bool                                    hasRecData;
    bool                                    isControl;
    bool                                    isPos;
    QMutex&                                 mutex;
};

class ExpertDiscoveryUpdateSelectionTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoveryUpdateSelectionTask(ExpertDiscoveryView* currentView, QTreeWidgetItem* tItem);
    void run();
    ReportResult report();
private:
    AnnotatedDNAView*                   currentAdv;
    const EDProcessedSignal*            curPS;
    ExpertDiscoveryView*                view;
    QTreeWidgetItem*                    curretItem;
    bool                                updatePS;
    EDProjectItem*                      pItem;

    void updateAnnotations();

};

class ExpertDiscoverySaveDocumentTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoverySaveDocumentTask(ExpertDiscoveryData& data, const QString& fileName);

    void run();
private:
    ExpertDiscoveryData& edData;
    QString filename;
};

class ExpertDiscoveryLoadDocumentTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadDocumentTask(ExpertDiscoveryData& data, const QString& fileName);

    void run();
private:
    ExpertDiscoveryData& edData;
    QString filename;
};

class ExpertDiscoveryMarkupTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoveryMarkupTask(ExpertDiscoveryData& data);
    ExpertDiscoveryMarkupTask(ExpertDiscoveryData& data, const EDProcessedSignal* signal);

    void run();
private:
    ExpertDiscoveryData& edData;
    bool isLettersMarkup;
    const EDProcessedSignal* signal;
    QString curSignalName;

    void addSignalMarkup(SequenceBase& rBase, MarkingBase& rAnn, bool isPos);
};

struct ErrorsInfo{
    ErrorsInfo(): maxErrorVal(0), minErrorVal(0){;}

    QVector<double> errorFirstType;
    QVector<double> errorSecondType;
    QVector<double> score;
    double maxErrorVal;
    double minErrorVal;
};

struct CalculateErrorTaskInfo{
    CalculateErrorTaskInfo():scoreStep(0.1){}
    double scoreStep;
    U2Region scoreReg;
    QVector<double> posScore;
    QVector<double> negScore;
};

class ExpertDiscoveryCalculateErrors : public BackgroundTask<ErrorsInfo>{
public:
    ExpertDiscoveryCalculateErrors(const CalculateErrorTaskInfo& settings);
    void run();
private:
    CalculateErrorTaskInfo settings;
};   

//search
class ExpertDiscoverySearchResult {
public:
    ExpertDiscoverySearchResult() : strand(U2Strand::Direct), score(0){}

    AnnotationData toAnnotation(const QString& name) const {
        AnnotationData data;
        data.name = name;
        data.location->regions << region;
        data.setStrand(strand);
        data.qualifiers.append(U2Qualifier("score", QString::number(score)));
        return data;
    }

    U2Region region;
    U2Strand strand;
    float   score;
};

class ExpertDiscoverySearchCfg {
public:
    ExpertDiscoverySearchCfg() : minSCORE(0), complTT(NULL), complOnly(false) {}
    float minSCORE;
    DNATranslation* complTT;
    bool complOnly;
    U2Region searchRegion;
};

class ExpertDiscoverySearchTask : public Task, public SequenceWalkerCallback {
    Q_OBJECT
public:
    ExpertDiscoverySearchTask(ExpertDiscoveryData& data, const QByteArray& seq, const ExpertDiscoverySearchCfg& cfg, int resultsOffset);

    virtual void onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti);
    QList<ExpertDiscoverySearchResult> takeResults();

private:
    void addResult(const ExpertDiscoverySearchResult& r);

    QMutex                              lock;
    ExpertDiscoveryData&                edData;
    ExpertDiscoverySearchCfg            cfg;
    QList<ExpertDiscoverySearchResult>  results;
    int                                 resultsOffset;
    QByteArray                          wholeSeq;
    int                                 lenLeft;
    int                                 curLeft;
};

class ExpertDiscoveryExportSequences : public Task{
public:
    ExpertDiscoveryExportSequences(const SequenceBase& base);
    void prepare();
    void run();
private:
    const SequenceBase& base;
    QString fileName;
};

} // namespace
