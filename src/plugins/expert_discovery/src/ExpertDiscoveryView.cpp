#include "ExpertDiscoveryView.h"
#include "ExpertDiscoveryTask.h"
#include "ExpertDiscoveryPosNegDialog.h"
#include "ExpertDiscoveryControlDialog.h"
#include "ExpertDiscoveryPosNegMrkDialog.h"
#include "ExpertDiscoveryControlMrkDialog.h"
#include "ExpertDiscoveryExtSigWiz.h"
#include "ExpertDiscoveryPlugin.h"
#include "ExpertDiscoveryGraphs.h"
#include "ExpertDiscoverySearchDialogController.h"

#include <U2Gui/LastUsedDirHelper.h>
#include <U2View/ADVUtils.h>
#include <U2View/ADVSequenceObjectContext.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>
#include <U2View/AutoAnnotationUtils.h>
#include <U2View/DetView.h>

#include <U2Core/AppContext.h>
#include <U2Core/BaseDocumentFormats.h>
#include <U2Core/IOAdapter.h>
#include <U2Core/IOAdapterUtils.h>
#include <U2Core/AppSettings.h>
#include <U2Core/UserApplicationsSettings.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/GObjectSelection.h>
#include <U2Core/GHints.h>
#include <U2Core/Counter.h>
#include <U2Core/U2OpStatusUtils.h>
#include <U2Core/U2SequenceUtils.h>

#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>


