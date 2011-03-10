
#ifndef _U2_SAVE_DOCUMENT_STREAMING_TASK_H_
#define _U2_SAVE_DOCUMENT_STREAMING_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class Document;
class StateLock;
class IOAdapter;

class U2CORE_EXPORT SaveDocumentStreamingTask : public Task {
    Q_OBJECT
public:
    // io - opened io adapter
    SaveDocumentStreamingTask( Document* doc, IOAdapter* io );
    ~SaveDocumentStreamingTask();
    
    virtual void prepare();
    
    virtual void run();
    
    ReportResult report();
    
    Document* getDocument() const;
    
private:
    StateLock*  lock;
    Document*   doc;
    IOAdapter*  io;
    
}; // SaveDocumentStreamingTask

} // U2

#endif // _U2_SAVE_DOCUMENT_STREAMING_TASK_H_
