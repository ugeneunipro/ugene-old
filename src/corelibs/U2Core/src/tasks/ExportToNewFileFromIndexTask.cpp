
#include <U2Core/AppContext.h>
#include <U2Core/IOAdapter.h>

#include <U2Core/GetDocumentFromIndexTask.h>
#include <U2Core/SaveDocumentStreamingTask.h>

#include "ExportToNewFileFromIndexTask.h"

namespace U2 {


ExportToNewFileFromIndexTask::ExportToNewFileFromIndexTask( const UIndex& i, const QList< int >& n, const QString& file )
: Task( tr( "Export from index" ), TaskFlag_NoRun ),
  index( i ), docNums( n ), exportFilename( file ), io( NULL ), saveCurDocTask( NULL ) 
{
    setVerboseLogMode(true);
    if( exportFilename.isEmpty() ) {
        stateInfo.setError(tr( "Export filename is empty" ));
        return;
    }
    if( UIndex::NO_ERR != index.checkConsistency() ) {
        stateInfo.setError(tr( "Index is inconsistent" ));
        return;
    }
    if( docNums.isEmpty() ) {
        stateInfo.setError(tr( "Document numbers list is empty" ));
        return;
    }
    tpm = Progress_SubTasksBased;
}

ExportToNewFileFromIndexTask::~ExportToNewFileFromIndexTask() {
    assert( docsToSave.isEmpty() );
    assert( NULL == io );
}


void ExportToNewFileFromIndexTask::prepare() {
    io = getOpenedIOAdapter(exportFilename);
    if( NULL == io ) {
        assert( stateInfo.hasErrors() );
        return;
    }
    
    QList< Task* > subs;
    int itemsSz = index.items.size();
    foreach(int docInd, docNums) {
        if( !( 0 <= docInd && itemsSz > docInd ) ) {
            setError(tr( "Invalid document number: %1, max: %2" ).arg(docInd).arg(itemsSz));
            break;
        }
        subs << new GetDocumentFromIndexTask( index, docInd );
    }
    if (hasErrors() || isCanceled()) {
        qDeleteAll( subs );
        return;
    }
    foreach( Task* task, subs ) {
        addSubTask( task );
    }
}

Task::ReportResult ExportToNewFileFromIndexTask::report() {
    if( NULL != io ) {
        delete io;
        io = NULL;
    }
    return ReportResult_Finished;
}

IOAdapter* ExportToNewFileFromIndexTask::getOpenedIOAdapter(const QString& url) {
    IOAdapterId ioId = BaseIOAdapters::url2io( url );
    IOAdapterFactory* factory = AppContext::getIOAdapterRegistry()->getIOAdapterFactoryById( ioId );
    if( NULL == factory ) {
        setError(tr( "No IO adapter found for URL: %1").arg(url));
        return NULL;
    }
    IOAdapter* res = factory->createIOAdapter();
    if( !res->open( url, IOAdapterMode_Write ) ) {
        setError(tr("Can't open file for write: '%1'").arg(url));
        delete res;
        return NULL;
    }
    return res;
}

QList< Task* > ExportToNewFileFromIndexTask::getDocTaskFinished( GetDocumentFromIndexTask* t ) {
    assert( NULL != t );
    QList< Task* > ret;
    Document* doc = t->takeDocument();
    if( NULL == doc ) {
        stateInfo.setError(t->getError());
        return ret;
    }
    docsToSave << doc;
    if( NULL == saveCurDocTask ) {
        saveCurDocTask = new SaveDocumentStreamingTask( docsToSave.takeFirst(), io );
        ret << saveCurDocTask;
    }
    return ret;
}

QList< Task* > ExportToNewFileFromIndexTask::saveDocTaskFinished( SaveDocumentStreamingTask* t ) {
    assert( NULL != t );
    QList< Task* > ret;
    Document* saved = t->getDocument();
    assert( NULL != saved );
    delete saved;
    if( docsToSave.isEmpty() ) {
        saveCurDocTask = NULL;
    } else {
        saveCurDocTask = new SaveDocumentStreamingTask( docsToSave.takeFirst(), io );
        ret << saveCurDocTask;
    }
    return ret;
}

QList< Task* > ExportToNewFileFromIndexTask::onSubTaskFinished( Task* subTask ) {
    assert( NULL != subTask );
    if (isCanceled() || hasErrors()) {
        return QList< Task* >();
    }
    GetDocumentFromIndexTask* getDocTask = qobject_cast< GetDocumentFromIndexTask* >( subTask );
    if( NULL != getDocTask ) {
        return getDocTaskFinished( getDocTask );
    }
    SaveDocumentStreamingTask* saveDocTask = qobject_cast< SaveDocumentStreamingTask* >( subTask );
    if( NULL != saveDocTask ) {
        return saveDocTaskFinished( saveDocTask );
    }
    assert( false );
    return QList< Task* >();
}

} // U2
