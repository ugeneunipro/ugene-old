
#ifndef _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_
#define _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/UIndex.h>
#include <U2Core/DocumentModel.h>

namespace U2 {

struct GZipIndexAccessPoint;

class U2CORE_EXPORT GetDocumentFromIndexTask : public Task {
    Q_OBJECT
public:
    GetDocumentFromIndexTask( const UIndex& index, int docNum );
    ~GetDocumentFromIndexTask();
    
    virtual void run();
    virtual ReportResult report();
    virtual void cleanup();
    
    Document* getDocument() const;
    Document* takeDocument();

private:
    bool fillAccessPointNums( GZipIndexAccessPoint& point, const QString& numStr );
    bool getGzipIndexAccessPoint( GZipIndexAccessPoint& ret, const UIndex::IOSection& ioSec, qint64 offset );
    IOAdapter* getOpenedIOAdapter(const UIndex::ItemSection& itemSec, const UIndex::IOSection& ioSec);
private:
    UIndex    index;
    int       docNum;
    Document* doc;
    
}; // GetDocumentFromIndexTask

} // U2

#endif // _U2_GET_DOCUMENT_FROM_INDEX_TASK_H_
