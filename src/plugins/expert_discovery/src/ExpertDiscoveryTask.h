#pragma once

#include "DDisc/Extractor.h"
#include "DDisc/Sequence.h"
#include "DDisc/MetaInfo.h"
#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryCSUtil.h"
#include "ExpertDiscoveryView.h"


#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectSelection.h>
#include <U2View/AnnotatedDNAView.h>

#include <U2Core/Task.h>
#include <U2Core/GObject.h>
#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/AnnotationData.h>
#include <U2Core/SequenceWalkerTask.h>
#include <U2Core/U2Region.h>

#include <QtCore/QMutex>

namespace U2 {

class Document;

class ExpertDiscoveryLoadPosNegTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadPosNegTask(QString firstF, QString secondF, bool generateNeg);
    ~ExpertDiscoveryLoadPosNegTask();

    void run(){};
    void prepare();

    QList<Document*> getDocuments() const {return docs;}

    bool isGenerateNeg() {return generateNeg;}

private:
    QString firstFile, secondFile;
    bool generateNeg;
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
    ~ExpertDiscoveryLoadControlTask();

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

private:
    QString firstFile, secondFile, thirdFile;
    bool generateDescr;
    bool appendToCurrent;
    bool nucleotidesMarkup;

    ExpertDiscoveryData& edData;
    Document* posDoc;
    Document* negDoc;

    bool loadAnnotationFromUgeneDocument(MarkingBase& base, const SequenceBase& seqBase, Document* doc);
    //Document* loadFile(QString inFile);

signals:
    void si_stateChanged(Task* task);
};

class ExpertDiscoveryLoadControlMrkTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryLoadControlMrkTask(QString firstF, ExpertDiscoveryData& edD );

    void run(){};
    void prepare();

private:
    QString firstFile;
    ExpertDiscoveryData& edData;

    //Document* loadFile(QString inFile);

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

class ExpertDiscoveryCreateADVTask: public Task{
    Q_OBJECT
public:
    ExpertDiscoveryCreateADVTask(const MultiGSelection& selObjects);

    AnnotatedDNAView* getView() {return adv;}

    void run();
    void prepare() {};
private:
    const MultiGSelection& multiSelection;
    AnnotatedDNAView* adv;

signals:
    void si_stateChanged(Task* task);
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
    ExpertDiscoveryToAnnotationTask(AnnotationTableObject* aobj, const DNASequence& seq, ExpertDiscoveryData* d, const EDProcessedSignal* ps, QMutex& mut);
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
    QList<SharedAnnotationData>             resultList;
    U2Region                                seqRange;
    QPointer<AnnotationTableObject>         aObj;
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

/**
 * Simple template task which allows to grab its result.
 * Intended to be used as a base class for tasks for BackgroundTaskRunner.
 */
template<class Result>
class BackgroundTask : public Task {
public:
    inline Result getResult() const {return result;};
protected:
    BackgroundTask(const QString& _name, TaskFlags f) : Task(_name, f){};
    Result result;
};


/**
 * Stub containing Q_OBJECT macro, signals&slots. Classes with signal/slot related
 * stuff can't be templates, so everything needed for BackgroundTaskRunner is moved here
 */
class BackgroundTaskRunner_base: public QObject {
    Q_OBJECT
public:
    virtual ~BackgroundTaskRunner_base(){};
    virtual void emitFinished(){emit(si_finished());};
signals:
    void si_finished();
private slots:
    virtual void sl_finished() = 0;
};

/**
 * Simple manager for background tasks. 
 * Allows running only one background task at a time, canceling previous task
 * when the new one is queued with run(). Emits si_finished() (defined in the base)
 * when the queued task is finished. Cancels current task in destructor.
 */
template<class Result>
class BackgroundTaskRunner : public BackgroundTaskRunner_base {
public:
    BackgroundTaskRunner() : task(0) {}

    virtual ~BackgroundTaskRunner() {
        if(task) {
            task->cancel();
        }
    }

    void run(BackgroundTask<Result> * newTask)  {
        if(task) {
            task->cancel();
        }
        task = newTask;
        connect(task, SIGNAL(si_stateChanged()), SLOT(sl_finished()));
        AppContext::getTaskScheduler()->registerTopLevelTask(task);
    }

    inline Result getResult() const {
        if(task) {
            return Result();
        }
        return result;
    }

    inline bool isFinished() {
        return !task;
    }

private:
    virtual void sl_finished() {
        BackgroundTask<Result> * senderr = dynamic_cast<BackgroundTask<Result>*>(sender());
        assert(senderr);
        if(task != senderr) {
            return;
        }
        if(Task::State_Finished != senderr->getState()) {
            return;
        }
        result = task->getResult();
        task = NULL;
        emitFinished();
    }

private:
    BackgroundTask<Result> * task;
    Result result;

private:
    BackgroundTaskRunner(const BackgroundTaskRunner &);
    BackgroundTaskRunner operator=(const BackgroundTaskRunner &);
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

    SharedAnnotationData toAnnotation(const QString& name) const {
        SharedAnnotationData data;
        data = new AnnotationData;
        data->name = name;
        data->location->regions << region;
        data->setStrand(strand);
        data->qualifiers.append(U2Qualifier("score", QString::number(score)));
        return data;
    }

    static QList<SharedAnnotationData> toTable(const QList<ExpertDiscoverySearchResult>& res, const QString& name)
    {
        QList<SharedAnnotationData> list;
        foreach (const ExpertDiscoverySearchResult& f, res) {
            list.append(f.toAnnotation(name));
        }
        return list;
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

        
}//namespace
