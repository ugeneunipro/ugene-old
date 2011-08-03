#include "ExpertDiscoveryTask.h"

#include "ExpertDiscoveryExtSigWiz.h"
#include "ExpertDiscoveryPersistent.h"

//#include <U2Formats/DocumentFormatUtils.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentFormatConfigurators.h>
#include <U2Core/L10n.h>

#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>

#include <U2Core/Log.h>
#include <U2Core/L10n.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/BaseDocumentFormats.h>

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/GHints.h>

#include <QtGui/QMessageBox>

#include <QtCore/QSet>
#include <QtCore/QList>

#include <fstream>



namespace U2 {

ExpertDiscoveryLoadPosNegTask::ExpertDiscoveryLoadPosNegTask(QString firstF, QString secondF, bool generateNeg)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
    firstFile = firstF;

    secondFile = secondF;

    this->generateNeg = generateNeg;
}

ExpertDiscoveryLoadPosNegTask::~ExpertDiscoveryLoadPosNegTask(){
    // error while loading documents
    if (hasError()) {
        Project *project = AppContext::getProject();

        // skip added to the project documents
        if (project) {
            QList<Document*> projectDocs = project->getDocuments();

            foreach (Document *doc, projectDocs) {
                docs.removeAll(doc);
            }
        }

        // delete loaded but not added to the project documents
//        qDeleteAll(docs);
        foreach (Document *doc, docs) {
//            docs.removeAll(doc);
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

    if (hasError()) {
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

    //Project *project = AppContext::getProject();

    //Q_ASSERT(project);
    //Document *doc = project->findDocumentByURL(URL);

    // document already present in the project
//     if (doc) {
//         return doc;
//     }

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first().format;
    Q_ASSERT(format);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));

    Document* doc = new Document(format, iof, URL, QList<UnloadedObjectInfo>());
    //addSubTask(new AddDocumentTask(doc)); // add document to the project

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
    if (loadTask->getStateInfo().hasError()) {
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
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));
    QList<GObject*> negObjects = sequencesGenerator(positiveDoc->getObjects());
    negativeDoc = new Document(positiveDoc->getDocumentFormat(),iof,URL,negObjects);

    negativeDoc->setLoaded(true);

    Project *project = AppContext::getProject();
    //project->addDocument(negativeDoc);

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
    //assert(seq.getAlphabet()->isNucleic());
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

ExpertDiscoveryLoadPosNegMrkTask::ExpertDiscoveryLoadPosNegMrkTask(QString firstF, QString secondF, QString thirdF, bool generateDescr, bool appendToCurrentMrk, ExpertDiscoveryData& edD)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel |TaskFlag_FailOnSubtaskError))
,edData(edD)
,posDoc(NULL)
,negDoc(NULL)
{
    firstFile = firstF;

    secondFile = secondF;

    thirdFile = thirdF;

    this->generateDescr = generateDescr;

    appendToCurrent = appendToCurrentMrk;
}

