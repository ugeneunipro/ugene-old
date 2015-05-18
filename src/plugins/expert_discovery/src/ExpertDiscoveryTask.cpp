#include <fstream>
#include <limits>

#include <QtCore/QList>
#include <QtCore/QSet>

#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#else
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMessageBox>
#endif

#include <U2Core/AddDocumentTask.h>
#include <U2Core/AnnotationTableObject.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/Counter.h>
#include <U2Core/DNAAlphabet.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/GHints.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/L10n.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/MAlignment.h>
#include <U2Core/MAlignmentImporter.h>
#include <U2Core/MAlignmentObject.h>
#include <U2Core/MSAUtils.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/U2SafePoints.h>
#include <U2Core/U2SequenceUtils.h>
#include <U2Core/U2OpStatusUtils.h>

#include <U2Gui/DialogUtils.h>
#include <U2Gui/U2FileDialog.h>

#include <U2View/AutoAnnotationUtils.h>

#include "ExpertDiscoveryTask.h"
#include "ExpertDiscoveryExtSigWiz.h"
#include "ExpertDiscoveryPersistent.h"

namespace U2 {

ExpertDiscoveryLoadPosNegTask::ExpertDiscoveryLoadPosNegTask(QString firstF, QString secondF, bool generateNeg, int _negPerPositive)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
    firstFile = firstF;
    secondFile = secondF;
    this->generateNeg = generateNeg;
    negPerPositive = _negPerPositive;
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

Document * ExpertDiscoveryLoadPosNegTask::loadFile(QString inFile){
    GUrl url(inFile);

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first().format;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));

    Document* doc = format->createNewUnloadedDocument(iof, url, stateInfo);
    CHECK_OP(stateInfo, NULL);

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
    GUrl url(negFileName);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));

    negativeDoc = positiveDoc->getDocumentFormat()->createNewLoadedDocument(iof, url, stateInfo);
    CHECK_OP(stateInfo, );
    const QList<GObject*>& posObjects = positiveDoc->getObjects();
    bool isMsa = false;
    if(!posObjects.isEmpty()){
        if (posObjects.first()->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT){
            isMsa = true;
        }
    }
    QList<DNASequence> negObjects = sequencesGenerator(posObjects);
    if(!isMsa){
        foreach(const DNASequence& dnaSeq, negObjects) {
            U2EntityRef seqRef = U2SequenceUtils::import(negativeDoc->getDbiRef(), dnaSeq, stateInfo);
            CHECK_OP(stateInfo, );
            negativeDoc->addObject(new U2SequenceObject(dnaSeq.getName(), seqRef));
        }
    }else{
        MAlignment msa = MSAUtils::seq2ma(negObjects, stateInfo);
        CHECK_OP(stateInfo, );

        U2EntityRef msaRef = MAlignmentImporter::createAlignment(negativeDoc->getDbiRef(), msa, stateInfo);
        CHECK_OP(stateInfo, );

        MAlignmentObject* obj = new MAlignmentObject(msa.getName(), msaRef);

        negativeDoc->addObject(obj);
    }

    if (negativeDoc) {
        negativeDoc->setName("Negative");
        docs << negativeDoc;
    }
}

