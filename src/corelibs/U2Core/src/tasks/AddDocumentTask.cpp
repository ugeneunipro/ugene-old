#include <U2Core/DocumentModel.h>
#include <U2Core/ProjectModel.h>
#include <U2Core/AppContext.h>
#include "AddDocumentTask.h"

namespace U2
{

AddDocumentTask::AddDocumentTask( Document * _d ) :
Task( tr("Add document to the project: %1").arg(_d->getURLString()), TaskFlag_NoRun), d(_d)
{

}

Task::ReportResult AddDocumentTask::report() {
    Project * p = AppContext::getProject();
    if( p == NULL ) { // no project is opened
        setError(tr("No project is opened"));
        return ReportResult_Finished;
    }
    
    if( p->isStateLocked() ) {
        return ReportResult_CallMeAgain;
    } else if (d!= NULL) {
        Document* sameURLDoc = p->findDocumentByURL(d->getURL());
        if (sameURLDoc!=NULL) {
            stateInfo.setError(tr("Document is already added to the project %1").arg(d->getURL().getURLString()));
        } else {
            p->addDocument( d );
        }
    } else {
        stateInfo.setError(stateInfo.getError() + tr("Document was removed"));
    }
    return ReportResult_Finished;
}

}//namespace