void ExpertDiscoveryLoadPosNegMrkTask::prepare(){

    edData.clearScores();

    if(!appendToCurrent){
        edData.getPosMarkBase().clear();
        edData.getNegMarkBase().clear();
    }

    edData.getDescriptionBaseNoConst().clear();

    QString strPosName = firstFile;
    try {
        if (strPosName.right(4).compare(".xml", Qt::CaseInsensitive) == 0) {
            if (!edData.loadAnnotation(edData.getPosMarkBase(), edData.getPosSeqBase(), strPosName)) {
                throw std::exception();
            }
        } else  {
            QList<FormatDetectionResult> curFormats = DocumentUtils::detectFormat(firstFile);
            if (!curFormats.isEmpty()){
                if(curFormats.first().format->getFormatId() == BaseDocumentFormats::PLAIN_GENBANK){
                    GUrl URL(strPosName);
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));
                    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);

                    posDoc = new Document(f , iof, URL, QList<UnloadedObjectInfo>());
                    addSubTask(new LoadUnloadedDocumentTask(posDoc));

                } else {
                    ifstream fPosAnn(strPosName.toStdString().c_str());  
                    edData.getPosMarkBase().load(fPosAnn);
                }
            }
        }
    }
    catch (exception& ex) {
        edData.getPosMarkBase().clear();
        QString str = "Positive annotation: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        setError(str);
        return;
       // return false;
    }

    QString strNegName = secondFile;
    try {
        if (strNegName.right(4).compare(".xml", Qt::CaseInsensitive) == 0) {
            if (!edData.loadAnnotation(edData.getNegMarkBase(), edData.getNegSeqBase(), strNegName))
                throw std::exception();
        } else {
            QList<FormatDetectionResult> curFormats = DocumentUtils::detectFormat(strNegName);
            if (!curFormats.isEmpty()) {
                if(curFormats.first().format->getFormatId() == BaseDocumentFormats::PLAIN_GENBANK){
                    GUrl URL(strNegName);
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));
                    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);

                    negDoc = new Document(f, iof, URL, QList<UnloadedObjectInfo>());
                    addSubTask(new LoadUnloadedDocumentTask(negDoc));
                } else {
                    ifstream fNegAnn(strNegName.toStdString().c_str());
                    edData.getNegMarkBase().load(fNegAnn);
                }
            }
            
        }
    }
    catch (exception& ex) {
        edData.getPosMarkBase().clear();
        edData.getNegMarkBase().clear();
        QString str = "Negative annotation: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        setError(str);
        return;
       // return false;
    }

   

 //   return true;
}


Task::ReportResult ExpertDiscoveryLoadPosNegMrkTask::report(){
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }

    if(posDoc){
        try {
        if (!loadAnnotationFromUgeneDocument(edData.getPosMarkBase(), edData.getPosSeqBase(), posDoc))
            throw std::exception();
        }catch (exception& ex) {
            edData.getPosMarkBase().clear();
            QString str = "Positive annotation: ";
            str += ex.what();
            QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
            mb.exec();
            setError(str);
            return ReportResult_Finished;
        }
    }

    if(negDoc){
        try {
             if (!loadAnnotationFromUgeneDocument(edData.getNegMarkBase(), edData.getNegSeqBase(), negDoc))
                throw std::exception();
            }
            catch (exception& ex) {
                edData.getPosMarkBase().clear();
                edData.getNegMarkBase().clear();
                QString str = "Negative annotation: ";
                str += ex.what();
                QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
                mb.exec();
                setError(str);
                return ReportResult_Finished;
            }
    }

    try {
        if (generateDescr) {
            if (!edData.generateDescription(!appendToCurrent))
                throw std::exception();
        }
        else {
            ifstream fDesc( thirdFile.toStdString().c_str() );
            edData.getDescriptionBaseNoConst().load(fDesc);
        }
    }
    catch (exception& ex) {
        edData.getPosMarkBase().clear();
        edData.getNegMarkBase().clear();
        edData.getDescriptionBaseNoConst().clear();
        QString str = "Description: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        setError(str);
        return ReportResult_Finished;
        //     return false;
    }

    edData.getPosSeqBase().setMarking(edData.getPosMarkBase());
    edData.getNegSeqBase().setMarking(edData.getNegMarkBase());

    return ReportResult_Finished;
}

bool ExpertDiscoveryLoadPosNegMrkTask::loadAnnotationFromUgeneDocument(MarkingBase& base, const SequenceBase& seqBase, Document* doc){
    QList<GObject*> seqList = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    QList<GObject*> allSeqAnnotations = doc->findGObjectByType(GObjectTypes::ANNOTATION_TABLE);

    foreach(GObject* seqObj, seqList){

        int objN = seqBase.getObjNo(seqObj->getGObjectName().toStdString().c_str());
        if(objN < 0){
            QString sequenceId = seqObj->getGObjectName();
            int cutPos = sequenceId.indexOf("sequence");
            if(cutPos >= 0){
                sequenceId = sequenceId.left(cutPos);
            }
            sequenceId = sequenceId.trimmed();
            objN = seqBase.getObjNo(sequenceId.toStdString().c_str());
        }
        if(objN >= 0){
           Marking mrk;
           try {
               mrk = base.getMarking(objN);
           }catch(...){}
       
            QList<GObject*> annotations = GObjectUtils::findObjectsRelatedToObjectByRole(seqObj, 
                GObjectTypes::ANNOTATION_TABLE, GObjectRelationRole::SEQUENCE, 
                allSeqAnnotations, UOF_LoadedOnly);

            foreach(GObject* ao, annotations) {
                AnnotationTableObject* atobj = qobject_cast<AnnotationTableObject*>(ao);
                if(atobj){
                    const QList<Annotation*>& annotations =  atobj->getAnnotations();
                    foreach(Annotation* a, annotations){
                       const QVector<U2Region>& regions =  a->getRegions();
                       foreach(U2Region reg, regions){
                           reg.length;
                           int startPos = reg.startPos;
                           int endPos = reg.startPos + reg.length - 1;
                           if (endPos >= startPos && startPos >= 0) {
                               //mrk.set(signalName.toStdString(), familyName.toStdString(), DDisc::Interval(startPos, endPos));
                               mrk.set(a->getAnnotationName().toStdString(), "UGENE Annotation", DDisc::Interval(startPos, endPos));
                           }
                       }
                           
                    }
                }
             }
             base.setMarking(objN, mrk);
        }

    }

    return true;
}