namespace U2{
using namespace DDisc;
ExpertDiscoveryView::ExpertDiscoveryView(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p)
:GObjectView(factoryId, viewName, p)
,wizzard(false)
,splitter(NULL)
,currentAdv(NULL)
,edAutoAnnotationsUpdater(NULL)
,curPS(NULL)
,updatePS(false)
,curEDsequence(NULL)
,posUDoc(NULL)
,negUDoc(NULL)
,conUDoc(NULL)
,extrTask(NULL)
,updatesCount(0)
,curTask(NULL){

    createActions();

    edAutoAnnotationsUpdater = new ExpertDiscoverySignalsAutoAnnotationUpdater;
    AppContext::getAutoAnnotationsSupport()->registerAutoAnnotationsUpdater(edAutoAnnotationsUpdater);
    edAutoAnnotationsUpdater->setEDData(&d);
    edAutoAnnotationsUpdater->setEDProcSignals(curPS);
    edAutoAnnotationsUpdater->setEDMutex(&mutex);
}

ExpertDiscoveryView::~ExpertDiscoveryView(){

    clearSequencesView();
    delete posUDoc;
    delete negUDoc;
    delete conUDoc;
    delete curEDsequence;

    AppContext::getAutoAnnotationsSupport()->unregisterAutoAnnotationsUpdater(edAutoAnnotationsUpdater);
}

QWidget* ExpertDiscoveryView::createWidget(){

    GCOUNTER(cvar,tvar, "ExpertDiscoveryWindowOpened" );

    splitter = new QSplitter(Qt::Horizontal);

    signalsWidget = new EDProjectTree(splitter, d);
    signalsWidget->setHeaderLabel("Items");

    propWidget = new EDPropertiesTable(splitter);

    QSplitter* verticalSplitter = new QSplitter(Qt::Vertical);

    verticalSplitter->addWidget(signalsWidget);
    verticalSplitter->addWidget(propWidget);

    splitter->addWidget(verticalSplitter);

    //connect(signalsWidget, SIGNAL(itemClicked ( QTreeWidgetItem * , int )), SLOT(sl_treeItemSelChanged(QTreeWidgetItem *)));
    connect(signalsWidget, SIGNAL(itemActivated ( QTreeWidgetItem * , int )), SLOT(sl_treeItemSelChanged(QTreeWidgetItem *)));
    connect(signalsWidget, SIGNAL(currentItemChanged ( QTreeWidgetItem * , QTreeWidgetItem * ) ), SLOT(sl_treeItemSelChanged(QTreeWidgetItem *)));
    connect(signalsWidget, SIGNAL(si_loadMarkup(bool ) ), SLOT(sl_treeWidgetMarkup(bool )));
    connect(signalsWidget, SIGNAL(si_addToMarkup()), SLOT(sl_treeWidgetAddMarkup()));
    connect(signalsWidget, SIGNAL(si_showSequence() ), SLOT(sl_showSequence()));
    connect(signalsWidget, SIGNAL(si_addToShown() ), SLOT(sl_addToShown()));
    connect(signalsWidget, SIGNAL(si_clearDisplayed() ), SLOT(sl_clearDisplayed()));
    connect(signalsWidget, SIGNAL(si_showFirstSequences() ), SLOT(sl_showFirstSequences()));
    connect(signalsWidget, SIGNAL(si_changeProp(QTreeWidgetItem*)), propWidget, SLOT(sl_treeSelChanged(QTreeWidgetItem*)));

    connect(propWidget, SIGNAL(si_propChanged(EDProjectItem*, const EDPIProperty*, QString )), signalsWidget, SLOT(sl_propChanged(EDProjectItem* , const EDPIProperty* , QString )));

    GObjectViewFactory* factory = AppContext::getObjectViewFactoryRegistry()->getFactoryById("ED");
    ExpertDiscoveryViewFactory* edFactory = dynamic_cast<ExpertDiscoveryViewFactory*>(factory);
    assert(edFactory);
    connect(edFactory, SIGNAL(si_newTaskCreation(Task* )), SLOT(sl_newViewTask(Task*)));

    return splitter;
}


void ExpertDiscoveryView::insertSeqView( QWidget* view ){

    splitter->addWidget(view);

}

void ExpertDiscoveryView::createActions(){
    newDoc = new QAction(tr("New Expert Discovery Document"), this);
    newDoc->setIcon(QIcon(":expert_discovery/images/filenew.png"));
    connect(newDoc, SIGNAL(triggered()), SLOT(sl_newDoc()));

    openDoc = new QAction(tr("Open Expert Discovery Document"), this);
    openDoc->setIcon(QIcon(":expert_discovery/images/fileopen.png"));
    connect(openDoc, SIGNAL(triggered()), SLOT(sl_openDoc()));

    saveDoc = new QAction(tr("Save Expert Discovery Document"), this);
    saveDoc->setIcon(QIcon(":expert_discovery/images/filesave.png"));
    connect(saveDoc, SIGNAL(triggered()), SLOT(sl_saveDoc()));


    setUpRecBound = new QAction(tr("Set Recognition Bound"), this);
    setUpRecBound->setIcon(QIcon(":expert_discovery/images/setRecBound.png"));
    connect(setUpRecBound, SIGNAL(triggered()), SLOT(sl_setRecBound()));

    //     optimizeRecBound = new QAction(tr("Optimize Recognition Bound"), this);
    //     optimizeRecBound->setIcon(QIcon(":expert_discovery/images/optRecBound.png"));
    //     connect(optimizeRecBound, SIGNAL(triggered()), SLOT(sl_optimizeRecBound()));

    loadControlSeqAction = new QAction(tr("Load control sequences"), this);
    loadControlSeqAction->setIcon(QIcon(":expert_discovery/images/loadControlsSeq.png"));
    connect(loadControlSeqAction, SIGNAL(triggered()), SLOT(sl_showExpertDiscoveryControlDialog()));

    extractSignalsAction = new QAction(tr("Extract signals"), this);
    extractSignalsAction->setIcon(QIcon(":expert_discovery/images/extractSignals2.png"));
    connect(extractSignalsAction, SIGNAL(triggered()), SLOT(sl_extractSignals()));

    loadMarkupAction = new QAction(tr("Load markup"), this);
    loadMarkupAction->setIcon(QIcon(":expert_discovery/images/loadMarkup.png"));
    connect(loadMarkupAction, SIGNAL(triggered()), SLOT(sl_showExpertDiscoveryPosNegMrkDialog()));

    //     loadControlMarkupAction = new QAction(tr("Load control sequences markup"), this);
    //     loadControlMarkupAction->setIcon(QIcon(":expert_discovery/images/loadControlsSeqAnnot.png"));
    //     connect(loadControlMarkupAction, SIGNAL(triggered()), SLOT(sl_showExpertDiscoveryControlMrkDialog()));

    generateFullReportAction = new QAction(tr("Generate recognition report"), this);
    generateFullReportAction->setIcon(QIcon(":expert_discovery/images/genRep.png"));
    connect(generateFullReportAction, SIGNAL(triggered()), SLOT(sl_generateFullReport()));

    setUpRecBound->setEnabled(false);
    //optimizeRecBound->setEnabled(false);
    loadControlSeqAction->setEnabled(false);
    extractSignalsAction->setEnabled(false);
    loadMarkupAction->setEnabled(false);
    //loadControlMarkupAction->setEnabled(false);
    generateFullReportAction->setEnabled(false);

}

void ExpertDiscoveryView::sl_newDoc(){
    wizzard = true;

    if (askForSave()){
        sl_saveDoc();
    }

    setUpRecBound->setEnabled(false);
    //optimizeRecBound->setEnabled(false);
    loadControlSeqAction->setEnabled(false);
    extractSignalsAction->setEnabled(false);
    loadMarkupAction->setEnabled(false);
    /*loadControlMarkupAction->setEnabled(false);*/
    generateFullReportAction->setEnabled(false);

    d.setRecBound(0);
    d.cleanup();
    propWidget->clearAll();
    clearSequencesView();
    edObjects.clear();
    curPS = NULL;
    signalsWidget->clearTree();
    signalsWidget->updateTree(ED_UPDATE_ALL);
    d.setModifed(false);

    sl_showExpertDiscoveryPosNegDialog(); 
}

void ExpertDiscoveryView::sl_openDoc(){

    LastUsedDirHelper lod("ExpertDiscovery");           
    lod.url = QFileDialog::getOpenFileName(NULL, tr("Load ExpertDiscovery document"), lod.dir, tr("ExpertDiscovery files (*.exd)"));

    if (lod.url.length() <= 0) {
        return;
    }

    d.cleanup();
    propWidget->clearAll();
    clearSequencesView();
    edObjects.clear();
    curPS = NULL;
    signalsWidget->clearTree();
    signalsWidget->updateTree(ED_UPDATE_ALL);

    Task* t = new ExpertDiscoveryLoadDocumentTask(d, lod.url);
    connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_updateAll() ) );
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}
void ExpertDiscoveryView::sl_saveDoc(){

    LastUsedDirHelper lod("ExpertDiscovery");
    lod.url = QFileDialog::getSaveFileName(NULL, tr("Save ExpertDiscovery document"), lod.dir, tr("ExpertDiscovery files (*.exd)"));

    if (lod.url.length() <= 0) {
        return;
    }

    Task* t = new ExpertDiscoverySaveDocumentTask(d, lod.url);
    AppContext::getTaskScheduler()->registerTopLevelTask(t);
}

