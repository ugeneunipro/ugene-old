
#include <U2Core/IOAdapter.h>
#include <U2Core/DocumentModel.h>
#include <U2Core/L10n.h>

#include "SaveDocumentStreamingTask.h"

namespace U2 {

SaveDocumentStreamingTask::SaveDocumentStreamingTask( Document* d, IOAdapter* i )
: Task(tr("Save document"), TaskFlags(TaskFlag_None)), lock(NULL), doc(d), io(i) {
    if( NULL == doc ) {
        stateInfo.setError(L10N::badArgument("doc"));
        return;
    }
    if( NULL == io || !io->isOpen() ) {
        stateInfo.setError(L10N::badArgument("IO adapter"));
        return;
    }
    lock = new StateLock( getTaskName() );
    tpm = Progress_Manual;
}

SaveDocumentStreamingTask::~SaveDocumentStreamingTask() {
    assert( NULL == lock );
}

void SaveDocumentStreamingTask::prepare() {
    if( stateInfo.hasErrors() ) {
        return;
    }
    doc->lockState( lock );
}

void SaveDocumentStreamingTask::run() {
    if( stateInfo.hasErrors() ) {
        return;
    }
    DocumentFormat* df = doc->getDocumentFormat();
    df->storeDocument( doc, stateInfo, io );
}

Task::ReportResult SaveDocumentStreamingTask::report() {
    if( NULL != doc ) {
        doc->makeClean();
        doc->unlockState( lock );
    }
    if( NULL != lock ) {
        delete lock;
        lock = NULL;
    }
    return ReportResult_Finished;
}

Document* SaveDocumentStreamingTask::getDocument() const {
    return doc;
}

} // U2