//#define NUMBER_OF_NEGATIVE_PER_POSITIVE 10
QList<DNASequence> ExpertDiscoveryLoadPosNegTask::sequencesGenerator(const QList<GObject*> &objects){
    QList<DNASequence> neg;
    int acgtContent[4];

    foreach(GObject* obj, objects){
        if(obj->getGObjectType() == GObjectTypes::SEQUENCE){
            U2SequenceObject* seq = (U2SequenceObject*)obj;
            U2OpStatusImpl os;
            DNASequence dnaSequence = seq->getWholeSequence(os);
            CHECK_OP(os, neg);
            calculateACGTContent( dnaSequence, acgtContent);
            for (int i = 0; i < negPerPositive; i++){
                QByteArray curArr = generateRandomSequence(acgtContent, seq->getSequenceLength());
                QString name = seq->getGObjectName();
                name = name.append(QString("_neg%1").arg(i));
                DNASequence curSeq = DNASequence(seq->getSequenceName().append(QString("_neg%1").arg(i)), curArr,seq->getAlphabet());
                neg << curSeq;
            }
        }else if(obj->getGObjectType() == GObjectTypes::MULTIPLE_ALIGNMENT){
            MAlignmentObject* mobj =  qobject_cast<MAlignmentObject*>(obj);
            if(mobj){
                const MAlignment& ma = mobj->getMAlignment();
                QList<DNASequence> sequences = MSAUtils::ma2seq(ma, false);
                foreach(DNASequence seq, sequences){
                    calculateACGTContent(seq,acgtContent);
                    for (int i = 0; i < negPerPositive; i++){
                        QByteArray curArr = generateRandomSequence(acgtContent, seq.length());
                        QString name = seq.getName();
                        name = name.append(QString("_neg%1").arg(i));
                        DNASequence curSeq = DNASequence(seq.getName().append(QString("_neg%1").arg(i)), curArr,seq.alphabet);
                        neg << curSeq;
                    }
                }
            }
        }
    }

    return neg;
}

void ExpertDiscoveryLoadPosNegTask::calculateACGTContent(const DNASequence& seq, int* acgtContent) {
    acgtContent[0] = acgtContent[1] = acgtContent[2] = acgtContent[3] = 0;
    int seqLen = seq.length();
    int total = seq.length();
    QByteArray seqArr = seq.constSequence();
    for (int i=0; i < seqLen; i++) {
        char c = seqArr.at(i);
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

ExpertDiscoveryLoadPosNegMrkTask::ExpertDiscoveryLoadPosNegMrkTask(QString firstF, QString secondF, QString thirdF, bool generateDescr, bool appendToCurrentMrk,  bool isLettersMarkup, ExpertDiscoveryData& edD)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_FailOnSubtaskCancel |TaskFlag_FailOnSubtaskError))
,edData(edD)
,posDoc(NULL)
,negDoc(NULL)
{
    firstFile = firstF;
    secondFile = secondF;
    thirdFile = thirdF;
    this->generateDescr = generateDescr;
    appendToCurrent = appendToCurrentMrk;
    nucleotidesMarkup = isLettersMarkup;
}

void ExpertDiscoveryLoadPosNegMrkTask::prepare(){
    if(nucleotidesMarkup){
        return;
    }
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
                    GUrl url(strPosName);
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
                    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);

                    posDoc = f->createNewUnloadedDocument(iof, url, stateInfo);
                    CHECK_OP(stateInfo, );
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
    }

    QString strNegName = secondFile;
    if(strNegName.isEmpty()){
        return;
    }
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

                    negDoc = f->createNewUnloadedDocument(iof, URL, stateInfo);
                    CHECK_OP(stateInfo,);
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
    }
}

