#ifndef _U2_ADD_DOCUMENT_TASK_H_
#define _U2_ADD_DOCUMENT_TASK_H_

#include <qpointer.h>
#include <U2Core/Task.h>

namespace U2 {
class Document;

// Adds document to the project. Waits for locks if any
class U2CORE_EXPORT AddDocumentTask : public Task {
    Q_OBJECT
public:
    AddDocumentTask( Document * d );
    virtual ReportResult report();
private:
    QPointer<Document> d;
};


}//namespace

#endif
