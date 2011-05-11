#include "ExpertDiscoveryTask.h"

#include "ExpertDiscoveryExtSigWiz.h"

#include <U2Formats/DocumentFormatUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentFormatConfigurators.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <QtGui/QMessageBox>



#include <QtCore/QSet>



namespace U2 {

ExpertDiscoveryLoadPosNegTask::ExpertDiscoveryLoadPosNegTask(QString firstF, QString secondF, bool generateNeg)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
	firstFile = firstF;

	secondFile = secondF;

	this->generateNeg = generateNeg;
}

ExpertDiscoveryLoadPosNegTask::~ExpertDiscoveryLoadPosNegTask(){
	// error while loading documents
    if (hasErrors()) {
        Project *project = AppContext::getProject();

		// skip added to the project documents
		if (project) {
	        QList<Document*> projectDocs = project->getDocuments();

		    foreach (Document *doc, projectDocs) {
			    docs.removeAll(doc);
			}
		}

		// delete loaded but not added to the project documents
//		qDeleteAll(docs);
		foreach (Document *doc, docs) {
//			docs.removeAll(doc);
			delete doc;
		}
   }
}

void ExpertDiscoveryLoadPosNegTask::prepare(){
	// load sequences
	Document *doc = loadFile(firstFile);
	if (doc) {
		doc->setName("Positive");
		docs << doc;
	}

	if (hasErrors()) {
		return;
	}
	if(!generateNeg){
		doc = loadFile(secondFile);
		if (doc) {
			doc->setName("Negative");
			docs << doc;
		}
	}	
}


Document* ExpertDiscoveryLoadPosNegTask::loadFile(QString inFile){
	GUrl URL(inFile);

    Project *project = AppContext::getProject();

	Q_ASSERT(project);
    Document *doc = project->findDocumentByURL(URL);

	// document already present in the project
    if (doc) {
        return doc;
    }

    QList<DocumentFormat*> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first();
    Q_ASSERT(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(URL));

	doc = new Document(format, iof, URL, QList<UnloadedObjectInfo>());
    addSubTask(new AddDocumentTask(doc)); // add document to the project

	LoadUnloadedDocumentTask* ld = new LoadUnloadedDocumentTask(doc);
	if(generateNeg){
		connect(AppContext::getTaskScheduler(),SIGNAL(si_stateChanged(Task*)), SLOT(sl_generateNegativeSample(Task*)));
	}
	addSubTask(ld); // load document

    return doc;
}

void ExpertDiscoveryLoadPosNegTask::sl_generateNegativeSample(Task* task){
	LoadUnloadedDocumentTask *loadTask = qobject_cast<LoadUnloadedDocumentTask*>(task);
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }
	if (loadTask->getStateInfo().hasErrors()) {
		ExpertDiscoveryErrors::fileOpenError();
        return;
    }
	if(docs.isEmpty()){
		return;
	}
	Document* positiveDoc = docs.first();
	Document* negativeDoc;
	QString baseName = positiveDoc->getURL().baseFileName();
	baseName = baseName.append("_negative_generated");
	QString suffix = positiveDoc->getURL().completeFileSuffix();
	if(suffix!=""){
		suffix = QString(".").append(suffix);
	}
	baseName.append(suffix);
	QString negFileName = positiveDoc->getURL().dirPath().append("/"+baseName);
	GUrl URL(negFileName);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(URL));
	QList<GObject*> negObjects = sequencesGenerator(positiveDoc->getObjects());
	negativeDoc = new Document(positiveDoc->getDocumentFormat(),iof,URL,negObjects);

	negativeDoc->setLoaded(true);

	Project *project = AppContext::getProject();
	project->addDocument(negativeDoc);

	if (negativeDoc) {
		negativeDoc->setName("Negative");
		docs << negativeDoc;
	}
}

#define NUMBER_OF_NEGATIVE_PER_POSITIVE 10
QList<GObject*> ExpertDiscoveryLoadPosNegTask::sequencesGenerator(const QList<GObject*> &objects){
	QList<GObject*> neg;
	int acgtContent[4];

	foreach(GObject* obj, objects){
		if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
			DNASequenceObject* seq = (DNASequenceObject*)obj;
			calculateACGTContent(*seq,acgtContent);
			for (int i = 0; i < NUMBER_OF_NEGATIVE_PER_POSITIVE; i++){
				QByteArray curArr = generateRandomSequence(acgtContent, seq->getSequenceLen());
				QString name = seq->getGObjectName();
				name = name.append(QString("_neg%1").arg(i));
				DNASequence curSeq = DNASequence(seq->getDNASequence().getName().append(QString("_neg%1").arg(i)), curArr,seq->getAlphabet());
				DNASequenceObject* curSeqObj = new DNASequenceObject(name, curSeq);
				neg.append(curSeqObj);
			}
		}
	}

	return neg;
}

