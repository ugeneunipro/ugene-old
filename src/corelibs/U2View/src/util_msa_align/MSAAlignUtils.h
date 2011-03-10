#ifndef _U2_MSA_ALIGN_UTILS_H_
#define _U2_MSA_ALIGN_UTILS_H_

#include <U2Core/global.h>
#include <U2Core/GUrl.h>
#include <U2Core/Task.h>

#include <U2Algorithm/MSAAlignTask.h>

namespace U2 {

class MSAAlignTaskSettings;
class Document;
class LoadDocumentTask;
class SaveDocumentTask;
class AddDocumentTask;
class MAlignmentObject;

class U2VIEW_EXPORT MSAAlignFileTask : public Task {
    Q_OBJECT
public:
    MSAAlignFileTask(const MSAAlignTaskSettings& settings, bool viewResult = false);
    virtual void prepare();
    virtual ReportResult report();
    QList<Task*> onSubTaskFinished(Task* subTask);
    const MAlignmentObject* getAlignResult();

private:
    MSAAlignTaskSettings settings;
    MSAAlignTask* alignTask;
    AddDocumentTask* addDocumentTask;
    LoadDocumentTask* loadDocumentTask;
    SaveDocumentTask* saveDocumentTask;
    MAlignmentObject* obj;
    Document* doc;
    bool openView;
}; 

} // U2

#endif // _U2_MSA_ALIGN_TASK_H_
