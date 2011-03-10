
#ifndef _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_
#define _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_

#include <U2Core/DocumentModel.h>

#include <U2Core/Task.h>
#include <U2Core/UIndex.h>

namespace U2 {

class IOAdapter;
class GetDocumentFromIndexTask;
class SaveDocumentStreamingTask;

/*
 * Exported documents may be written to filename not in docNums list order
 */
class U2CORE_EXPORT ExportToNewFileFromIndexTask : public Task {
    Q_OBJECT
public:
    ExportToNewFileFromIndexTask( const UIndex& index, const QList< int >& docNums, const QString& filename );
    ~ExportToNewFileFromIndexTask();
    
    virtual void prepare();
    virtual ReportResult report();
    
private:
    QList< Task* > getDocTaskFinished( GetDocumentFromIndexTask* t );
    QList< Task* > saveDocTaskFinished( SaveDocumentStreamingTask* t );
    
protected:
    virtual QList< Task* > onSubTaskFinished( Task* subTask );
    IOAdapter* getOpenedIOAdapter(const QString& url);
    
private:
    UIndex             index;
    QList< int >       docNums;
    QString            exportFilename;
    QList< Document* > docsToSave;
    
    IOAdapter*                 io;
    SaveDocumentStreamingTask* saveCurDocTask;
    
}; // ExportToNewFileFromIndexTask

} // U2

#endif // _U2_EXPORT_TO_NEW_FILE_FROM_INDEX_TASK_H_