void ExpertDiscoveryLoadPosNegTask::calculateACGTContent(const DNASequenceObject& seq, int* acgtContent) {
    assert(seq.getAlphabet()->isNucleic());
    acgtContent[0] = acgtContent[1] = acgtContent[2] = acgtContent[3] = 0;
	int seqLen = seq.getSequenceLen();
    int total = seq.getSequenceLen();
	for (int i=0; i < seqLen; i++) {
		char c = seq.getSequence().at(i);
        if (c == 'A') {
			acgtContent[0]++;
        } else if (c == 'C') {
            acgtContent[1]++;
        } else if (c == 'G') {
            acgtContent[2]++;
        } else if (c == 'T') {
            acgtContent[3]++;
        } else {
            total--;
        }
    }

    for (int i=0;i<4;i++) {
        acgtContent[i] = qRound(acgtContent[i] * 100. / total);
    }
}


QByteArray ExpertDiscoveryLoadPosNegTask::generateRandomSequence(const int* acgtContent, int seqLen)  {

    QByteArray randomSequence;
    randomSequence.reserve(seqLen);

    int aPercentRange = acgtContent[0];
    int cPercentRange = aPercentRange + acgtContent[1];
    int gPercentRange = cPercentRange + acgtContent[2];

    assert(gPercentRange + acgtContent[3] > 0);

    for (int i=0; i < seqLen; i++) {
        int r = qrand();
        float perc = 100 * (float(r) / RAND_MAX);
        char c = 'T';
        if (perc <= aPercentRange) {
            c = 'A';
        } else if (perc <= cPercentRange) {
            c = 'C';
        } else if (perc <= gPercentRange) {
            c = 'G';
        }
        randomSequence.append(c);
    }
    return randomSequence;
}

ExpertDiscoveryLoadControlTask::ExpertDiscoveryLoadControlTask(QString firstF)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
	firstFile = firstF;
}

ExpertDiscoveryLoadControlTask::~ExpertDiscoveryLoadControlTask(){
	// error while loading documents
    if (hasErrors()) {
        Project *project = AppContext::getProject();

		// skip added to the project documents
		if (project) {
	        QList<Document*> projectDocs = project->getDocuments();

		    foreach (Document *doc, projectDocs) {
			    docs.removeAll(doc);
			}
		}

		// delete loaded but not added to the project documents
//		qDeleteAll(docs);
		foreach (Document *doc, docs) {
//			docs.removeAll(doc);
			delete doc;
		}
   }
}

void ExpertDiscoveryLoadControlTask::prepare(){
	// load sequences
	Document *doc = loadFile(firstFile);
	if (doc) {
		doc->setName("Control");
		docs << doc;
	}
}


Document* ExpertDiscoveryLoadControlTask::loadFile(QString inFile){
	GUrl URL(inFile);

    Project *project = AppContext::getProject();

	Q_ASSERT(project);
    Document *doc = project->findDocumentByURL(URL);

	// document already present in the project
    if (doc) {
        return doc;
    }

    QList<DocumentFormat*> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first();
    Q_ASSERT(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::url2io(URL));

	doc = new Document(format, iof, URL, QList<UnloadedObjectInfo>());
    addSubTask(new AddDocumentTask(doc)); // add document to the project
	addSubTask(new LoadUnloadedDocumentTask(doc)); // load document

    return doc;
}

void ExpertDiscoveryErrors::fileOpenError(const QString &filename) {

	QMessageBox mb(QMessageBox::Critical, tr("File opening error"), tr("Error opening file %1").arg(filename));
	mb.exec();
}

ExpertDiscoverySignalExtractorTask::ExpertDiscoverySignalExtractorTask(ExpertDiscoveryData* d)
: Task(tr("ExpertDiscovery signals extracting"), TaskFlags(TaskFlag_FailOnSubtaskCancel)){
	data = d;
	extractor = NULL;
}
ExpertDiscoverySignalExtractorTask::~ExpertDiscoverySignalExtractorTask(){
	delete extractor;
}