bool ExpertDiscoveryView::askForSave(){
    if(!d.isModified()){
        return false;
    }

    QMessageBox mb(QMessageBox::Question, tr("Save ExpertDiscovery document"), tr("Do you want to save current ExpertDiscovery document?"), QMessageBox::Yes|QMessageBox::No);
    if(mb.exec()==QMessageBox::Yes)
        return true;
    else 
        return false;
}

void ExpertDiscoveryView::sl_showExpertDiscoveryPosNegDialog(){
    Task *tasks = new Task("Loading positive and negative sequences", TaskFlag_NoRun);

    ExpertDiscoveryPosNegDialog d(QApplication::activeWindow());
    if (d.exec()) {
        if (!AppContext::getProject()) {
            tasks->addSubTask( AppContext::getProjectLoader()->createNewProjectTask() );
        }

        ExpertDiscoveryLoadPosNegTask *t = new ExpertDiscoveryLoadPosNegTask(d.getFirstFileName(), d.getSecondFileName(), d.isGenerateNegative());
        connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_loadPosNegTaskStateChanged() ) );
        tasks->addSubTask(t);
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);

}
void ExpertDiscoveryView::sl_loadPosNegTaskStateChanged(){
    ExpertDiscoveryLoadPosNegTask *loadTask = qobject_cast<ExpertDiscoveryLoadPosNegTask*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasError()) {
        ExpertDiscoveryErrors::fileOpenError();
        return;
    }

    QList <Document *> docs = loadTask->getDocuments();

    Q_ASSERT(!docs.isEmpty());
    Q_ASSERT(docs.count() > 1);

    posUDoc = docs.first();
    d.setPosBase(posUDoc->getObjects());
    d.setBaseFilename(d.getPosSeqBase(), posUDoc->getURLString());

    bool isSequencesDoc = true;
    foreach(GObject* gobj, posUDoc->getObjects()){
        //addObject(gobj);
        if(gobj->getGObjectType()==GObjectTypes::SEQUENCE){
            edObjects.push_back(gobj);
        }else{
            isSequencesDoc = false;
        }
    }
    if(posUDoc->isStateLocked() || !isSequencesDoc){
        posUDoc = NULL;
    }

    negUDoc = docs.at(1);
    d.setNegBase(negUDoc->getObjects());
    d.setBaseFilename(d.getNegSeqBase(), negUDoc->getURLString());

    isSequencesDoc = true;
    foreach(GObject* gobj, negUDoc->getObjects()){
        //addObject(gobj);
        if(gobj->getGObjectType()==GObjectTypes::SEQUENCE){
            edObjects.push_back(gobj);
        }else{
            isSequencesDoc = false;
        }
    }

    if(negUDoc->isStateLocked() || !isSequencesDoc){
        negUDoc = NULL;
    }

    signalsWidget->updateSequenceBase(PIT_POSSEQUENCEBASE);
    signalsWidget->updateSequenceBase(PIT_NEGSEQUENCEBASE);


    setUpRecBound->setEnabled(true);
    //optimizeRecBound->setEnabled(true);
    loadControlSeqAction->setEnabled(true);
    loadMarkupAction->setEnabled(true);
    generateFullReportAction->setEnabled(true);

    //markup
    sl_showExpertDiscoveryPosNegMrkDialog();

    wizzard = false;
}

void ExpertDiscoveryView::sl_showExpertDiscoveryPosNegMrkDialog(){
    Task *tasks = new Task("Loading positive and negative sequences markups", TaskFlag_NoRun);

    ExpertDiscoveryPosNegMrkDialog dialog(QApplication::activeWindow());
    if (dialog.exec()) {

        ExpertDiscoveryLoadPosNegMrkTask *t = new ExpertDiscoveryLoadPosNegMrkTask(dialog.getFirstFileName(), dialog.getSecondFileName(), dialog.getThirdFileName(), dialog.isGenerateDescr(), dialog.isAppendToCurrentMarkup(), dialog.isNucleotidesMarkup(), d );
        connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_loadPosNegMrkTaskStateChanged() ) );
        tasks->addSubTask(t);
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
}
void ExpertDiscoveryView::sl_loadPosNegMrkTaskStateChanged(){
    ExpertDiscoveryLoadPosNegMrkTask *loadTask = qobject_cast<ExpertDiscoveryLoadPosNegMrkTask*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasError()) {
        ExpertDiscoveryErrors::markupLoadError();
        return;
    }

    signalsWidget->updateMarkup();
    extractSignalsAction->setEnabled(true);
}

// void ExpertDiscoveryView::sl_showExpertDiscoveryControlMrkDialog(){
//     Task *tasks = new Task("Loading control sequences markups", TaskFlag_NoRun);
// 
//     ExpertDiscoveryControlMrkDialog dialog(QApplication::activeWindow());
//     if (dialog.exec()) {
// 
//         ExpertDiscoveryLoadControlMrkTask *t = new ExpertDiscoveryLoadControlMrkTask(dialog.getFirstFileName(), d );
//         connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_loadControlMrkTaskStateChanged() ) );
//         tasks->addSubTask(t);
//     }
// 
//     AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
// }
// void ExpertDiscoveryView::sl_loadControlMrkTaskStateChanged(){
//     ExpertDiscoveryLoadControlMrkTask *loadTask = qobject_cast<ExpertDiscoveryLoadControlMrkTask*>(sender());
//     if (!loadTask || !loadTask->isFinished()) {
//         return;
//     }
// 
//     if (loadTask->getStateInfo().hasError()) {
//         ExpertDiscoveryErrors::markupLoadError();
//         return;
//     }
// }

