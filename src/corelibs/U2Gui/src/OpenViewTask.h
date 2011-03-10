#ifndef _U2_OPEN_DOCUMENT_TASK_H_
#define _U2_OPEN_DOCUMENT_TASK_H_

#include <U2Core/Task.h>
#include <U2Core/GUrl.h>

namespace U2 {

class Document;
class LoadDocumentTask;
class LoadRemoteDocumentTask;

class U2GUI_EXPORT LoadUnloadedDocumentAndOpenViewTask : public Task {
    Q_OBJECT
public:
    LoadUnloadedDocumentAndOpenViewTask(Document* d);
    
protected:
    virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
    void clearResourceUse();

    class LoadUnloadedDocumentTask* loadUnloadedTask;
};

class U2GUI_EXPORT LoadRemoteDocumentAndOpenViewTask : public Task {
    Q_OBJECT
public:
    LoadRemoteDocumentAndOpenViewTask(const QString& accId, const QString& dbName);
    LoadRemoteDocumentAndOpenViewTask(const QString& accId, const QString& dbName, const QString & fullpath);
    LoadRemoteDocumentAndOpenViewTask(const GUrl& url);
    virtual void prepare();
protected:
    QList<Task*> onSubTaskFinished(Task* subTask);
private:
    QString accNumber, databaseName;
    QString fullpath;
    GUrl    docUrl;
    LoadRemoteDocumentTask* loadRemoteDocTask;

};

class U2GUI_EXPORT OpenViewTask : public Task {
    Q_OBJECT
public:
    OpenViewTask(Document* d);
protected:
    void prepare();
private:
    Document* doc;

};


}//namespace

#endif