void ExpertDiscoverySignalExtractorTask::run(){
    //test
    folder = &data->getRootFolder();
        //performNextStep();
    //
	if(!extractor){
		return;
	}
	
	stateInfo.progress = 0;
//test
    Signal* ps = NULL;
    OpReiteration *op = new OpReiteration();
    op->setCount(Interval(1,2));
    op->setDistance(Interval(1,2));
    ps = new Signal(op,"olegSig","descrSig");
    emit si_newSignalReady(ps->clone(), folder);

    Signal* ps1 = NULL;
    OpReiteration *op1 = new OpReiteration();
    op->setCount(Interval(1,2));
    op->setDistance(Interval(1,2));
    ps1 = new Signal(op,"olegSig","descrSig1");
    emit si_newSignalReady(ps1->clone(), folder);
//test
	while(performNextStep()){
		if (stateInfo.cancelFlag) break;
		stateInfo.progress = short(extractor->progress() + 0.5);
	}

	stateInfo.progress = 100;

}
void ExpertDiscoverySignalExtractorTask::prepare(){
    //relocate
    data->markupLetters();

	ExpertDiscoveryExtSigWiz w(QApplication::activeWindow(), &data->getRootFolder());
	if(w.exec()){
		PredicatBase* predicatBase = new PredicatBase(data->getDescriptionBase());
		predicatBase->create(w.getPredicates());

		extractor = new Extractor(&data->getPosSeqBase(), &data->getNegSeqBase(), predicatBase);
		extractor->setFisherBound			   ( w.getFisher());
		extractor->setProbabilityBound	       ( w.getProbability());
		extractor->setInterestFisher		   ( w.getIntFisher());
		extractor->setInterestProbability      ( w.getIntProbability());
		extractor->setCoverageBound		       ( w.getCoverage()		   );
		extractor->setMaxComplexity		       ( w.getMaxComplexity()		   );
		extractor->setMinComplexity		       ( w.getMinComplexity()		   );
		extractor->setMinCorrelationOnPos      ( w.getMinPosCorrelation()	);
		extractor->setMaxCorrelationOnPos      ( w.getMaxPosCorrelation()	);
		extractor->setMinCorrelationOnNeg      ( w.getMinNegCorrelation()	);
		extractor->setMaxCorrelationOnNeg      ( w.getMaxNegCorrelation()	);
		extractor->setCorrelationImportant     ( w.getCorrelationImportant()  );
		extractor->setCheckFisherMinimization  ( w.getCheckFisherMinimization());
		extractor->setStoreOnlyDifferent       ( w.getStoreOnlyDifferent()    );
		extractor->setUmEnabled( w.getUmEnabled() );
		extractor->setUmSamplesBound( w.getUmSamplesBound() );
		extractor->setUmBound( w.getUmBound() );
	    folder = w.getFolder();
	}
}

bool ExpertDiscoverySignalExtractorTask::performNextStep(){
	Signal* pSignal = NULL;
    bool needOneMore = extractor->step(&pSignal);
	if (pSignal){
		//void* pointer = (void*)(pSignal->clone());
		//QVariant signal = qVariantFromValue<void*>(pointer);
		emit si_newSignalReady(pSignal->clone(), folder);
	}
    return needOneMore;

    //test
    /*Signal* ps = NULL;
    OpReiteration *op = new OpReiteration();
    op->setCount(Interval(1,2));
    op->setDistance(Interval(1,2));
    ps = new Signal(op,"olegSig","descrSig");
    emit si_newSignalReady(ps->clone(), folder);

    Signal* ps1 = NULL;
    OpReiteration *op1 = new OpReiteration();
    op->setCount(Interval(1,2));
    op->setDistance(Interval(1,2));
    ps1 = new Signal(op,"olegSig","descrSig1");
    emit si_newSignalReady(ps1->clone(), folder);

    return false;*/
}

ExpertDiscoveryCreateADVTask::ExpertDiscoveryCreateADVTask(const MultiGSelection& selObjects)
: Task(tr("ExpertDiscovery sequence view"), TaskFlags(TaskFlag_NoRun))
,adv(NULL)
,multiSelection(selObjects)
{
    
}


void ExpertDiscoveryCreateADVTask::run(){
//     QSet<GObject*> objectsToOpen = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &multiSelection, UOF_LoadedAndUnloaded);
// 
//     QSet<GObject*> selectedObjects = SelectionUtils::findObjects("", &multiSelection, UOF_LoadedAndUnloaded);
//     QList<GObject*> objectsWithSequenceRelation = GObjectUtils::selectObjectsWithRelation(selectedObjects.toList(), 
//         GObjectTypes::SEQUENCE, GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);
// 
//     objectsToOpen.unite(objectsWithSequenceRelation.toSet());
// 
//     const DocumentSelection* ds = qobject_cast<const DocumentSelection*>(multiSelection.findSelectionByType(GSelectionTypes::DOCUMENTS));
//     if (ds != NULL) {
//         foreach(Document* doc, ds->getSelectedDocuments()) {
//             objectsToOpen.unite(doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).toSet());
//             objectsToOpen.unite(GObjectUtils::selectObjectsWithRelation(doc->getObjects(), GObjectTypes::SEQUENCE, 
//                 GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true).toSet());
//         }
//     }

//     OpenAnnotatedDNAViewTask* task = new OpenAnnotatedDNAViewTask(objectsToOpen.toList());

}