void ExpertDiscoveryView::initADVView(AnnotatedDNAView* adv){
    if(!adv){
        return;
    }

    clearSequencesView();

    foreach(GObject* gobj, adv->getObjects()){
        GHints* edHints = new GHintsDefaultImpl();
        edHints->set("EDHint", QVariant(true));
        gobj->setGHints(edHints);
        addObject(gobj);
    }
    adv->setClosingInterface(closeInterface);
    currentAdv = adv;
    splitter->addWidget(adv->getWidget());
    //    adv->addAutoAnnotationsUpdated(edAutoAnnotationsUpdater);

    //hide complement strand and translations, adding the graph action
    QList<ADVSequenceWidget*> curSequenceWidgets = currentAdv->getSequenceWidgets();
    foreach(ADVSequenceWidget* seqWidget, curSequenceWidgets){
        ADVSequenceObjectContext* cont = seqWidget->getActiveSequenceContext();
        U2SequenceObject* seqObj = cont->getSequenceObject();
        QString seqName = seqObj->getSequenceName();
        SequenceType sType = d.getSequenceTypeByName(seqName);
        int seqNumber = d.getSequenceIndex(seqName, sType);
         
        if(seqNumber!=-1){
            GSequenceGraphFactory* graphFactory = new ExpertDiscoveryScoreGraphFactory(seqWidget, d, seqNumber, sType);
            GraphAction* graphAction = new GraphAction(graphFactory);
            GraphMenuAction::addGraphAction(seqWidget->getActiveSequenceContext(), graphAction);
        }

        ADVSingleSequenceWidget* singleSeqWidget = dynamic_cast<ADVSingleSequenceWidget*>(seqWidget);
        if(singleSeqWidget){
            DetView* dv = singleSeqWidget->getDetView();
            if(dv){
                dv->setShowComplement(false);
                dv->setShowTranslation(false);
            }
        }
    }

    foreach(ADVSequenceObjectContext* sctx, currentAdv->getSequenceContexts()){    
        AutoAnnotationsADVAction* aaAction = AutoAnnotationUtils::findAutoAnnotationADVAction( sctx );
        assert(aaAction);
        AutoAnnotationObject* aaobj = aaAction->getAAObj();
        assert(aaobj);
        connect(aaobj, SIGNAL(si_updateStarted()), SLOT(sl_autoAnnotationUpdateStarted()));
        connect(aaobj, SIGNAL(si_updateFinshed()), SLOT(sl_autoAnnotationUpdateFinished()));
    }

    ADVGlobalAction* a = new ADVGlobalAction(adv, QIcon(), tr("Search for regions with ExpertDiscovery"), 80);
    a->addAlphabetFilter(DNAAlphabet_NUCL);
    connect(a, SIGNAL(triggered()), SLOT(sl_search()));

    connect(adv, SIGNAL( si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*) ), SLOT( sl_sequenceItemSelChanged(ADVSequenceWidget*) ));

    createEDSequence();
}

void ExpertDiscoveryView::sl_testView(){
    Task *loadTask = qobject_cast<Task*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasError()) {
        return;
    }

    ExpertDiscoveryCreateViewTask* viewTask = dynamic_cast<ExpertDiscoveryCreateViewTask*>(loadTask);
    if(!viewTask){
        return;
    }

    AnnotatedDNAView* adv = viewTask->getADV();
    initADVView(adv);

}

void ExpertDiscoveryView::sl_newViewTask(Task* t){
    ExpertDiscoveryCreateViewTask* edTask = dynamic_cast<ExpertDiscoveryCreateViewTask*>(t);
    if(!edTask){
        return;
    }

    connect(edTask,SIGNAL( si_stateChanged() ), SLOT( sl_testView() ) );
}

void ExpertDiscoveryView::sl_updateAll(){
    ExpertDiscoveryLoadDocumentTask *loadTask = qobject_cast<ExpertDiscoveryLoadDocumentTask*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasError()) {
        return;
    }
    signalsWidget->updateTree(ED_CURRENT_ITEM_CHANGED,NULL);
    signalsWidget->updateSequenceBase(PIT_POSSEQUENCEBASE);
    signalsWidget->updateSequenceBase(PIT_NEGSEQUENCEBASE);
    signalsWidget->updateSequenceBase(PIT_CONTROLSEQUENCEBASE);
    signalsWidget->updateTree(ED_UPDATE_ALL);

    bool enableActions = d.getPosSeqBase().getSize() != 0  || d.getNegSeqBase().getSize() != 0;
    setUpRecBound->setEnabled(enableActions);
    //optimizeRecBound->setEnabled(enableActions);
    loadControlSeqAction->setEnabled(enableActions);
    extractSignalsAction->setEnabled(enableActions);
    loadMarkupAction->setEnabled(enableActions);
    /*loadControlMarkupAction->setEnabled(d.getConSeqBase().getSize() != 0);*/
    generateFullReportAction->setEnabled(enableActions);

}

void ExpertDiscoveryView::sl_autoAnnotationUpdateStarted(){
    updatesCount++;
    if(updatesCount > 0){
        signalsWidget->setEnabled(false);
    }
}

