#include "RemoveDocumentTask.h"

#include "SaveDocumentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>


namespace U2 {

RemoveMultipleDocumentsTask::RemoveMultipleDocumentsTask(Project* _p, const QList<Document*>& _docs, bool _saveModifiedDocs, bool _useGUI) 
: Task(tr("Remove document"), TaskFlag_NoRun), p(_p), saveModifiedDocs(_saveModifiedDocs), useGUI(_useGUI)
{
    assert(!_docs.empty());
    assert(p!=NULL);

    foreach(Document* d, _docs) {
        docPtrs.append(d);
    }
    lock = new StateLock(getTaskName());
}

RemoveMultipleDocumentsTask::~RemoveMultipleDocumentsTask() {
    assert(lock == NULL);
}

void RemoveMultipleDocumentsTask::prepare() {
    p->lockState(lock);
    if (p->isTreeItemModified() && saveModifiedDocs) {
        QList<Document*> docs;
        foreach(Document* d, docPtrs) {
            if (d!=NULL) {
                docs.append(d);
            }
        }
        QList<Document*> modifiedDocs = SaveMiltipleDocuments::findModifiedDocuments(docs);
        if (!modifiedDocs.isEmpty()) {
            addSubTask(new SaveMiltipleDocuments(modifiedDocs, useGUI));
        }
    }
}


Task::ReportResult RemoveMultipleDocumentsTask::report() {
    if (lock!=NULL) {
        assert(!p.isNull());
        p->unlockState(lock);
        delete lock;
        lock = NULL;
            
        Task* t = getSubtaskWithErrors();
        if (t!=NULL) {
            stateInfo.setError(t->getError());
            return Task::ReportResult_Finished;
        }
    }

    if (p.isNull()) {
        return Task::ReportResult_Finished;
    }

    if (p->isStateLocked()) {
        return Task::ReportResult_CallMeAgain;
    }

    foreach(Document* doc, docPtrs) {
        if (doc!=NULL) {
            p->removeDocument(doc);
        }
    }

    return Task::ReportResult_Finished;
}


}//namespace