void ExpertDiscoveryLoadPosNegMrkTask::run(){
    if(nucleotidesMarkup){
        edData.markupLetters();
    }
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
           } catch (...) { }

            QList<GObject *> annotations = GObjectUtils::findObjectsRelatedToObjectByRole(seqObj, GObjectTypes::ANNOTATION_TABLE, ObjectRole_Sequence,
                allSeqAnnotations, UOF_LoadedOnly);

            foreach (GObject* ao, annotations) {
                AnnotationTableObject *atobj = qobject_cast<AnnotationTableObject *>(ao);
                if (atobj) {
                    const QList<Annotation *> annotations =  atobj->getAnnotations();
                    foreach (Annotation *a, annotations) {
                        const QVector<U2Region>& regions = a->getRegions();
                        foreach (const U2Region &reg, regions ) {
                            if (reg.endPos() >= reg.startPos && reg.startPos >= 0) {
                                mrk.set(a->getName().toStdString(), "UGENE Annotation", DDisc::Interval(reg.startPos, reg.endPos()));
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
,edData(edD)
,conDoc(NULL){
    firstFile = firstF;
}

void ExpertDiscoveryLoadControlMrkTask::prepare(){

    QString strPosName = firstFile;
    try {
        if (strPosName.right(4).compare(".xml", Qt::CaseInsensitive) == 0) {
            edData.loadControlSequenceAnnotation(strPosName);
        } else  {
            QList<FormatDetectionResult> curFormats = DocumentUtils::detectFormat(firstFile);
            if (!curFormats.isEmpty()){
                if(curFormats.first().format->getFormatId() == BaseDocumentFormats::PLAIN_GENBANK){
                    GUrl url(strPosName);
                    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
                    DocumentFormat* f = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::PLAIN_GENBANK);

                    conDoc = f->createNewUnloadedDocument(iof, url, stateInfo);
                    CHECK_OP(stateInfo, );
                    addSubTask(new LoadUnloadedDocumentTask(conDoc));

                } else {
                    ifstream fConAnn(strPosName.toStdString().c_str());
                    edData.getConMarkBase().load(fConAnn);
                }
            }
        }
    }
    catch (exception& ex) {
        edData.getConMarkBase().clear();
        QString str = "Control annotation: ";
        str += ex.what();
        QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
        mb.exec();
        setError(str);
        return;
    }
}

Task::ReportResult ExpertDiscoveryLoadControlMrkTask::report(){
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }

    if(conDoc){
        try {
            if (!ExpertDiscoveryLoadPosNegMrkTask::loadAnnotationFromUgeneDocument(edData.getConMarkBase(), edData.getConSeqBase(), conDoc))
                throw std::exception();
        }catch (exception& ex) {
            edData.getConMarkBase().clear();
            QString str = "Control annotation: ";
            str += ex.what();
            QMessageBox mb(QMessageBox::Critical, tr("Error"), str);
            mb.exec();
            setError(str);
            return ReportResult_Finished;
        }
    }

    edData.getConSeqBase().setMarking(edData.getConMarkBase());

    return ReportResult_Finished;
}


ExpertDiscoveryLoadControlTask::ExpertDiscoveryLoadControlTask(QString firstF)
: Task(tr("ExpertDiscovery loading"), TaskFlags(TaskFlag_NoRun | TaskFlag_FailOnSubtaskCancel)){
    firstFile = firstF;
}

void ExpertDiscoveryLoadControlTask::prepare( ) {
    // load sequences
    Document *doc = loadFile(firstFile);
    if (doc) {
        doc->setName("Control");
        docs << doc;
    }
}

Document* ExpertDiscoveryLoadControlTask::loadFile(QString inFile){
    GUrl url(inFile);

    QList<FormatDetectionResult> formats = DocumentUtils::detectFormat(inFile);
    if (formats.isEmpty()) {
        stateInfo.setError(tr("Detecting format error for file %1").arg(inFile));
        return NULL;
    }

    DocumentFormat* format = formats.first().format;
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));

    Document* doc = format->createNewUnloadedDocument(iof, url, stateInfo);
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

ExpertDiscoverySignalExtractorTask::ExpertDiscoverySignalExtractorTask(ExpertDiscoveryData* d) :
    Task(tr("ExpertDiscovery signals extracting"), TaskFlags(TaskFlag_FailOnSubtaskCancel)),
    extractor(NULL),
    folder(NULL),
    data(d)
{

}

ExpertDiscoverySignalExtractorTask::~ExpertDiscoverySignalExtractorTask(){
    delete extractor;
}

void ExpertDiscoverySignalExtractorTask::run(){
    GCOUNTER(cvar,tvar, "ExpertDiscoverySignalExtractor" );
    if(!extractor){
        return;
    }
    stateInfo.progress = 0;
    while(performNextStep()){
        if (stateInfo.cancelFlag) break;
        stateInfo.progress = short(extractor->progress() + 0.5);
    }

    stateInfo.progress = 100;
}

void ExpertDiscoverySignalExtractorTask::prepare(){
    ExpertDiscoveryExtSigWiz w(QApplication::activeWindow(), &data->getRootFolder(), data->getMaxPosSequenceLen(), data->isLettersMarkedUp());
    connect(&w, SIGNAL(si_newFolder(const QString&)), SLOT(sl_newFolder(const QString&)));
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
        emit si_newSignalReady(pSignal->clone(), folder);
    }
    return needOneMore;
}

void ExpertDiscoverySignalExtractorTask::sl_newFolder(const QString& folderName){
    emit si_newFolder(folderName);
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
            ObjectRole_Sequence, allSequenceObjects, UOF_LoadedAndUnloaded);

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
static QString deriveViewName(const QList<U2SequenceObject*>& seqObjects) {
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

static bool objLessThan(const U2SequenceObject* o1 , const U2SequenceObject* o2) {
    if (o1->getDocument() == o2->getDocument()) {
        return o1->getGObjectName() < o2->getGObjectName();
    }
    return o1->getDocument()->getURLString() < o2->getDocument()->getURLString();
}


void ExpertDiscoveryCreateViewTask::open() {
    if (stateInfo.hasError() || sequenceObjectRefs.isEmpty()) {
        return;
    }
    QList<U2SequenceObject*> seqObjects;
    QList<GObject*> allSequenceObjects = GObjectUtils::findAllObjects(UOF_LoadedOnly, GObjectTypes::SEQUENCE);
    foreach(const GObjectReference& r, sequenceObjectRefs) {
        GObject* obj = GObjectUtils::selectObjectByReference(r, allSequenceObjects, UOF_LoadedOnly);
        U2SequenceObject* seqObj = qobject_cast<U2SequenceObject*>(obj);
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

    AnnotationTableObject *aObj = aa->getAnnotationObject();
    U2OpStatusImpl os;
    const DNASequence& dna = aa->getSeqObject()->getWholeSequence(os);
    CHECK_OP(os, NULL);
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

ExpertDiscoveryToAnnotationTask::ExpertDiscoveryToAnnotationTask(AnnotationTableObject *aobj,
    const DNASequence &seq, ExpertDiscoveryData* d, const EDProcessedSignal* ps, QMutex& mut)
    :Task(tr("Find and store expert discovery signals on a sequence"), TaskFlags_FOSCOE),
      recDataTask(NULL),
      dna(seq),
      edData(d),
      aObj(aobj),
      curPS(ps),
      hasRecData(false),
      isControl(false),
      isPos(false),
      mutex(mut)
{
    seqRange = U2Region(0, seq.length());
    curDnaName = seq.getName();
}

void ExpertDiscoveryToAnnotationTask::prepare(){
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
    seqNumber = edData->getPosSeqBase().getObjNo(curDnaName.toStdString().c_str());
    if(seqNumber == -1){
        seqNumber = edData->getNegSeqBase().getObjNo(curDnaName.toStdString().c_str());
        if(seqNumber == -1){
            seqNumber = edData->getConSeqBase().getObjNo(curDnaName.toStdString().c_str());
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
    csToAnnotation(seqNumber, edSeq.getSize());

    recDataTask = new ExpertDiscoveryGetRecognitionDataTask(*edData, recData, edSeq);
    addSubTask(recDataTask);
}

void ExpertDiscoveryToAnnotationTask::run( ) {

}

Task::ReportResult ExpertDiscoveryToAnnotationTask::report(){
    if (isCanceled() || hasError()) {
        return ReportResult_Finished;
    }

    if (aObj->isStateLocked()) {
        setError(tr("Annotation obj %1 is locked for modifications").arg(aObj->getGObjectName()));
        return ReportResult_Finished;
    }

    aObj->addAnnotations(resultList, "ExpertDiscover Signals");

    return ReportResult_Finished;
}

void ExpertDiscoveryToAnnotationTask::csToAnnotation(int seqNumber, unsigned int seqLen){
    assert(seqNumber!=-1);

    if(isControl || !curPS){
        return;
    }

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
    while (i < seqLen) {
        first_data = "";
        if (set.is_set(i)) {
            first_data = QString::fromStdString(set.association(i));
        }

        j = i+1;
        while (j < seqLen) {
            second_data = "";
            if (set.is_set(j)) {
                second_data = QString::fromStdString(set.association(i));
            }
            if (first_data != second_data || second_data.isEmpty()) {
                break;
            }
            j++;
        }
        if (!first_data.isEmpty()) {
            SharedAnnotationData data(new AnnotationData);
            data->name = "signal";
            data->location->regions << U2Region(i,j-i);
            data->qualifiers.append(U2Qualifier("name", first_data));
            resultList.append(data);
        }

        i = j;
    }
}

QList<Task*> ExpertDiscoveryToAnnotationTask::onSubTaskFinished(Task* subTask){
    QList<Task*> res;
    if(subTask == recDataTask){
        if(recDataTask->hasRecData()){
            recDataToAnnotation();
        }
    }
    return res;
}

void ExpertDiscoveryToAnnotationTask::recDataToAnnotation( ) {
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
            SharedAnnotationData data(new AnnotationData);
            data->name = "rec.data";
            data->location->regions << U2Region(i,j-i);
            data->qualifiers.append(U2Qualifier("criteria", QString::number(first_rec_data)));
            resultList.append(data);
        }
        i = j;
    }
}

ExpertDiscoveryUpdateSelectionTask::ExpertDiscoveryUpdateSelectionTask(ExpertDiscoveryView* currentView, QTreeWidgetItem* tItem)
    : Task("Update selection task", TaskFlag_None), view(currentView), curretItem(tItem)
{
    currentAdv = view->getCurrentAdv();
    curPS = view->getCurrentProcessedSignals();
    updatePS = true;
    pItem = NULL;
}

void ExpertDiscoveryUpdateSelectionTask::run(){
    assert(curretItem);
    pItem = dynamic_cast<EDProjectItem*>(curretItem);
    assert(pItem);

    switch(pItem->getType()){
       case PIT_CS:
       case PIT_CSN_WORD:
       case PIT_CSN_INTERVAL:
       case PIT_CSN_REPETITION:
       case PIT_CSN_DISTANCE:
       case PIT_CSN_MRK_ITEM:
       case PIT_MRK_ITEM:{
           EDPICSNode* pPICSN = dynamic_cast<EDPICSNode*>(pItem);

           if (curPS == pPICSN->getProcessedSignal(view->getExpertDiscoveryData())) {
               updatePS = false;
           }
           else {
               curPS = pPICSN->getProcessedSignal(view->getExpertDiscoveryData());
               updatePS = true;
           }
       }
       break;
       default:
           assert(0);
    }


}
Task::ReportResult ExpertDiscoveryUpdateSelectionTask::report(){
    if (curPS == NULL) {
        updateAnnotations();
        updatePS = false;
        view->getPropertiesWidget()->sl_treeSelChanged(pItem);
        view->setProcessedSignals(curPS);
        return ReportResult_Finished;
    }

    if(updatePS){
        updateAnnotations();
    }
    view->getPropertiesWidget()->sl_treeSelChanged(curretItem);
    view->setProcessedSignals(curPS);
    return ReportResult_Finished;
}

void ExpertDiscoveryUpdateSelectionTask::updateAnnotations(){
    if(!currentAdv || !view->getProjectTree()->isEnabled() || view->getProjectTree()->isUpdatingItem()){
        return;
    }

    view->getAutoAnnotationUpdater()->setEDProcSignals(curPS);

    foreach(ADVSequenceObjectContext* sctx, currentAdv->getSequenceContexts()){
        AutoAnnotationUtils::triggerAutoAnnotationsUpdate(sctx, "ExpertDiscover Signals");
    }
}

ExpertDiscoveryGetRecognitionDataTask::ExpertDiscoveryGetRecognitionDataTask(ExpertDiscoveryData& data, RecognizationData& _recData, Sequence& seq)
:Task("ExpertDiscvery recognition task", TaskFlag_None)
,edData(data)
,recData(_recData)
,curSequence(seq)
,isRecData(false)
{

}
void ExpertDiscoveryGetRecognitionDataTask::run(){
    isRecData = edData.recDataStorage.getRecognizationData(recData, &curSequence, edData.getSelectedSignalsContainer(), stateInfo);
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

    edData.setModifed(false);

}

ExpertDiscoveryLoadDocumentTask::ExpertDiscoveryLoadDocumentTask(ExpertDiscoveryData& data, const QString& fileName)
:Task("Load ExpertDiscovery document task", TaskFlag_None)
,edData(data)
,filename(fileName)
{
    tpm = Progress_Manual;
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

    stateInfo.progress = 0;
    if(stateInfo.isCanceled()){
        return;
    }

    QDataStream inStream(&file);

    EDPMCSFolder::load(inStream, &edData.getRootFolder());

    bool m_bOptimizeRecogniztionBound;
    double m_dRecognizationBound;

    inStream >> m_dRecognizationBound;
    inStream >> m_bOptimizeRecogniztionBound;

    edData.setRecBound(m_dRecognizationBound);

    stateInfo.progress = 10;
    if(stateInfo.isCanceled()){
        return;
    }

    EDPMSeqBase::load(inStream, edData.getPosSeqBase());
    EDPMSeqBase::load(inStream, edData.getNegSeqBase());
    EDPMSeqBase::load(inStream, edData.getConSeqBase());
    stateInfo.progress = 50;
    if(stateInfo.isCanceled()){
        return;
    }
    EDPMMrkBase::load(inStream, edData.getPosMarkBase(), edData.getPosSeqBase().getSize());
    EDPMMrkBase::load(inStream, edData.getNegMarkBase(), edData.getNegSeqBase().getSize());
    EDPMMrkBase::load(inStream, edData.getConMarkBase(), edData.getConSeqBase().getSize());
    EDPMDescription::load(inStream, edData.getDescriptionBaseNoConst());

    stateInfo.progress = 80;
    if(stateInfo.isCanceled()){
        return;
    }

    edData.getPosSeqBase().setMarking(edData.getPosMarkBase());
    edData.getNegSeqBase().setMarking(edData.getNegMarkBase());
    edData.getConSeqBase().setMarking(edData.getConMarkBase());

    edData.getSelectedSignalsContainer().load(inStream, edData.getRootFolder());

    stateInfo.progress = 100;
}

ExpertDiscoveryMarkupTask::ExpertDiscoveryMarkupTask(ExpertDiscoveryData& data)
:Task("ExpertDiscovery markup letters", TaskFlag_None), edData(data){
    isLettersMarkup = true;
    signal = NULL;
}
ExpertDiscoveryMarkupTask::ExpertDiscoveryMarkupTask(ExpertDiscoveryData& data, const EDProcessedSignal* _signal)
:Task("ExpertDiscovery signal markup", TaskFlag_None), edData(data){
    isLettersMarkup = false;
    signal = _signal;
}
void ExpertDiscoveryMarkupTask::run(){
    if(isLettersMarkup){
        stateInfo.progress = 0;
        edData.markupLetters();
        stateInfo.progress = 100;
    }else{
        if(!signal){
            return;
        }
        edData.clearScores();

        // Adding new signal family for letters
        std::string strFamilyName = ExpertDiscoveryData::FAMILY_ED_SIGNAL.toStdString();
        std::string strMethodName = ExpertDiscoveryData::FAMILY_ED_METHOD.toStdString();

        Family ed_signals;
        bool initialiseFamily = false;
        try{
            ed_signals = edData.getDescriptionBase().getSignalFamily(strFamilyName);
        }catch(...){
            initialiseFamily = true;
            ed_signals.setName(strFamilyName);
        }

        MetaInfo mi;
        int number = ed_signals.getSignalNumber();
        curSignalName = QString("ED_SIGNAL_%1").arg(number);
        mi.setName(curSignalName.toStdString());
        mi.setNo(number);
        mi.setMethodName(strMethodName);
        ed_signals.AddInfo(mi);

        if(initialiseFamily){
            edData.getDescriptionBaseNoConst().addFamily(ed_signals);
        }

        addSignalMarkup(edData.getPosSeqBase(), edData.getPosMarkBase(), true);
        addSignalMarkup(edData.getNegSeqBase(), edData.getNegMarkBase(), false);

        edData.setModifed();
    }
}

void ExpertDiscoveryMarkupTask::addSignalMarkup(SequenceBase& rBase, MarkingBase& rAnn, bool isPos){
    std::string strFamilyName = ExpertDiscoveryData::FAMILY_ED_SIGNAL.toStdString();

    int size = rBase.getSize();
    std::string seq;
    for (int i=0; i<size; i++) {
        const Set& signalSet = isPos ? signal->getYesRealizations(i) : signal->getNoRealizations(i);
        seq = rBase.getSequence(i).getSequence();
        Marking mrk;
        try {
            mrk = rAnn.getMarking(i);
            }
            catch (exception) {
            }
            int len = (int)seq.size();
            for (int j=0; j<len; j++) {
                if(signalSet.is_set(j)){
                    mrk.set(curSignalName.toStdString(), strFamilyName, Interval(j,j));
                }
            }
            rAnn.setMarking(i, mrk);
        }
     rBase.setMarking(rAnn);
}


ExpertDiscoveryCalculateErrors::ExpertDiscoveryCalculateErrors(const CalculateErrorTaskInfo& _settings)
:BackgroundTask<ErrorsInfo>(tr("Error calculation"), TaskFlag_None), settings(_settings){
    tpm = Progress_Manual;
}
void ExpertDiscoveryCalculateErrors::run(){

    double step = settings.scoreStep;
    int stepsNum = settings.scoreReg.length / step;
    stateInfo.progress = 0;
    result.errorFirstType.resize(stepsNum);
    result.errorSecondType.resize(stepsNum);
    result.maxErrorVal = 0;
    result.minErrorVal = std::numeric_limits<double>::max();
    double probPosRej = 0;
    double probNegRec = 0;
    double curScore = settings.scoreReg.startPos;
    for(int i = 0; i < stepsNum; i++, curScore+=step){
        stateInfo.progress = (i/stepsNum)*100;
        probPosRej = 0;
        int size = (int)settings.posScore.size();
        for (int j=0; j<size; j++)
            if (settings.posScore[j] < curScore) probPosRej++;
        probPosRej /= settings.posScore.size();
        result.errorFirstType[i] = probPosRej;

        probNegRec = 0;
        size = (int)settings.negScore.size();
        for (int j=0; j<size; j++)
            if (settings.negScore[j] >= curScore) probNegRec++;
        probNegRec /= settings.negScore.size();
        result.errorSecondType[i] = probNegRec;

        result.maxErrorVal = qMax(result.maxErrorVal, probPosRej);
        result.maxErrorVal = qMax(result.maxErrorVal, probNegRec);

        result.minErrorVal = qMin(result.minErrorVal, probPosRej);
        result.minErrorVal = qMin(result.minErrorVal, probNegRec);
    }

    stateInfo.progress = 100;
}

//search

ExpertDiscoverySearchTask::ExpertDiscoverySearchTask(ExpertDiscoveryData& data, const QByteArray& seq, const ExpertDiscoverySearchCfg& cfg, int ro)
    : Task(tr("ExpertDiscovery Search"), TaskFlags_NR_FOSCOE), edData(data), cfg(cfg), resultsOffset(ro), wholeSeq(seq), lenLeft(0), curLeft(-1)
{
    SequenceWalkerConfig c;
    c.walkCircular = false;
    c.seq = wholeSeq.constData();
    c.seqSize = wholeSeq.length();
    c.complTrans  = cfg.complTT;
    c.strandToWalk = cfg.complTT == NULL ? StrandOption_DirectOnly : StrandOption_Both;
    c.aminoTrans = NULL;

    c.chunkSize = edData.getMaxPosSequenceLen();
    c.nThreads = 1;         //for now ExpertDiscovery signal can be processed only in one thread
    c.overlapSize = c.chunkSize - 1; //move window to 1bp

    //for progress
    lenLeft = wholeSeq.size();
    if (c.strandToWalk == StrandOption_Both){
        lenLeft*=2;
    }
    stateInfo.progress = 0;

    SequenceWalkerTask* t = new SequenceWalkerTask(c, this, tr("ExpertDiscovery Search Parallel"));
    addSubTask(t);
}

void ExpertDiscoverySearchTask::onRegion(SequenceWalkerSubtask* t, TaskStateInfo& ti) {
    if ((cfg.complOnly && !t->isDNAComplemented()) || ti.isCanceled()) {
        return;
    }
    U2Region globalRegion = t->getGlobalRegion();
    qint64 seqLen = globalRegion.length;
    const char* seq = t->getGlobalConfig().seq + globalRegion.startPos;
    DNATranslation* complTT = t->isDNAComplemented() ? t->getGlobalConfig().complTrans : NULL;
    float score = ExpertDiscoveryData::calculateSequenceScore(seq, seqLen, edData, complTT);
    if (score < 0) {
        ti.setError(  tr("Internal error, score:%1").arg(score) );
        return;
    }
    ExpertDiscoverySearchResult r;
    r.score = score;
    if (r.score >= cfg.minSCORE) {//report result
        r.strand = t->isDNAComplemented() ? U2Strand::Complementary : U2Strand::Direct;
        r.region.startPos = globalRegion.startPos + resultsOffset;
        r.region.length = seqLen;
        addResult(r);
    }

    if(ti.progress <= 100){
        ti.progress+= (int)(100/(float)lenLeft  + 0.5);
    }
}


void ExpertDiscoverySearchTask::addResult(const ExpertDiscoverySearchResult& r) {
    lock.lock();
    results.append(r);
    lock.unlock();
}

QList<ExpertDiscoverySearchResult> ExpertDiscoverySearchTask::takeResults() {
    lock.lock();
    QList<ExpertDiscoverySearchResult> res = results;
    results.clear();
    lock.unlock();
    return res;
}



ExpertDiscoveryExportSequences::ExpertDiscoveryExportSequences( const SequenceBase& _base )
:Task(tr("Export Sequences Task"), TaskFlag_None)
,base(_base)
{

}


void ExpertDiscoveryExportSequences::prepare() {
    const QString filter = DialogUtils::prepareDocumentsFileFilter(BaseDocumentFormats::FASTA, false, QStringList());
    fileName = U2FileDialog::getSaveFileName(qobject_cast<QWidget *>(AppContext::getMainWindow()->getQMainWindow()), tr("Save File"), "", filter);
    if (fileName.isEmpty()) {
        cancel();
    }
}

void ExpertDiscoveryExportSequences::run() {
    if (hasError() || isCanceled()){
        return;
    }

    ofstream out(fileName.toStdString().c_str());

    if (!out.is_open()) {
        setError(tr("Report generation failed: ") + L10N::errorOpeningFileWrite(GUrl(fileName)));
        return;
    }

    base.save(out);
}

Task::ReportResult ExpertDiscoveryExportSequences::report() {
    if (hasError()) {
        QMessageBox(QMessageBox::Critical, tr("Error"), stateInfo.getError()).exec();
    }
    return ReportResult_Finished;
}

}//namespace