ExpertDiscoveryLoadControlMrkTask::ExpertDiscoveryLoadControlMrkTask(QString firstF, ExpertDiscoveryData& edD)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel))
,edData(edD){
    firstFile = firstF;
}

void ExpertDiscoveryLoadControlMrkTask::prepare(){
    edData.loadControlSequenceAnnotation(firstFile);
}


ExpertDiscoveryLoadControlTask::ExpertDiscoveryLoadControlTask(QString firstF)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
    firstFile = firstF;
}

ExpertDiscoveryLoadControlTask::~ExpertDiscoveryLoadControlTask(){
    // error while loading documents
    if (hasError()) {
        Project *project = AppContext::getProject();

        // skip added to the project documents
        if (project) {
            QList<Document*> projectDocs = project->getDocuments();

            foreach (Document *doc, projectDocs) {
                docs.removeAll(doc);
            }
        }

        // delete loaded but not added to the project documents
//        qDeleteAll(docs);
        foreach (Document *doc, docs) {
//            docs.removeAll(doc);
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

//     Project *project = AppContext::getProject();
// 
//     Q_ASSERT(project);
//     Document *doc = project->findDocumentByURL(URL);
// 
//     // document already present in the project
//     if (doc) {
//         return doc;
//     }

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first().format;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(URL));

    Document* doc = new Document(format, iof, URL, QList<UnloadedObjectInfo>());
    //addSubTask(new AddDocumentTask(doc)); // add document to the project
    addSubTask(new LoadUnloadedDocumentTask(doc)); // load document

    return doc;
}

void ExpertDiscoveryErrors::fileOpenError(const QString &filename) {

    QMessageBox mb(QMessageBox::Critical, tr("File opening error"), tr("Error opening file %1").arg(filename));
    mb.exec();
}

void ExpertDiscoveryErrors::markupLoadError() {

    QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("Error loading markups"));
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
    //folder = &data->getRootFolder();
        //performNextStep();
    //
    if(!extractor){
        return;
    }

    stateInfo.progress = 0;
//test
//     Signal* ps = NULL;
//     OpReiteration *op = new OpReiteration();
//     op->setCount(Interval(1,2));
//     op->setDistance(Interval(1,2));
//     ps = new Signal(op,"olegSig","descrSig");
//     emit si_newSignalReady(ps->clone(), folder);
// 
//     Signal* ps1 = NULL;
//     OpReiteration *op1 = new OpReiteration();
//     op->setCount(Interval(1,2));
//     op->setDistance(Interval(1,2));
//     ps1 = new Signal(op,"olegSig","descrSig1");
//     emit si_newSignalReady(ps1->clone(), folder);
//test
    while(performNextStep()){
        if (stateInfo.cancelFlag) break;
        stateInfo.progress = short(extractor->progress() + 0.5);
    }

    stateInfo.progress = 100;

}
void ExpertDiscoverySignalExtractorTask::prepare(){
    //relocate
    //data->markupLetters();

    ExpertDiscoveryExtSigWiz w(QApplication::activeWindow(), &data->getRootFolder(), data->getMaxPosSequenceLen());
    if(w.exec()){
        PredicatBase* predicatBase = new PredicatBase(data->getDescriptionBase());
        predicatBase->create(w.getPredicates());

        extractor = new Extractor(&data->getPosSeqBase(), &data->getNegSeqBase(), predicatBase);
        extractor->setFisherBound ( w.getFisher());
        extractor->setProbabilityBound ( w.getProbability());
        extractor->setInterestFisher ( w.getIntFisher());
        extractor->setInterestProbability ( w.getIntProbability());
        extractor->setCoverageBound ( w.getCoverage());
        extractor->setMaxComplexity ( w.getMaxComplexity());
        extractor->setMinComplexity ( w.getMinComplexity());
        extractor->setMinCorrelationOnPos ( w.getMinPosCorrelation());
        extractor->setMaxCorrelationOnPos ( w.getMaxPosCorrelation());
        extractor->setMinCorrelationOnNeg ( w.getMinNegCorrelation());
        extractor->setMaxCorrelationOnNeg ( w.getMaxNegCorrelation());
        extractor->setCorrelationImportant ( w.getCorrelationImportant());
        extractor->setCheckFisherMinimization ( w.getCheckFisherMinimization());
        extractor->setStoreOnlyDifferent ( w.getStoreOnlyDifferent());
        extractor->setUmEnabled( w.getUmEnabled());
        extractor->setUmSamplesBound( w.getUmSamplesBound());
        extractor->setUmBound( w.getUmBound());
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
    if (stateInfo.hasError() || sequenceObjectRefs.isEmpty()) {
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

ExpertDiscoverySignalsAutoAnnotationUpdater::ExpertDiscoverySignalsAutoAnnotationUpdater()
:AutoAnnotationsUpdater(tr("Signals"), "ExpertDiscover Signals")
,curPS(NULL)
,edData(NULL)
,mutex(NULL)
{

}
Task* ExpertDiscoverySignalsAutoAnnotationUpdater::createAutoAnnotationsUpdateTask(const AutoAnnotationObject* aa){
    if(!edData){
        return NULL;
    }

    AnnotationTableObject* aObj = aa->getAnnotationObject();
    const DNASequence& dna = aa->getSeqObject()->getDNASequence();
    Task* task = new ExpertDiscoveryToAnnotationTask(aObj, dna, edData, curPS, *mutex);
    return task;
}
bool ExpertDiscoverySignalsAutoAnnotationUpdater::checkConstraints(const AutoAnnotationConstraints& constraints){
    if (constraints.alphabet == NULL) {
        return false;
    }

    bool edEnabled = false;
    if(constraints.hints != NULL){
        QVariant hint = constraints.hints->get("EDHint");
        if(hint.isNull()){
            return false;
        }
        edEnabled = hint.toBool();
    }

    return constraints.alphabet->isNucleic() && edEnabled;
}

ExpertDiscoveryToAnnotationTask::ExpertDiscoveryToAnnotationTask(AnnotationTableObject* aobj, const DNASequence& seq, ExpertDiscoveryData* d, const EDProcessedSignal* ps, QMutex& mut)
:Task(tr("Find and store expert discovery signals on a sequence"), TaskFlags_FOSCOE), dna(seq), edData(d), aObj(aobj), curPS(ps), mutex(mut){

    seqRange = U2Region(0, seq.length());
}

void ExpertDiscoveryToAnnotationTask::run(){
    if (isCanceled() || hasError()) {
        return;
    }

    if (aObj.isNull()) {
        stateInfo.setError(  tr("Annotation table does not exist") );
        return;
    }

    if (aObj->isStateLocked()) {
        stateInfo.setError(  tr("Annotation table is read-only") );
        return;
    }

    assert(edData);
    int seqNumber = -1;
    DDisc::Sequence edSeq;
    seqNumber = edData->getPosSeqBase().getObjNo(dna.getName().toStdString().c_str());
    if(seqNumber == -1){
        seqNumber = edData->getNegSeqBase().getObjNo(dna.getName().toStdString().c_str());
        if(seqNumber == -1){
             seqNumber = edData->getConSeqBase().getObjNo(dna.getName().toStdString().c_str());
             if(seqNumber == -1){
                 stateInfo.setError(tr("No expert discovery sequence"));
                 return;
             }else{
                 edSeq = edData->getConSeqBase().getSequence(seqNumber); 
                 isControl = true;
             }
        }else{
            edSeq = edData->getNegSeqBase().getSequence(seqNumber);
            isControl = false;
            isPos = false;
        }
    }
    else{
        edSeq = edData->getPosSeqBase().getSequence(seqNumber);
        isControl = false;
        isPos = true;
    }

    //cs start
   // mutex.lock();
    csToAnnotation(seqNumber, edSeq.getSize());
    
    hasRecData = edData->recDataStorage.getRecognizationData(recData, &edSeq, edData->getSelectedSignalsContainer());
    if(hasRecData){
        recDataToAnnotation();    
    }
   // mutex.unlock();
    //cs end
    
}

Task::ReportResult ExpertDiscoveryToAnnotationTask::report(){
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }

    if (aObj->isStateLocked()) {
        setError(tr("Annotation obj %1 is locked for modifications").arg(aObj->getGObjectName()));
        return ReportResult_Finished;
    }

    QList<Annotation*> annotations;
    foreach (const SharedAnnotationData& data, resultList) {      
        annotations.append(new Annotation(data));
    }

    aObj->addAnnotations(annotations, "ExpertDiscover Signals");
    
    return ReportResult_Finished;
}

void ExpertDiscoveryToAnnotationTask::csToAnnotation(int seqNumber, unsigned int seqLen){
    assert(seqNumber!=-1);

    if(isControl || !curPS){
        return;
    }

    //EDProcessedSignal curPsCopy = *curPS;
    if(isPos){
        if(curPS->getYesSequenceNumber() <= seqNumber){
            return;
        }
    }else{
        if(curPS->getNoSequenceNumber() <= seqNumber){
            return;
        }
    }

    const Set& set = isPos? curPS->getYesRealizations(seqNumber) : curPS->getNoRealizations(seqNumber);


   
    unsigned int i = 0;
    unsigned int j = 0;
    QString first_data = "";
    QString second_data = "";
    while(i < seqLen){
        first_data = "";
        if(set.is_set(i)){
            first_data = QString::fromStdString(set.association(i));
        }

        j = i+1;
        while(j < seqLen){
            second_data = "";
            if(set.is_set(j)){
                second_data = QString::fromStdString(set.association(i));
            }
            if(first_data != second_data || second_data.isEmpty()){
                break;
            }
            j++;
        }
        if(!first_data.isEmpty()){
            SharedAnnotationData data;
            data = new AnnotationData;
            data->name = "signal";
            data->location->regions << U2Region(i,j-i);
            data->qualifiers.append(U2Qualifier("name", first_data));
            resultList.append(data);
        }

        i = j;
    }
//     QMap<QString, QList<U2Region>> sigMap;
//     QList<U2Region> regions;
//     for(unsigned int i = 0; i <seqLen; i++){
//         if(set.is_set(i)){
//             QString sigName = QString::fromStdString(set.association(i));
//             if(!sigMap.contains(sigName)){
//                 QList<U2Region> emptyList;
//                 sigMap.insert(sigName, emptyList);
//                 
//             }
// 
//             sigMap[sigName].append(U2Region(i,1));
//             //regions.append(U2Region(i,1));
//            
//             //data->location->regions << U2Region(i,1);  
//         }
//     }
//     foreach(QString name, sigMap.keys()){
//        // QString sigName = QString::fromStdString(set.association(i));
//         SharedAnnotationData data;
//         data = new AnnotationData;
//         data->name = "signal";
//         const QList<U2Region>& regions = sigMap.value(name);
//         foreach(U2Region reg, regions){
//             data->location->regions << reg;
//         }
//         
//         data->qualifiers.append(U2Qualifier("name", name));
//         resultList.append(data);
//     }
}

void ExpertDiscoveryToAnnotationTask::recDataToAnnotation(){

    unsigned int recSize= recData.size();
    unsigned int i = 0;
    unsigned int j = 0;
    double first_rec_data = 0;
    double second_rec_data = 0;
    while(i < recSize){
        first_rec_data = recData[i];

        j = i+1;
        while(j < recSize){
            second_rec_data = recData[j];
            if(first_rec_data != second_rec_data){
                break;
            }
            j++;
        }
        if(first_rec_data!=0){
            SharedAnnotationData data;
            data = new AnnotationData;
            data->name = "rec.data";
            data->location->regions << U2Region(i,j-i);
            data->qualifiers.append(U2Qualifier("criteria", QString::number(first_rec_data)));
            resultList.append(data);
        }

        i = j;
    }
  
//     SharedAnnotationData data;
//     data = new AnnotationData;
//     data->name = "rec.data";
//     data->location->regions << U2Region(1,3);
//     data->qualifiers.append(U2Qualifier("ed_rec_criteria", "3"));
// 
//     return data;
}


ExpertDiscoverySaveDocumentTask::ExpertDiscoverySaveDocumentTask(ExpertDiscoveryData& data, const QString& fileName)
:Task("Save ExpertDiscovery document task", TaskFlag_None)
,edData(data)
,filename(fileName)
{
    
}

void ExpertDiscoverySaveDocumentTask::run(){
    if(hasError()) {
        return;
    }

    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly)) {
        setError(L10N::errorOpeningFileWrite(filename));
        return;
    }
    
    QDataStream out(&file);

    //out.setByteOrder(QDataStream::LittleEndian);

    EDPMCSFolder::save(out, &edData.getRootFolder());

    out << edData.getRecognizationBound();
    out << false;

    EDPMSeqBase::save(out, edData.getPosSeqBase());
    EDPMSeqBase::save(out,edData.getNegSeqBase());
    EDPMSeqBase::save(out,edData.getConSeqBase());
    EDPMMrkBase::save(out, edData.getPosMarkBase(), edData.getPosSeqBase().getSize());
    EDPMMrkBase::save(out, edData.getNegMarkBase(), edData.getNegSeqBase().getSize());
    EDPMMrkBase::save(out,edData.getConMarkBase(), edData.getConSeqBase().getSize());
    EDPMDescription::save(out, edData.getDescriptionBaseNoConst());

    edData.getPosSeqBase().setMarking(edData.getPosMarkBase());
    edData.getNegSeqBase().setMarking(edData.getNegMarkBase());
    edData.getConSeqBase().setMarking(edData.getConMarkBase());
   
    edData.getSelectedSignalsContainer().save(out, edData.getRootFolder());

    out << false << 1;  //LargeSequeceMode (for compatibility with the old version)

    edData.setModifed(false);

}

