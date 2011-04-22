#include "AssemblyBrowserFactory.h"

#include <U2Core/AppContext.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AssemblyObject.h>
#include <U2Core/SelectionUtils.h>
#include <U2Core/U2SafePoints.h>

#include "AssemblyBrowser.h"

namespace U2 {

//==============================================================================
// AssemblyBrowserFactory
//==============================================================================

const GObjectViewFactoryId AssemblyBrowserFactory::ID = "assembly-browser-factory";

AssemblyBrowserFactory::AssemblyBrowserFactory(QObject * parent /* = 0 */) :
GObjectViewFactory(ID, tr("Assembly Browser"), parent) 
{
}

bool AssemblyBrowserFactory::canCreateView(const MultiGSelection & multiSelection) {
    bool hasAssembly = !SelectionUtils::findDocumentsWithObjects(GObjectTypes::ASSEMBLY, &multiSelection, UOF_LoadedAndUnloaded, true).isEmpty();
    return hasAssembly;
}

#define MAX_VIEWS 5

Task * AssemblyBrowserFactory::createViewTask(const MultiGSelection & multiSelection, bool single /* = false */) {
    QList<GObject*> asmObjects = SelectionUtils::findObjects(GObjectTypes::ASSEMBLY, &multiSelection, UOF_LoadedAndUnloaded);
    QSet<Document*> docsWithAsm = SelectionUtils::findDocumentsWithObjects(GObjectTypes::ASSEMBLY, 
        &multiSelection, UOF_LoadedAndUnloaded, false);
    QList<OpenAssemblyBrowserTask*> resTasks;

    foreach(Document* doc, docsWithAsm) {
        QList<GObject*> docObjs = doc->findGObjectByType(GObjectTypes::ASSEMBLY, UOF_LoadedAndUnloaded);
        if (!docObjs.isEmpty()) {
            asmObjects+=docObjs;
        } else {
            resTasks.append(new OpenAssemblyBrowserTask(doc));
            if (resTasks.size() == MAX_VIEWS) {
                break;
            }
        }
    }

    if (!asmObjects.isEmpty()) {
        foreach(GObject* o, asmObjects) {
            if (resTasks.size() == MAX_VIEWS) {
                break;
            }
            if (o->getGObjectType() == GObjectTypes::UNLOADED) {
                resTasks.append(new OpenAssemblyBrowserTask(qobject_cast<UnloadedObject*>(o)));
            } else {
                SAFE_POINT(o->getGObjectType() == GObjectTypes::ASSEMBLY, "Invalid assembly object!", NULL);
                resTasks.append(new OpenAssemblyBrowserTask(qobject_cast<AssemblyObject*>(o)));
            }
        }
    }

    if (resTasks.isEmpty()) {
        return NULL;
    }

    if (resTasks.size() == 1 || single) {
        return resTasks.first();
    }

    Task* result = new Task(tr("Open multiple views"), TaskFlag_NoRun);
    foreach(Task* t, resTasks) {
        result->addSubTask(t);
    }
    return result;
}

//==============================================================================
// OpenAssemblyBrowserTask
//==============================================================================

OpenAssemblyBrowserTask::OpenAssemblyBrowserTask(AssemblyObject * obj) : ObjectViewTask(AssemblyBrowserFactory::ID) {
    selectedObjects.append(obj);
}

OpenAssemblyBrowserTask::OpenAssemblyBrowserTask(UnloadedObject * unloadedObj) : ObjectViewTask(AssemblyBrowserFactory::ID),
    unloadedObjRef(unloadedObj) {
        documentsToLoad.append(unloadedObj->getDocument());
}

OpenAssemblyBrowserTask::OpenAssemblyBrowserTask(Document * doc) : ObjectViewTask(AssemblyBrowserFactory::ID) {
    assert(!doc->isLoaded());
    documentsToLoad.append(doc);
}

void OpenAssemblyBrowserTask::open() {
    if (stateInfo.hasErrors() || (documentsToLoad.isEmpty() && selectedObjects.isEmpty())) {
        return;
    }
    
    if (selectedObjects.isEmpty()) {
        assert(1 == documentsToLoad.size());
        Document* doc = documentsToLoad.first();
        QList<GObject*> objects;
        if (unloadedObjRef.isValid()) {
            GObject* obj = doc->findGObjectByName(unloadedObjRef.objName);
            if (obj!=NULL && obj->getGObjectType() == GObjectTypes::ASSEMBLY) {
                selectedObjects.append(qobject_cast<AssemblyObject*>(obj));
            }
        } else {
            QList<GObject*> objects = doc->findGObjectByType(GObjectTypes::ASSEMBLY, UOF_LoadedAndUnloaded);
            if(!objects.isEmpty()) {
                selectedObjects.append(qobject_cast<AssemblyObject*>(objects.first()));
            } 
        }
        if (selectedObjects.isEmpty()) {
            stateInfo.setError(tr("Assembly object not found"));
            return;
        }
    }
    
    foreach(QPointer<GObject> po, selectedObjects) {
        AssemblyObject* o = qobject_cast<AssemblyObject*>(po);
        
        SAFE_POINT(o, "Invalid assembly object!", );

        viewName = GObjectViewUtils::genUniqueViewName(o->getDocument(), o);
        AssemblyBrowser * v = new AssemblyBrowser(o);
        GObjectViewWindow* w = new GObjectViewWindow(v, viewName, false);
        AppContext::getMainWindow()->getMDIManager()->addMDIWindow(w);
    }
}

} //ns