ExpertDiscoveryCreateViewTask::ExpertDiscoveryCreateViewTask(const QList<GObject*>& objects) 
: ObjectViewTask("ED")
,adv(NULL)
{
    //  remember only sequence objects -> other added automatically
    //  load all objects

    QSet<Document*> docsToLoadSet;
    QSet<GObject*>  refsAdded;
    QList<GObject*> allSequenceObjects = GObjectUtils::findAllObjects(UOF_LoadedAndUnloaded, GObjectTypes::SEQUENCE);
    foreach(GObject* obj, objects) {
        uiLog.trace("Object to open sequence view: '" + obj->getGObjectName()+"'");
        Document* doc = obj->getDocument();
        if (!doc->isLoaded()) {
            docsToLoadSet.insert(doc);
        }
        if (GObjectUtils::hasType(obj, GObjectTypes::SEQUENCE)) {
            sequenceObjectRefs.append(GObjectReference(doc->getURLString(), obj->getGObjectName(), GObjectTypes::SEQUENCE));
            refsAdded.insert(obj);
            continue;
        }


        //look for sequence object using relations
        QList<GObject*> objWithSeqRelation = GObjectUtils::selectRelations(obj, GObjectTypes::SEQUENCE, 
            GObjectRelationRole::SEQUENCE, allSequenceObjects, UOF_LoadedAndUnloaded);

        foreach(GObject* robj, objWithSeqRelation) {
            if (!GObjectUtils::hasType(robj, GObjectTypes::SEQUENCE)) {
                continue;
            }
            if (refsAdded.contains(robj)) {
                continue;
            }
            Document* rdoc = robj->getDocument();
            if (!rdoc->isLoaded()) {
                docsToLoadSet.insert(rdoc);
            }
            refsAdded.insert(robj);
            sequenceObjectRefs.append(GObjectReference(rdoc->getURLString(), robj->getGObjectName(), GObjectTypes::SEQUENCE));

        }
    }
    foreach(Document* doc, docsToLoadSet) {
        uiLog.trace("Document to load: '" + doc->getURLString()+"'");
        documentsToLoad.append(doc);
    }
}

#define MAX_SEQ_OBJS_PER_VIEW 50


static QString deriveViewName(const QList<DNASequenceObject*>& seqObjects) {
    QString viewName;
    if (seqObjects.size() > 1) {
        bool singleDocument = true;
        Document* doc = seqObjects.first()->getDocument();
        foreach(GObject* obj, seqObjects) {
            if (doc != obj->getDocument()) {
                singleDocument = false;
                break;
            }
        }
        if (singleDocument) {
            viewName = GObjectViewUtils::genUniqueViewName(doc->getName());
        } else {
            viewName = GObjectViewUtils::genUniqueViewName(ExpertDiscoveryCreateViewTask::tr("ED"));
        }
    } else {
        GObject* obj = seqObjects.first();
        viewName = GObjectViewUtils::genUniqueViewName(obj->getDocument(), obj);
    }
    return viewName;
}

static bool objLessThan(const DNASequenceObject* o1 , const DNASequenceObject* o2) {
    if (o1->getDocument() == o2->getDocument()) {
        return o1->getGObjectName() < o2->getGObjectName();
    }
    return o1->getDocument()->getURLString() < o2->getDocument()->getURLString();
}


void ExpertDiscoveryCreateViewTask::open() {
    if (stateInfo.hasErrors() || sequenceObjectRefs.isEmpty()) {
        return;
    }
    QList<DNASequenceObject*> seqObjects;
    QList<GObject*> allSequenceObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::SEQUENCE);
    foreach(const GObjectReference& r, sequenceObjectRefs) {
        GObject* obj = GObjectUtils::selectObjectByReference(r, allSequenceObjects, UOF_LoadedOnly);
        DNASequenceObject* seqObj = qobject_cast<DNASequenceObject*>(obj);
        if (seqObj!=NULL) {
            seqObjects.append(seqObj);
            if (seqObjects.size() > MAX_SEQ_OBJS_PER_VIEW) {
                uiLog.details(tr("Maximum number of objects per view reached: %1").arg(MAX_SEQ_OBJS_PER_VIEW));
                break;
            }
        } else {
            uiLog.details(tr("Sequence object not available! URL %1, name %2").arg(r.docUrl).arg(r.objName));
        }
    }
    if (seqObjects.isEmpty()) { //object was removed asynchronously with the task
        stateInfo.setError(tr("No sequence objects found"));
        return;
    }
    qSort(seqObjects.begin(), seqObjects.end(), objLessThan);
    QString viewName = deriveViewName(seqObjects);
    AnnotatedDNAView* v = new AnnotatedDNAView(viewName, seqObjects);
    adv = v;
//     GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
//     MWMDIManager* mdiManager =  AppContext::getMainWindow()->getMDIManager();
//     mdiManager->addMDIWindow(w);

}

}//namespace
