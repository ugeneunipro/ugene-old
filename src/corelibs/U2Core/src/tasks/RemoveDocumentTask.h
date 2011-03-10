#ifndef _U2_REMOVE_DOCUMENT_TASK_H_
#define _U2_REMOVE_DOCUMENT_TASK_H_


#include <U2Core/Task.h>

#include <QtCore/QPointer>

namespace U2 {

class Document;
class Project;
class StateLock;

class U2CORE_EXPORT RemoveMultipleDocumentsTask : public Task {
    Q_OBJECT
public:
    RemoveMultipleDocumentsTask(Project* p, const QList<Document*>& docs, bool saveModifiedDocs, bool useGUI);
    ~RemoveMultipleDocumentsTask();

    virtual void prepare();

    ReportResult report();

private:
    QPointer<Project>           p;
    bool                        saveModifiedDocs;
    bool                        useGUI;
    StateLock*                  lock;
    QList<QPointer<Document> >  docPtrs;
};


}//namespace

#endif
