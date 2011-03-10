#include "AddSequencesToAlignmentTask.h"

#include <U2Core/DocumentModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Core/LoadDocumentTask.h>
#include <U2Core/DocumentUtils.h>
#include <U2Core/IOAdapter.h>

namespace U2 {


AddSequencesToAlignmentTask::AddSequencesToAlignmentTask( MAlignmentObject* obj, const QString& fileWithSequencesUrl )
: Task("Add sequences to alignment task", TaskFlag_NoRun), maObj(obj)
{
    assert(!fileWithSequencesUrl.isEmpty());
    QList<DocumentFormat*> detectedFormats = DocumentUtils::detectFormat(fileWithSequencesUrl);    
    if (!detectedFormats.isEmpty()) {
        IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById(BaseIOAdapters::LOCAL_FILE);
        DocumentFormat* format = detectedFormats.first();
        loadTask = new LoadDocumentTask(format->getFormatId(), fileWithSequencesUrl, factory);
        addSubTask(loadTask);
    } else {
        setError("Unknown format");
    }
}

QList<Task*> AddSequencesToAlignmentTask::onSubTaskFinished( Task* subTask )
{
    QList<Task*> subTasks;

    propagateSubtaskError();
    if ( (subTask != loadTask )|| (isCanceled()) || (hasErrors()) ) {
        return subTasks;
    }

    Document* doc = loadTask->getDocument();
    QList<GObject*> seqObjects = doc->findGObjectByType(GObjectTypes::SEQUENCE);
    
    foreach(GObject* obj, seqObjects) {
        DNASequenceObject* dnaObj = qobject_cast<DNASequenceObject*>(obj);
        assert(dnaObj != NULL);
        DNAAlphabet* dnaAl = dnaObj->getAlphabet();
        if (maObj->getAlphabet()->getType() == dnaAl->getType()) {
            maObj->addRow(dnaObj->getDNASequence());
        } else {
            stateInfo.setError(tr("Sequence %1 from %2 has different alphabet").arg(dnaObj->getGObjectName()).arg(loadTask->getDocument()->getURLString()));
        }
    }
    
    return subTasks;

}

Task::ReportResult AddSequencesToAlignmentTask::report()
{
    return ReportResult_Finished;
}


}