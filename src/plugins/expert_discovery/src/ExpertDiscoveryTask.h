#pragma once

#include "DDisc/Extractor.h"
#include "ExpertDiscoveryData.h"
#include "ExpertDiscoveryCSUtil.h"

#include <U2Core/DNASequenceObject.h>
#include <U2Core/GObjectSelection.h>
#include <U2View/AnnotatedDNAView.h>

#include <U2Core/Task.h>
#include <U2Core/GObject.h>
#include <U2Gui/ObjectViewTasks.h>
#include <U2Core/GObjectReference.h>



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
        
}//namespace
