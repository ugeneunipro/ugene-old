#include "ObjectViewTasks.h"

#include <U2Core/LoadDocumentTask.h>
#include <U2Core/Log.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/GObject.h>

namespace U2 {

/* TRANSLATOR U2::ObjectViewTask */

ObjectViewTask::ObjectViewTask(GObjectView* _view, const QString& stateName, const QVariantMap& s) 
: Task("", TaskFlag_NoRun), taskType(Type_Update), stateData(s), view(_view), stateIsIllegal(false)
{
    assert(view!=NULL);
    const QString& vName = view->getName();
    setTaskName(tr("Update '%1' to '%2' state").arg(vName).arg(stateName));
    setVerboseLogMode(true);
}

ObjectViewTask::ObjectViewTask(GObjectViewFactoryId fid, const QString& vName, const QVariantMap& s) 
: Task("", TaskFlag_NoRun), taskType(Type_Open), stateData(s), view(NULL), viewName(vName), stateIsIllegal(false)
{
    if (vName.isEmpty()) {
        QString factoryName = AppContext::getObjectViewFactoryRegistry()->getFactoryById(fid)->getName();
        setTaskName(tr("Open new '%1'").arg(factoryName));
    } else {
        setTaskName(tr("Open '%1'").arg(vName));
    }
    
    setVerboseLogMode(true);
}


void ObjectViewTask::prepare() {
    QSet<Document*> processed;
    foreach(QPointer<Document> pd, documentsToLoad) {
        if (!pd.isNull() && !processed.contains(pd)){
            addSubTask(new LoadUnloadedDocumentTask(pd));
            processed.insert(pd);
        }
    }
}

Task::ReportResult ObjectViewTask::report() {
    foreach(QPointer<Document> pd, documentsToLoad) {
        if (pd.isNull()){
            continue;
        }
        if (!pd->isLoaded()) {
            documentsFailedToLoad.append(pd);
            continue;
        }
        onDocumentLoaded(pd);
    }

    if (taskType == Type_Open) {
        open();
    } else {
        assert(taskType == Type_Update);
        update();
    }

    if (stateIsIllegal) {
        //todo: ask to remove illegal state
    }

    return ReportResult_Finished;
}

//////////////////////////////////////////////////////////////////////////
// AddToViewTask

AddToViewTask::AddToViewTask(GObjectView* v, GObject* obj) 
: Task(tr("Add object to view %1").arg(obj->getGObjectName()), TaskFlags_NR_FOSCOE), 
objView(v), viewName(v->getName()), objRef(obj), objDoc(obj->getDocument())
{
    assert(objDoc!=NULL);
    if (obj->isUnloaded()) {
        addSubTask(new LoadUnloadedDocumentTask(objDoc));
    }
    
}

Task::ReportResult AddToViewTask::report() {
    if (hasErrors() || isCanceled()) {
        return ReportResult_Finished;
    }
    if (objDoc == NULL) {
        stateInfo.setError(tr("Document was removed %1").arg(objRef.docUrl));
        return ReportResult_Finished;
    }
    GObject* obj = objDoc->findGObjectByName(objRef.objName);
    if (obj == NULL) {
        stateInfo.setError(tr("Object not found %1").arg(objRef.objName));
        return ReportResult_Finished;
    }
    if (objView == NULL) {
        stateInfo.setError(tr("View was closed %1").arg(viewName));
        return ReportResult_Finished;
    }
    objView->addObject(obj);
    return ReportResult_Finished;
}

} // namespace