ExpertDiscoveryLoadDocumentTask::ExpertDiscoveryLoadDocumentTask(ExpertDiscoveryData& data, const QString& fileName)
:Task("Load ExpertDiscovery document task", TaskFlag_None)
,edData(data)
,filename(fileName)
{

}

void ExpertDiscoveryLoadDocumentTask::run(){
    if(hasError()) {
        return;
    }

    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)) {
        setError(L10N::errorOpeningFileRead(filename));
        return;
    }

    QDataStream inStream(&file);

    EDPMCSFolder::load(inStream, &edData.getRootFolder());

    bool m_bOptimizeRecogniztionBound;
    double m_dRecognizationBound;

    inStream >> m_dRecognizationBound;
    inStream >> m_bOptimizeRecogniztionBound;

    edData.setRecBound(m_dRecognizationBound);

    EDPMSeqBase::load(inStream, edData.getPosSeqBase());
    EDPMSeqBase::load(inStream, edData.getNegSeqBase());
    EDPMSeqBase::load(inStream, edData.getConSeqBase());
    EDPMMrkBase::load(inStream, edData.getPosMarkBase(), edData.getPosSeqBase().getSize());
    EDPMMrkBase::load(inStream, edData.getNegMarkBase(), edData.getNegSeqBase().getSize());
    EDPMMrkBase::load(inStream, edData.getConMarkBase(), edData.getConSeqBase().getSize());
    EDPMDescription::load(inStream, edData.getDescriptionBaseNoConst());

    edData.getPosSeqBase().setMarking(edData.getPosMarkBase());
    edData.getNegSeqBase().setMarking(edData.getNegMarkBase());
    edData.getConSeqBase().setMarking(edData.getConMarkBase());

    edData.getSelectedSignalsContainer().load(inStream, edData.getRootFolder());
}

}//namespace