void ExpertDiscoveryView::sl_search(){
    GObjectViewAction* action = qobject_cast<GObjectViewAction*>(sender());
    AnnotatedDNAView* av = qobject_cast<AnnotatedDNAView*>(action->getObjectView());

    ADVSequenceObjectContext* seqCtx = av->getSequenceInFocus();
    assert(seqCtx->getAlphabet()->isNucleic());
    ExpertDiscoverySearchDialogController sdialog(seqCtx, d, av->getWidget());
    sdialog.exec();
}
void ExpertDiscoveryView::sl_autoAnnotationUpdateFinished(){
    updatesCount--;

    if(updatesCount <= 0){
        signalsWidget->setEnabled(true);
        signalsWidget->setFocus();
        updatesCount = 0;
    }
}

void ExpertDiscoveryView::sl_showExpertDiscoveryControlDialog(){
    Task *tasks = new Task("Loading control sequences", TaskFlag_NoRun);

    ExpertDiscoveryControlDialog d(QApplication::activeWindow());
    if (d.exec()) {
        Q_ASSERT(AppContext::getProject());
        ExpertDiscoveryLoadControlTask *t = new ExpertDiscoveryLoadControlTask(d.getFirstFileName());
        connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_loadControlTaskStateChanged() ) );
        tasks->addSubTask(t);
    }

    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);

}
void ExpertDiscoveryView::sl_loadControlTaskStateChanged(){
    ExpertDiscoveryLoadControlTask *loadTask = qobject_cast<ExpertDiscoveryLoadControlTask*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasError()) {
        ExpertDiscoveryErrors::fileOpenError();
        return;
    }

    QList <Document *> docs = loadTask->getDocuments();

    Q_ASSERT(!docs.isEmpty());

    conUDoc = docs.first();

    propWidget->clearAll();

    if(d.getConSeqBase().getSize() != 0){
        clearSequencesView();

        QList<EDPISequence*> selSeqList = d.getSelectetSequencesList();
        d.clearSelectedSequencesList();
        foreach(EDPISequence* curS, selSeqList){
            signalsWidget->updateItem(curS);
        }

        foreach(GObject* gobj, edObjects){
            SequenceType seqType = d.getSequenceTypeByName(gobj->getGObjectName());
            if(seqType == CONTROL_SEQUENCE){
                edObjects.removeOne(gobj);
            }
        }
    }

    curPS = NULL;

    d.clearContrBase();
    d.clearContrAnnot();

    d.setConBase(conUDoc->getObjects());
    d.setBaseFilename(d.getConSeqBase(), conUDoc->getURLString());

    bool isSequencesDoc = true;
    foreach(GObject* gobj, conUDoc->getObjects()){
        if(gobj->getGObjectType() == GObjectTypes::SEQUENCE){
            edObjects.push_back(gobj);
        }else{
            isSequencesDoc = false;
        }
    }

    if(conUDoc->isStateLocked() || !isSequencesDoc){
        conUDoc = NULL;
    }

    if (d.isLettersMarkedUp() && d.getConSeqBase().getSize() != 0)
        d.markupLetters(d.getConSeqBase(), d.getConMarkBase());

    signalsWidget->updateSequenceBase(PIT_CONTROLSEQUENCEBASE);

    /*loadControlMarkupAction->setEnabled(true);*/

}

void ExpertDiscoveryView::sl_newSignalReady(DDisc::Signal* signal, CSFolder* folder){
    Signal* ps = signal;
    CSFolder *pFolder = folder;
    ps->setName(pFolder->makeUniqueSignalName().toStdString());
    pFolder->addSignal(ps);
    EDProjectItem* pParent = signalsWidget->findEDItem(pFolder);
    EDPICS* pItem = new EDPICS(ps);
    signalsWidget->addSubitem(pItem, pParent);  
    signalsWidget->updateSorting();
}
void ExpertDiscoveryView::sl_newFolder(const QString& folderName){
    CSFolder* pFolder = &(signalsWidget->getRootItem());
    CSFolder* pNewFolder = new CSFolder();
    if(folderName.isEmpty()){
        pNewFolder->setName(pFolder->makeUniqueFolderName());
    }else{
        pNewFolder->setName(folderName);
    }
    if (pFolder->addFolder( pNewFolder ) < 0) {
        delete pNewFolder;
        return;
    }
    EDPICSFolder* newFol = new EDPICSFolder(pNewFolder);
    EDProjectItem* pParent = signalsWidget->findEDItem(pFolder);
    signalsWidget->addSubitem(newFol, pParent);  
    signalsWidget->updateSorting();
}

void ExpertDiscoveryView::sl_optimizeRecBound(){
    d.optimizeRecognizationBound();
    propWidget->updateCurrentProperties();
}

void ExpertDiscoveryView::sl_setRecBound(){
    d.setRecBound();
    propWidget->updateCurrentProperties();
}

void ExpertDiscoveryView::sl_extractSignals(){
    Task *tasks = new Task("Extracting signals", TaskFlag_NoRun);
    ExpertDiscoverySignalExtractorTask *t = new ExpertDiscoverySignalExtractorTask(&d);
    extrTask = t;
    connect(t, SIGNAL(si_newSignalReady(DDisc::Signal* , CSFolder* )), SLOT(sl_newSignalReady(DDisc::Signal* , CSFolder* )));
    connect(t, SIGNAL(si_newFolder(const QString&)), SLOT(sl_newFolder(const QString&)));
    tasks->addSubTask(t);
    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
}

