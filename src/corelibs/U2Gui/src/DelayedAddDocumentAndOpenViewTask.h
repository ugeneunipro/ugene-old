#ifndef _U2_DELAYED_ADD_AND_OPEN_VIEW_TASK_H_
#define _U2_DELAYED_ADD_AND_OPEN_VIEW_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class Document;

/**
    This task is a simple wrapper for an AddDocumentTask and a LoadUnloadedDocumentAndOpenViewTask
    which waits for a signal providing a document and then proceeds to create the two tasks as its
    subtasks and register itself with the task scheduler.
    It can only be deleted by the task scheduler so it's imperative that the correct signal is received
    at some point by an instance of this class and it's allowed to do its work and no memory leak occurs.
*/
class U2GUI_EXPORT DelayedAddDocumentAndOpenViewTask: public Task {
    Q_OBJECT
public:
    DelayedAddDocumentAndOpenViewTask() : Task(tr("Delayed load and open document task"), TaskFlags_NR_FOSCOE) {}
    ~DelayedAddDocumentAndOpenViewTask() {};

public slots:
    void sl_onDocumentAvailable(Document *d);
};


} // namespace


#endif
