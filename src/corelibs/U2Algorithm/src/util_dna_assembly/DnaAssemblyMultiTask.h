#ifndef _U2_DNA_ASSEMBLY_MULTI_TASK_
#define _U2_DNA_ASSEMBLY_MULTI_TASK_

#include <U2Algorithm/DnaAssemblyTask.h>
#include <U2Core/Task.h>
#include <U2Core/GUrl.h>

namespace U2 {

class Document;
class LoadDocumentTask;
class AddDocumentTask;
class MAlignmentObject;

class U2ALGORITHM_EXPORT DnaAssemblyMultiTask : public Task {
    Q_OBJECT
public:
    DnaAssemblyMultiTask(const DnaAssemblyToRefTaskSettings& settings, bool viewResult = false, bool justBuildIndex = false);
    virtual void prepare();
    virtual ReportResult report();
    virtual QString generateReport() const;
    QList<Task*> onSubTaskFinished(Task* subTask);
    const MAlignmentObject* getAssemblyResult();

signals:
    void documentAvailable(Document*);

private:
    DnaAssemblyToRefTaskSettings settings;
    DnaAssemblyToReferenceTask* assemblyToRefTask;
    AddDocumentTask* addDocumentTask;
    LoadDocumentTask* loadDocumentTask;
    Document* doc;
    QList<GUrl> shortReadUrls;
    bool openView;
    bool justBuildIndex;
}; 

} // namespace
#endif