void ExpertDiscoveryView::sl_treeItemSelChanged(QTreeWidgetItem* tItem){

    if(!tItem){
        propWidget->sl_treeSelChanged(NULL);
        return;
    }

    EDProjectItem* pItem = dynamic_cast<EDProjectItem*>(tItem);
    if(!pItem){
        propWidget->sl_treeSelChanged(NULL);
        curPS = NULL;
        return;
    }

    switch(pItem->getType()){
       case PIT_CS:
       case PIT_CSN_WORD:
       case PIT_CSN_INTERVAL:
       case PIT_CSN_REPETITION:
       case PIT_CSN_DISTANCE:
       case PIT_CSN_MRK_ITEM:
       case PIT_MRK_ITEM:{
           //check if running!!!!
           if(curTask){
               if(curTask->isFinished()){
                   curTask = new ExpertDiscoveryUpdateSelectionTask(this, tItem);
                   connect( curTask, SIGNAL( si_stateChanged() ), SLOT( sl_updateTaskFinished() ) );
                   //signalsWidget->setEnabled(false);
                   AppContext::getTaskScheduler()->registerTopLevelTask(curTask);
               }else{
                    return;
               }
           }else{
               curTask = new ExpertDiscoveryUpdateSelectionTask(this, tItem);
               connect( curTask, SIGNAL( si_stateChanged() ), SLOT( sl_updateTaskFinished() ) );
               //signalsWidget->setEnabled(false);
               AppContext::getTaskScheduler()->registerTopLevelTask(curTask);
           }

//            ExpertDiscoveryUpdateSelectionTask* t = new ExpertDiscoveryUpdateSelectionTask(this, tItem);
//            AppContext::getTaskScheduler()->registerTopLevelTask(t);


           //            EDPICSNode* pPICSN = dynamic_cast<EDPICSNode*>(pItem);
           // 
           //            if (curPS == pPICSN->getProcessedSignal(d)) { //to separate process
           //                updatePS = false;
           //            }
           //            else {
           //                mutex.lock();
           //                curPS = pPICSN->getProcessedSignal(d);
           //                updatePS = true;
           //                mutex.unlock();
           //            }
           // 
           //            if (curPS == NULL) {
           //                updateAnnotations();
           //                updatePS = false;
           //                propWidget->sl_treeSelChanged(pItem);
           //                return;
           //            }
           // 
           //            if(updatePS){
           //                 updateAnnotations();
           //                 updatePS = false;
           //            }
           break;
                         }
       default:
           //mutex.lock();
           curPS = NULL;
           propWidget->sl_treeSelChanged(tItem);
           //mutex.unlock();
    }

    // 
}

void ExpertDiscoveryView::sl_updateTaskFinished(){
    ExpertDiscoveryUpdateSelectionTask* task = dynamic_cast<ExpertDiscoveryUpdateSelectionTask*>(sender());
    if(task && (curTask == task) && task->isFinished()){
        curTask = NULL;
    }
}

void ExpertDiscoveryView::updateAnnotations(){

    if(!currentAdv || !signalsWidget->isEnabled() || signalsWidget->isUpdatingItem()){
        return;
    }

    edAutoAnnotationsUpdater->setEDProcSignals(curPS);
    //AppContext::getAutoAnnotationsSupport()->registerAutoAnnotationsUpdater(edAutoAnnotationsUpdater);

    foreach(ADVSequenceObjectContext* sctx, currentAdv->getSequenceContexts()){    
        AutoAnnotationUtils::triggerAutoAnnotationsUpdate(sctx, "ExpertDiscover Signals");
    }

    //AppContext::getAutoAnnotationsSupport()->unregisterAutoAnnotationsUpdater(edAutoAnnotationsUpdater);
}

void ExpertDiscoveryView::createEDSequence(){
    if(!currentAdv){
        return;
    }

    ADVSequenceObjectContext* seqInfocus =  currentAdv->getSequenceInFocus();
    if(!seqInfocus){
        return;
    }
    U2SequenceObject* dnaSeqObject = seqInfocus->getSequenceObject();
    const QString& seqName = dnaSeqObject->getSequenceName();

    SequenceType seqType = d.getSequenceTypeByName(seqName);

    if(seqType != UNKNOWN_SEQUENCE && curEDsequence !=NULL){
        delete curEDsequence;
        curEDsequence = NULL;
    }

    switch(seqType){
        case POSITIVE_SEQUENCE:
            curEDsequence = new EDPISequence(d.getPosSeqBase(), d.getPosSeqBase().getObjNo(seqName.toStdString().c_str()), d);
            break;

        case NEGATIVE_SEQUENCE:
            curEDsequence = new EDPISequence(d.getNegSeqBase(), d.getNegSeqBase().getObjNo(seqName.toStdString().c_str()), d);
            break;

        case CONTROL_SEQUENCE:
            curEDsequence = new EDPIControlSequence(d.getConSeqBase(), d.getConSeqBase().getObjNo(seqName.toStdString().c_str()), d);
            break;
    }

    updateEDSequenceProperties();

}
void ExpertDiscoveryView::updateEDSequenceProperties(){
    if(!curEDsequence){
        return;
    }
    propWidget->sl_treeSelChanged(curEDsequence);
}

