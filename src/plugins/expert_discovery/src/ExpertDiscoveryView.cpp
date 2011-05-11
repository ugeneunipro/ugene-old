#include "ExpertDiscoveryView.h"
#include "ExpertDiscoveryTask.h"
#include "ExpertDiscoveryPosNegDialog.h"
#include "ExpertDiscoveryControlDialog.h"
#include "ExpertDiscoveryExtSigWiz.h"
#include "ExpertDiscoveryPlugin.h"

#include <U2View/ADVUtils.h>
#include <U2View/AnnotatedDNAView.h>
#include <U2View/AnnotatedDNAViewFactory.h>

#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

#include <U2Core/GObjectSelection.h>


namespace U2{
using namespace DDisc;
ExpertDiscoveryView::ExpertDiscoveryView(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p)
:GObjectView(factoryId, viewName, p)
,wizzard(false)
,splitter(NULL)
,currentAdv(NULL){

    createActions();

}

QWidget* ExpertDiscoveryView::createWidget(){
    splitter = new QSplitter(Qt::Horizontal);

    signalsWidget = new EDProjectTree(splitter, d);
    signalsWidget->setHeaderLabel("Items");

    propWidget = new EDPropertiesTable(splitter);


    //    seqWidget = new W


    splitter->addWidget(signalsWidget);
    splitter->addWidget(propWidget);
    //  splitter->addWidget(seqWidget);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0, 0, 0, 0);
    //setLayout(layout);

    connect(signalsWidget, SIGNAL( 	itemPressed ( QTreeWidgetItem * , int )), propWidget, SLOT(sl_treeSelChanged(QTreeWidgetItem *)));
    connect(signalsWidget, SIGNAL( 	itemActivated ( QTreeWidgetItem * , int )), propWidget, SLOT(sl_treeSelChanged(QTreeWidgetItem *)));
    connect(signalsWidget, SIGNAL( 	currentItemChanged ( QTreeWidgetItem * , QTreeWidgetItem * ) ), propWidget, SLOT(sl_treeSelChanged(QTreeWidgetItem *)));
    connect(propWidget, SIGNAL(si_propChanged(EDProjectItem*, const EDPIProperty*, QString )), signalsWidget, SLOT(sl_propChanged(EDProjectItem* , const EDPIProperty* , QString )));

    GObjectViewFactory* factory = AppContext::getObjectViewFactoryRegistry()->getFactoryById("ED");
    ExpertDiscoveryViewFactory* edFactory = dynamic_cast<ExpertDiscoveryViewFactory*>(factory);
    assert(edFactory);
    connect(edFactory, SIGNAL(si_newTaskCreation(Task* )), SLOT(sl_newViewTask(Task*)));

    //test
    //     ExpertDiscoverySignalExtractorTask se(&d);
    //     connect(&se, SIGNAL(si_newSignalReady(DDisc::Signal* , CSFolder* )), SLOT(sl_newSignalReady(DDisc::Signal* , CSFolder* )));
    //     se.run();

    ///test
    return splitter;
}

// void ExpertDiscoveryView::setupMDIToolbar(QToolBar* tb){
//     tb->addAction(newDoc);
//     tb->addAction(loadControlSeqAction);
//     tb->addAction(extractSignalsAction);
//     tb->addAction(setUpRecBound);
//     tb->addAction(optimizeRecBound);
// 
// }

void ExpertDiscoveryView::insertSeqView( QWidget* view ){
   //seqWidget = view;  
   //seqWidget = new ExpertDiscoveryADVSplitWidget(view);
   //view->insertWidgetIntoSplitter(seqWidget);
   splitter->addWidget(view);
}

void ExpertDiscoveryView::createActions(){
    newDoc = new QAction(tr("New ED Document"), this);
    //newDoc->setIcon();
    connect(newDoc, SIGNAL(triggered()), SLOT(sl_newDoc()));

    setUpRecBound = new QAction(tr("Set recognization bound"), this);
    connect(setUpRecBound, SIGNAL(triggered()), SLOT(sl_setRecBound()));
    
    optimizeRecBound = new QAction(tr("Optimize recognization bound"), this);
    connect(optimizeRecBound, SIGNAL(triggered()), SLOT(sl_optimizeRecBound()));

    loadControlSeqAction = new QAction(tr("Load control sequences"), this);
    connect(loadControlSeqAction, SIGNAL(triggered()), SLOT(sl_showExpertDiscoveryControlDialog()));

    extractSignalsAction = new QAction(tr("Extract signals"), this);
    connect(extractSignalsAction, SIGNAL(triggered()), SLOT(sl_extractSignals()));

    setUpRecBound->setEnabled(false);
    optimizeRecBound->setEnabled(false);
    loadControlSeqAction->setEnabled(false);
    extractSignalsAction->setEnabled(false);
    
}

void ExpertDiscoveryView::sl_newDoc(){
    wizzard = true;

    setUpRecBound->setEnabled(false);
    optimizeRecBound->setEnabled(false);
    loadControlSeqAction->setEnabled(false);
    extractSignalsAction->setEnabled(false);

    d.setRecBound(0);

    sl_showExpertDiscoveryPosNegDialog();
}

void ExpertDiscoveryView::sl_showExpertDiscoveryPosNegDialog(){
	Task *tasks = new Task("Loading positive and negative sequences", TaskFlag_NoRun);

    ExpertDiscoveryPosNegDialog d(QApplication::activeWindow());
	if (d.exec()) {
	    if (!AppContext::getProject()) {
			QList<GUrl> emptyList;
			tasks->addSubTask( AppContext::getProjectLoader()->openProjectTask(emptyList, false) );
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

    if (loadTask->getStateInfo().hasErrors()) {
		ExpertDiscoveryErrors::fileOpenError();
        return;
    }

	QList <Document *> docs = loadTask->getDocuments();

	Q_ASSERT(!docs.isEmpty());
		Q_ASSERT(docs.count() > 1);

	Document * doc = docs.first();
	d.setPosBase(doc->getObjects());

    foreach(GObject* gobj, doc->getObjects()){
        //addObject(gobj);
        edObjects.push_back(gobj);
    }

	doc = docs.at(1);
	d.setNegBase(doc->getObjects());

    foreach(GObject* gobj, doc->getObjects()){
        //addObject(gobj);
        edObjects.push_back(gobj);
    }



//     //do smth here to add annotDNAviews on the widget
//     GObjectSelection os;
// 
//    // QList <Document *> docs = doc->getObjects();
//     foreach (Document* doc1, docs) {
//         os.addToSelection(doc1->getObjects());
//     }
// 
//     MultiGSelection ms; 
//     ms.addSelection(&os);
// 
//     GObjectViewFactoryRegistry* reg = AppContext::getObjectViewFactoryRegistry();
//     GObjectViewFactory* f = reg->getFactoryById(AnnotatedDNAViewFactory::ID);
// 
//     bool canCreate = f->canCreateView(ms);
//     if (canCreate) {
//         Task* crView= f->createViewTask(ms, false);
//         connect(crView,SIGNAL( si_stateChanged() ), SLOT( sl_testView() ) );
//         AppContext::getTaskScheduler()->registerTopLevelTask(crView);
//         //createdByWizard = true; // set flag that we need to show a dotplot settings dialog
//     }
    
    setUpRecBound->setEnabled(true);
    optimizeRecBound->setEnabled(true);
    loadControlSeqAction->setEnabled(true);
    extractSignalsAction->setEnabled(true);

    if(wizzard){
        //markup
        //sl_showExpertDiscoveryControlDialog();
    }

    wizzard = false;
}

void ExpertDiscoveryView::initADVView(AnnotatedDNAView* adv){
    if(!adv){
        return;
    }

    if(currentAdv){
        foreach(GObject* obj, objects){
            removeObject(obj);
        }

        delete currentAdv->getWidget();
        delete currentAdv;
    }    
        foreach(GObject* gobj, adv->getObjects()){
            addObject(gobj);
        }
        adv->setClosingInterface(closeInterface);
        currentAdv = adv;
        splitter->addWidget(adv->getWidget());
}

void ExpertDiscoveryView::sl_testView(){
    Task *loadTask = qobject_cast<Task*>(sender());
    if (!loadTask || !loadTask->isFinished()) {
        return;
    }

    if (loadTask->getStateInfo().hasErrors()) {
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

    if (loadTask->getStateInfo().hasErrors()) {
		ExpertDiscoveryErrors::fileOpenError();
        return;
    }

    

	QList <Document *> docs = loadTask->getDocuments();

	Q_ASSERT(!docs.isEmpty());

	Document * doc = docs.first();

    d.clearContrBase();
    d.clearContrAnnot();

	d.setConBase(doc->getObjects());

    foreach(GObject* gobj, doc->getObjects()){
        edObjects.push_back(gobj);
    }

    if (d.isLettersMarkedUp() && d.getConSeqBase().getSize() != 0)
        d.markupLetters(d.getConSeqBase(), d.getConMarkBase());

}

void ExpertDiscoveryView::sl_newSignalReady(DDisc::Signal* signal, CSFolder* folder){
    /*SetModifiedFlag();
    CCSFolder *pFolder = (CCSFolder *) wParam;
    Signal* pSignal = (Signal*) lParam;
    pSignal->setName(pFolder->MakeUniqueSignalName().GetString());
    pFolder->AddSignal(pSignal);
    CProjectItem* pParent = FindItem(m_RootItem.FindItemConnectedTo(pFolder));
    CPICS* pItem = new CPICS(pSignal);
    pParent->AddSubitem(pItem);
    UpdateAllViews(NULL, ITEM_ADDED, pItem);*/
    

    //void* p =  qVariantValue<void*>(signal);
    //Signal* pSignal = (Signal*) p;
   // signal->setName(folder->makeUniqueSignalName().toStdString());
    //folder->addSignal(signal);

    //test 
     Signal* ps = signal;
    CSFolder *pFolder = folder;
    ps->setName(pFolder->makeUniqueSignalName().toStdString());
    pFolder->addSignal(ps);
    EDProjectItem* pParent = signalsWidget->findEDItem(pFolder);
    EDPICS* pItem = new EDPICS(ps);
    signalsWidget->addSubitem(pItem, pParent);    

    //test
    propWidget->representPIProperties(pParent);
    //test
    //propWidget->representPIProperties(NULL);

}

void ExpertDiscoveryView::sl_optimizeRecBound(){
    d.optimizeRecognizationBound();
}

void ExpertDiscoveryView::sl_setRecBound(){
    d.setRecBound();
}

void ExpertDiscoveryView::sl_extractSignals(){
	Task *tasks = new Task("Extracting signals", TaskFlag_NoRun);
    ExpertDiscoverySignalExtractorTask *t = new ExpertDiscoverySignalExtractorTask(&d);
    connect(t, SIGNAL(si_newSignalReady(DDisc::Signal* , CSFolder* )), SLOT(sl_newSignalReady(DDisc::Signal* , CSFolder* )));
    tasks->addSubTask(t);
    AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
}

void ExpertDiscoveryView::sl_treeItemSelChanged(QTreeWidgetItem* tItem){

//     if (!signalsWidget->currentItem())
//         return;
// 
//     switch (m_pCurrentItem->GetType()) {
//    case PIT_CONTROLSEQUENCE:
//        {
//            if (pDoc->LargeSequenceMode())
//                m_nWindowSize = pDoc->GetWindowSize();
//            else 
//                m_nWindowSize = 1;
//            m_CurrentView = VIEW_SEQUENCE;
//            OnDrawSequence(pDC);
//            break;
//        }
//    case PIT_SEQUENCE:
//        {
//            m_nWindowSize = 1;
//            m_CurrentView = VIEW_SEQUENCE;
//            OnDrawSequence(pDC);
//            break;
//        }
//    case PIT_POSSEQUENCEBASE:
//    case PIT_NEGSEQUENCEBASE:
//    case PIT_CONTROLSEQUENCEBASE:
//        {
//            m_CurrentView = VIEW_SEQUENCEBASE;
//            CRect rect;
//            GetClientRect(rect);
//            rect.top   += 10;
//            rect.right -= 10;
//            rect.left  += 10;
// 
//            CFont *pOldFont = pDC->SelectObject(CFont::FromHandle((HFONT)GetStockObject(OEM_FIXED_FONT)));
// 
//            CPISequenceBase* pPI = (CPISequenceBase*) m_pCurrentItem;
//            int nSeqNum = pPI->GetSubitemsNumber();
//            SIZE szText;
//            for (int nSeq=0; nSeq<nSeqNum; nSeq++) {
//                CPISequence* pPISeq = (CPISequence*) pPI->GetSubitem(nSeq);
//                CString strName = "> " + pPISeq->GetSequenceName();
//                COLORREF crTextOld = pDC->SetTextColor(m_colorSeqName);
//                do {
//                    int nCharNum = 0;
//                    GetTextExtentExPoint(pDC->GetSafeHdc(), strName, strName.GetLength(), rect.Width(),&nCharNum,NULL,&szText);
//                    if (nCharNum == 0)
//                        return;
//                    pDC->TextOut(rect.left, rect.top, strName.Left(nCharNum));
//                    rect.top += szText.cy + 3;
//                    strName = strName.Right(strName.GetLength() - nCharNum);
//                } while (!strName.IsEmpty());
//                pDC->SetTextColor(crTextOld);
// 
//                CString strCode = pPISeq->GetSequenceCode();
//                do {
//                    int nCharNum = 0;
//                    GetTextExtentExPoint(pDC->GetSafeHdc(), strCode, strCode.GetLength(), rect.Width(),&nCharNum,NULL,&szText);
//                    if (nCharNum == 0)
//                        return;
//                    pDC->TextOut(rect.left, rect.top, strCode.Left(nCharNum));
//                    rect.top += szText.cy + 3;
//                    strCode = strCode.Right(strCode.GetLength() - nCharNum);
//                } while (!strCode.IsEmpty());
//                rect.top += 10;
//            }
//            pDC->SelectObject(pOldFont);
//            SetScrollSizes(MM_TEXT, CSize(rect.Width(), rect.top + 10));
//            break;
//        }
//    case PIT_CS:
//    case PIT_CSN_WORD:
//    case PIT_CSN_INTERVAL:
//    case PIT_CSN_REPETITION:
//    case PIT_CSN_DISTANCE:
//    case PIT_CSN_MRK_ITEM:
//    case PIT_MRK_ITEM:
//        m_CurrentView = VIEW_CSN;
//        OnDrawCSN(pDC);
//        break;
//    default:
//        m_CurrentView = VIEW_NONE;
//        SetScrollSizes(MM_TEXT, CSize(0,0));
//     }
    propWidget->sl_treeSelChanged(tItem);
}

ExpertDiscoveryADVSplitWidget::ExpertDiscoveryADVSplitWidget(AnnotatedDNAView* view): ADVSplitWidget(view){
    
}




ExpertDiscoveryViewWindow::ExpertDiscoveryViewWindow(GObjectView* view, const QString& viewName, bool persistent)
:GObjectViewWindow(view, viewName, persistent){
    
}
void ExpertDiscoveryViewWindow::setupMDIToolbar(QToolBar* tb){
    ExpertDiscoveryView* curEdView = dynamic_cast<ExpertDiscoveryView*>(view);
    assert(curEdView);
    tb->addAction(curEdView->getNewDocAction());
    tb->addAction(curEdView->getLoadControlSeqAction());
    tb->addAction(curEdView->getExtractSignalsAction());
    tb->addAction(curEdView->getSetUpRecBoundAction());
    tb->addAction(curEdView->getOptimizeRecBoundAction());
}


ExpertDiscoveryView1::ExpertDiscoveryView1(GObjectViewFactoryId factoryId, const QString& viewName, QObject* p)
:GObjectView(factoryId,viewName,p){


}
QWidget* ExpertDiscoveryView1::createWidget(){
    return new QSplitter();
}
}//namespace