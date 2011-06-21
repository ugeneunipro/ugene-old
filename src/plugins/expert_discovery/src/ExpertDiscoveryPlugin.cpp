#include "ExpertDiscoveryPlugin.h"
#include "ExpertDiscoveryView.h"
#include "ExpertDiscoveryViewCtx.h"
#include "ExpertDiscoveryTask.h"

#include <U2Core/DNASequence.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/AppContext.h>
#include <U2Core/AutoAnnotationsSupport.h>
#include <U2Gui/MainWindow.h>
#include <U2View/AnnotatedDNAViewFactory.h>


#include <U2Core/ProjectModel.h>
#include <U2Core/DocumentModel.h>

#include <U2Core/SelectionUtils.h>
#include <U2Core/DocumentSelection.h>

#include <U2Core/AnnotationTableObject.h>
#include <U2Core/GObjectTypes.h>
#include <U2Core/GObjectRelationRoles.h>
#include <U2Core/GObjectUtils.h>
#include <U2Core/GObjectSelection.h>

#include<QtGui/QMenu>
#include <QtGui/QMessageBox>


namespace U2 {

extern "C" Q_DECL_EXPORT Plugin * U2_PLUGIN_INIT_FUNC() {
    if (AppContext::getMainWindow()) {
        ExpertDiscoveryPlugin * plug = new ExpertDiscoveryPlugin();
        return plug;
    }
    return NULL;
}

ExpertDiscoveryPlugin::ExpertDiscoveryPlugin() : Plugin( tr("Expert Discovery"), tr("Expert Discovery plugin") ),viewCtx(NULL), delay(false) {
   
    connect( AppContext::getPluginSupport(), SIGNAL( si_allStartUpPluginsLoaded() ), SLOT(sl_initExpertDiscoveryViewCtx()));

    windowId = 0;

    ExpertDiscoveryViewFactory* edFactory = new ExpertDiscoveryViewFactory("ED", "ED", this);
    AppContext::getObjectViewFactoryRegistry()->registerGObjectViewFactory(edFactory);

}

void ExpertDiscoveryPlugin::sl_initExpertDiscoveryViewCtx() {
     if (AppContext::getMainWindow()) {
          viewCtx = new ExpertDiscoveryViewCtx(this);
          Q_ASSERT(viewCtx);
          viewCtx->init();

          //AppContext::getAutoAnnotationsSupport()->registerAutoAnnotationsUpdater(new ExpertDiscoverySignalsAutoAnnotationUpdater );
          //change the icon
          QAction* action = new QAction(  tr("Expert Discovery (alpha)"), this );
          connect( action, SIGNAL( triggered() ), SLOT( sl_expertDiscoveryView() ) );

          AppContext::getMainWindow()->getTopLevelMenu( MWMENU_TOOLS )->addAction( action );
     }
}

void ExpertDiscoveryPlugin::sl_expertDiscoveryView(){

 
    //create project here
    if (!AppContext::getProject()) {
        Task *tasks = new Task("Creating empty project", TaskFlag_NoRun);
        Task* t = AppContext::getProjectLoader()->createNewProjectTask();
        connect( t, SIGNAL( si_stateChanged() ), SLOT( sl_expertDiscoveryViewDelay() ) );
        tasks->addSubTask(t);        
        AppContext::getTaskScheduler()->registerTopLevelTask(tasks);
        delay = true;
    }else{
        delay = false;
        sl_expertDiscoveryViewDelay();
    }
 
}

void ExpertDiscoveryPlugin::sl_expertDiscoveryViewDelay(){
    if(delay){
        if(!AppContext::getProject()){
            return;
        }
        delay = false;
    }

    MWMDIManager* mdi = AppContext::getMainWindow()->getMDIManager();
    MWMDIWindow* mdiWindow = mdi->getWindowById(windowId);
    if (mdiWindow==NULL) {

        ExpertDiscoveryView* v = new ExpertDiscoveryView("ED", "EDView", this);
        v->addObjectHandler(viewCtx);
        ExpertDiscoveryViewWindow* wind = new ExpertDiscoveryViewWindow(v, "Expert Discovery");
        windowId = wind->getId();
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(wind);
        AppContext::getMainWindow()->getMDIManager()->activateWindow(wind);

    }else{
        AppContext::getMainWindow()->getMDIManager()->activateWindow(mdiWindow);
    }
    
}

const GObjectViewFactoryId ExpertDiscoveryViewFactory::ID("ED");

ExpertDiscoveryViewFactory::ExpertDiscoveryViewFactory(GObjectViewFactoryId id, const QString& name, QObject* p)
:GObjectViewFactory(id,name,p){

}

bool ExpertDiscoveryViewFactory::canCreateView(const MultiGSelection& multiSelection) {
    //return true if
    
    //0. All the documents in the selection are from ExperDiscoveryData
    //1. selection has loaded of unloaded DNA sequence object
    //2. selection has any object with SEQUENCE relation to DNA sequence object that is in the project
    //3. selection has document that have sequence object or object assosiated with sequence

    //0.
    if(!checkSelection(multiSelection)){
        return false;
    }
    //1.
    QList<GObject*> selectedObjects = SelectionUtils::findObjects("", &multiSelection, UOF_LoadedAndUnloaded);
    QList<GObject*> selectedSequences = GObjectUtils::select(selectedObjects, GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded);
    if (!selectedSequences.isEmpty()) {
        return true;
    }

    //2.
    QList<GObject*> objectsWithSeqRelation = GObjectUtils::selectObjectsWithRelation(selectedObjects, GObjectTypes::SEQUENCE, 
        GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);
    if (!objectsWithSeqRelation.isEmpty()) {
        return true;
    }

    //3.
    const DocumentSelection* ds = qobject_cast<const DocumentSelection*>(multiSelection.findSelectionByType(GSelectionTypes::DOCUMENTS));
    if (ds == NULL) {
        return false;
    }
    foreach(Document* doc, ds->getSelectedDocuments()) {
        if (!doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded).isEmpty()) {
            return true;
        }    
        objectsWithSeqRelation = GObjectUtils::selectObjectsWithRelation(doc->getObjects(), 
            GObjectTypes::SEQUENCE, GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);

        if (!objectsWithSeqRelation.isEmpty()) {
            return true;
        }
    }