U2SequenceObject* ExpertDiscoveryView::getSeqObjectFromEDSequence(EDPISequence* sItem){
    U2SequenceObject* dnaSeqObj = NULL;
    bool seqFound = false;
    foreach(GObject* obj, edObjects){
        dnaSeqObj = dynamic_cast<U2SequenceObject*>(obj);
        if(dnaSeqObj){
            if(dnaSeqObj->getSequenceName().compare(sItem->getSequenceName(), Qt::CaseInsensitive) == 0){
                seqFound = true;
                return dnaSeqObj;
            }
        }
    }
    if(!seqFound){ 
        SequenceType sType = d.getSequenceTypeByName(sItem->getSequenceName());
        Document* curDoc = NULL;
        switch(sType){
            case POSITIVE_SEQUENCE:
                if(!posUDoc){
                    posUDoc = createUDocument(sType);
                    posUDoc->setName("Positive");
                }
                curDoc = posUDoc;
                break;
            case NEGATIVE_SEQUENCE:
                if(!negUDoc){
                    negUDoc = createUDocument(sType);
                    negUDoc->setName("Negative");
                }
                curDoc = negUDoc;
                break;
            case CONTROL_SEQUENCE:
                if(!conUDoc){
                    conUDoc = createUDocument(sType);
                    conUDoc->setName("Control");
                }
                curDoc = conUDoc;
                break;
            default:
                return NULL;
        }
        QByteArray seqarray  = QByteArray(sItem->getSequenceCode().toAscii());
        DNASequence dnaseq (sItem->getSequenceName(), seqarray);
        dnaseq.alphabet = AppContext::getDNAAlphabetRegistry()->findById(BaseDNAAlphabetIds::NUCL_DNA_EXTENDED());
        TaskStateInfo stateInfo;
        U2EntityRef seqRef = U2SequenceUtils::import(curDoc->getDbiRef(), dnaseq, stateInfo);
        if (stateInfo.isCoR()) {
            return NULL;
        } 
        if(!curDoc){
            return NULL;
        }
        U2SequenceObject* danseqob = new U2SequenceObject(sItem->getSequenceName(), seqRef);
        curDoc->addObject(danseqob);
        edObjects.append(danseqob);
        return danseqob;
    }   
    return NULL;
}

Document* ExpertDiscoveryView::createUDocument(SequenceType sType){

    QString baseName = AppContext::getAppSettings()->getUserAppsSettings()->getCurrentProcessTemporaryDirPath("expert_discovery");
    switch(sType){
        case POSITIVE_SEQUENCE:
            baseName.append("_Positive");
            break;
        case NEGATIVE_SEQUENCE:
            baseName.append("_Negative");
            break;
        case CONTROL_SEQUENCE:
            baseName.append("_Control");
            break;
    }
    QString suffix = QString(".fa");
    baseName.append(suffix);
    GUrl url(baseName);
    IOAdapterFactory* iof = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(IOAdapterUtils::url2io(url));
    DocumentFormat* dformat = AppContext::getDocumentFormatRegistry()->getFormatById(BaseDocumentFormats::FASTA);
    U2OpStatus2Log os;
    Document* doc = dformat->createNewLoadedDocument(iof, url, os);
    return doc;
}

void ExpertDiscoveryView::sl_sequenceItemSelChanged(ADVSequenceWidget* seqWidget){
    createEDSequence();    
}

void ExpertDiscoveryView::sl_treeWidgetMarkup(bool isLetters){
    if(isLetters){
        extractSignalsAction->setEnabled(true);
    }else{
        sl_showExpertDiscoveryPosNegMrkDialog();
    }
}

void ExpertDiscoveryView::sl_treeWidgetAddMarkup(){
    ExpertDiscoveryMarkupTask *addToMarkupTask = new ExpertDiscoveryMarkupTask(d, curPS);
    connect(addToMarkupTask, SIGNAL( si_stateChanged() ), SLOT( sl_updateMarking() ) );
    AppContext::getTaskScheduler()->registerTopLevelTask(addToMarkupTask);
}

void ExpertDiscoveryView::sl_generateFullReport(){
    if(d.getSelectedSignalsContainer().GetSelectedSignals().size() == 0){
        QMessageBox mb(QMessageBox::Critical, tr("Error"), tr("No signals are selected to generate report"));
        mb.exec();
    }else{
        d.generateRecognitionReportFull();
    }
}
void ExpertDiscoveryView::sl_updateMarking(){
    ExpertDiscoveryMarkupTask *markuptask = qobject_cast<ExpertDiscoveryMarkupTask*>(sender());
    if (!markuptask || !markuptask->isFinished()) {
        return;
    }
    signalsWidget->updateMarkup();
}

void ExpertDiscoveryView::sl_showSequence(){
    EDPISequence* sItem = dynamic_cast<EDPISequence*>(signalsWidget->currentItem());
    if(!sItem){
        return;
    }

    U2SequenceObject* dnaSeqObj = getSeqObjectFromEDSequence(sItem);

    QList<EDPISequence*> selSeqList = d.getSelectetSequencesList();
    d.clearSelectedSequencesList();
    foreach(EDPISequence* curS, selSeqList){
        signalsWidget->updateItem(curS);
    }
    d.addSequenceToSelected(sItem);
    QList<U2SequenceObject*> listdna;
    listdna.append(dynamic_cast<U2SequenceObject*>(dnaSeqObj));
    AnnotatedDNAView* danadv = new AnnotatedDNAView(dnaSeqObj->getSequenceName(),listdna);
    initADVView(danadv);
    signalsWidget->updateItem(sItem);
}

