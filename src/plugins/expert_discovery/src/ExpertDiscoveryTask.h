#pragma once

#include "DDisc/Extractor.h"
#include "DDisc/Sequence.h"
#include "DDisc/MetaInfo.h"
#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryCSUtil.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectSelection.h>
#include <U2View/AnnotatedDNAView.h>

#include <U2Core/Task.h>
#include <U2Core/GObject.h>
#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/GObjectReference.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Core/AnnotationData.h>

#include <QMutex>

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

	QList<GObject*> sequencesGenerator(const QList<GObject*> &);
	QByteArray generateRandomSequence(const int* acgtContent, int seqLen);
	void calculateACGTContent(const DNASequenceObject& seq, int* acgtContent);

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
    ExpertDiscoveryLoadPosNegMrkTask(QString firstF, QString secondF, QString thirdF, bool generateDescr, bool appendToCurrentMrk, ExpertDiscoveryData& edData);

    void run(){};
    void prepare();
    ReportResult report();

private:
    QString firstFile, secondFile, thirdFile;
    bool generateDescr;
    bool appendToCurrent;

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

signals:
    void si_newSignalReady(DDisc::Signal* signal, CSFolder* folder);
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

class ExpertDiscoveryToAnnotationTask : public Task{
    Q_OBJECT
public:
    ExpertDiscoveryToAnnotationTask(AnnotationTableObject* aobj, const DNASequence& seq, ExpertDiscoveryData* d, const EDProcessedSignal* ps, QMutex& mut);
    void run();
    ReportResult report();
private:
    void recDataToAnnotation();
    void csToAnnotation(int seqNumberm, unsigned int seqLen);

    const DNASequence&                  dna;
    ExpertDiscoveryData*                edData;
    const EDProcessedSignal*            curPS;
    QList<SharedAnnotationData>         resultList;
    U2Region                            seqRange;
    QPointer<AnnotationTableObject>     aObj;
    RecognizationData                   recData;
    bool                                hasRecData;
    bool                                isControl;
    bool                                isPos;
    QMutex&                             mutex;
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
        
}//namespace