    return false;
}


Task* ExpertDiscoveryViewFactory::createViewTask(const MultiGSelection& multiSelection, bool single /*=false*/) {
    Q_UNUSED(single);
    QList<GObject*> objectsToOpen = SelectionUtils::findObjects(GObjectTypes::SEQUENCE, &multiSelection, UOF_LoadedAndUnloaded);

    QList<GObject*> selectedObjects = SelectionUtils::findObjects("", &multiSelection, UOF_LoadedAndUnloaded);
    QList<GObject*> objectsWithSequenceRelation = GObjectUtils::selectObjectsWithRelation(selectedObjects, 
        GObjectTypes::SEQUENCE, GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true);

    objectsToOpen.append(objectsWithSequenceRelation);

    const DocumentSelection* ds = qobject_cast<const DocumentSelection*>(multiSelection.findSelectionByType(GSelectionTypes::DOCUMENTS));
    if (ds != NULL) {
        foreach(Document* doc, ds->getSelectedDocuments()) {
            objectsToOpen.append(doc->findGObjectByType(GObjectTypes::SEQUENCE, UOF_LoadedAndUnloaded));
            objectsToOpen.append(GObjectUtils::selectObjectsWithRelation(doc->getObjects(), GObjectTypes::SEQUENCE, 
                GObjectRelationRole::SEQUENCE, UOF_LoadedAndUnloaded, true));
        }
    }

   ExpertDiscoveryCreateViewTask* task = new ExpertDiscoveryCreateViewTask(objectsToOpen);
   emit si_newTaskCreation(task);
    return task;
}

bool ExpertDiscoveryViewFactory::checkSelection(const MultiGSelection& multiSelection){
    QList<GObjectViewWindow*> views;
    QList<MWMDIWindow*> windows = AppContext::getMainWindow()->getMDIManager()->getWindows();
    GObjectSelection* objectsSelection = (GObjectSelection*)multiSelection.findSelectionByType(GSelectionTypes::GOBJECTS);
    if (objectsSelection!=NULL) {
        QSet<GObject*> objectsInSelection = objectsSelection->getSelectedObjects().toSet();
        foreach(MWMDIWindow* w, windows) {
            GObjectViewWindow* ov = qobject_cast<GObjectViewWindow*>(w);
            if (ov==NULL) {
                continue;
            }
            if (ov->getViewFactoryId() != ID) {
                continue;
            }
            ExpertDiscoveryViewWindow* edViewWindow = dynamic_cast<ExpertDiscoveryViewWindow*>(ov);
            if(!edViewWindow){
                continue;
            }
            ExpertDiscoveryView* edView = dynamic_cast<ExpertDiscoveryView*>(edViewWindow->getObjectView());
            if(!edView){
                continue;
            }
            const QList<GObject*>& viewObjects = edView->getEDObjects();
            bool allIn = true;
            foreach(GObject* o, objectsInSelection) {
                if (!viewObjects.contains(o)) {
                    allIn = false;
                    break;
                }
            }
            return allIn;
        }
    }
    return false;
}

// bool ExpertDiscoveryViewFactory::isStateInSelection(const MultiGSelection& multiSelection, const QVariantMap& stateData) {
//     AnnotatedDNAViewState state(stateData);
//     if (!state.isValid()) {
//         return false;
//     }
//     QList<GObjectReference> refs = state.getSequenceObjects();
//     assert(!refs.isEmpty());
//     foreach (const GObjectReference& ref, refs) {
//         Document* doc = AppContext::getProject()->findDocumentByURL(ref.docUrl);
//         if (doc == NULL) { //todo: accept to use invalid state removal routines of ObjectViewTask ???
//             return false;
//         }
//         //check that document is in selection
//         QList<Document*> selectedDocs = SelectionUtils::getSelectedDocs(multiSelection);
//         bool docIsSelected = selectedDocs.contains(doc);
// 
//         //check that object is in selection
//         QList<GObject*> selectedObjects = SelectionUtils::getSelectedObjects(multiSelection);
//         GObject* obj = doc->findGObjectByName(ref.objName);
//         bool objIsSelected = obj!=NULL && selectedObjects.contains(obj);
// 
//         //check that object associated with sequence object is in selection
//         bool refIsSelected = false;
//         foreach (const GObject* selObject, selectedObjects) {
//             GObjectReference selRef(selObject);
//             if (ref == selRef) {
//                 refIsSelected = true;
//                 break;
//             }
//         }
//         if (!docIsSelected && !objIsSelected && !refIsSelected) {
//             return false;
//         }
//     }
// 
//     return true;
// }

// Task* ExpertDiscoveryViewFactory::createViewTask(const QString& viewName, const QVariantMap& stateData) {
//     return new OpenSavedAnnotatedDNAViewTask(viewName, stateData);
// }



} //namespace