#define MAX_SEQUENCES_COUNT_ON_WIDGET 50
void ExpertDiscoveryView::sl_addToShown(){
    EDPISequence* sItem = dynamic_cast<EDPISequence*>(signalsWidget->currentItem());
    if(!sItem){
        return;
    }

    U2SequenceObject* dnaSeqObj = getSeqObjectFromEDSequence(sItem);
    if(currentAdv){
        if(currentAdv->getSequenceContexts().size() >= MAX_SEQUENCES_COUNT_ON_WIDGET){
            return;
        }
    }
    //currentAdv->addObject(dnaSeqObj); 

    //auto annotations bug

    d.addSequenceToSelected(sItem);
    QList<U2SequenceObject*> listdna;
    listdna.append(dynamic_cast<U2SequenceObject*>(dnaSeqObj));
    if(currentAdv){
        foreach(ADVSequenceObjectContext* curSoc, currentAdv->getSequenceContexts()){
            listdna.append(curSoc->getSequenceObject());
        }
    }
    AnnotatedDNAView* danadv = new AnnotatedDNAView(dnaSeqObj->getSequenceName(),listdna);
    initADVView(danadv);
    signalsWidget->updateItem(sItem);

}
#define MAX_SEQUENCES_COUNT_TO_ONCE_DISPLAY 25
void ExpertDiscoveryView::sl_showFirstSequences(){
    EDPISequenceBase* sItem = dynamic_cast<EDPISequenceBase*>(signalsWidget->currentItem());
    if(!sItem){
        return;
    }

    clearSequencesView();
    QList<EDPISequence*> selSeqList = d.getSelectetSequencesList();
    d.clearSelectedSequencesList();
    foreach(EDPISequence* curS, selSeqList){
        signalsWidget->updateItem(curS);
    }

    const SequenceBase& seqBase = sItem->getSequenceBase();
    int baseSize = seqBase.getSize();
    int widgetItemsCount = baseSize < MAX_SEQUENCES_COUNT_TO_ONCE_DISPLAY ? baseSize : MAX_SEQUENCES_COUNT_TO_ONCE_DISPLAY;

    QList<U2SequenceObject*> listdna;

    for(int i = 0; i < widgetItemsCount; i++){
        QTreeWidgetItem* tItem = sItem->child(i);
        EDPISequence* pItemSequence = dynamic_cast<EDPISequence*>(tItem);
        if(!pItemSequence){
            continue;
        }
        U2SequenceObject* dnaSeqObj = getSeqObjectFromEDSequence(pItemSequence);
        d.addSequenceToSelected(pItemSequence);
        signalsWidget->updateItem(pItemSequence);
        listdna.append(dynamic_cast<U2SequenceObject*>(dnaSeqObj));
    }


    AnnotatedDNAView* danadv = new AnnotatedDNAView("Base",listdna);
    initADVView(danadv);
    
}
void ExpertDiscoveryView::sl_clearDisplayed(){
    clearSequencesView();
    QList<EDPISequence*> selSeqList = d.getSelectetSequencesList();
    d.clearSelectedSequencesList();
    foreach(EDPISequence* curS, selSeqList){
        signalsWidget->updateItem(curS);
    }
}

void ExpertDiscoveryView::clearSequencesView()
{
    if(currentAdv){
        foreach(ADVSequenceObjectContext* sctx, currentAdv->getSequenceContexts()){    
            AutoAnnotationsADVAction* aaAction = AutoAnnotationUtils::findAutoAnnotationADVAction( sctx );
            assert(aaAction);
            AutoAnnotationObject* aaobj = aaAction->getAAObj();
            assert(aaobj);
            disconnect(aaobj, SIGNAL(si_updateStarted()),this,  SLOT(sl_autoAnnotationUpdateStarted()));
            disconnect(aaobj, SIGNAL(si_updateFinshed()), this, SLOT(sl_autoAnnotationUpdateFinished()));
        }
        disconnect(currentAdv, SIGNAL( si_focusChanged(ADVSequenceWidget*, ADVSequenceWidget*) ), this,  SLOT( sl_sequenceItemSelChanged(ADVSequenceWidget*) ));
        foreach(GObject* obj, objects){
            removeObject(obj);
        }

        delete currentAdv->getWidget();
        delete currentAdv;
        currentAdv = NULL;
    }
}

ExpertDiscoveryViewWindow::ExpertDiscoveryViewWindow(GObjectView* view, const QString& viewName, bool persistent)
:GObjectViewWindow(view, viewName, persistent){
    
}
void ExpertDiscoveryViewWindow::setupMDIToolbar(QToolBar* tb){
    ExpertDiscoveryView* curEdView = dynamic_cast<ExpertDiscoveryView*>(view);
    assert(curEdView);
    tb->addAction(curEdView->getNewDocAction());
    tb->addAction(curEdView->getOpenDocAction());
    tb->addAction(curEdView->getSaveDocAction());
    tb->addSeparator();
    tb->addAction(curEdView->getLoadMarkupAction());
    tb->addSeparator();
    tb->addAction(curEdView->getLoadControlSeqAction());
    //tb->addAction(curEdView->getLoadControlMarkupAction());
    tb->addSeparator();
    tb->addAction(curEdView->getExtractSignalsAction());
    tb->addSeparator();
    tb->addAction(curEdView->getSetUpRecBoundAction());
    //tb->addAction(curEdView->getOptimizeRecBoundAction()); 
    tb->addSeparator();
    tb->addAction(curEdView->getGenerateFullReportAction());
    
}

bool ExpertDiscoveryViewWindow::onCloseEvent(){
    ExpertDiscoveryView* curEdView = dynamic_cast<ExpertDiscoveryView*>(view);
    assert(curEdView);
    Task* t = curEdView->getExtractTask();
    if(t && (t->isRunning())){
        QMessageBox mb(QMessageBox::Critical, tr("Closing error"), tr("There are unfinished extracting tasks. Cancel them before closing"));
        mb.exec();
        return false;
    }
    if(curEdView->askForSave()){
        curEdView->getSaveDocAction()->trigger();
        return false;
    }
    
    return true;
}
}//namespace
