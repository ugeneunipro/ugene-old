#include "DelayedAddDocumentAndOpenViewTask.h"

#include <U2Gui/OpenViewTask.h>
#include <U2Core/AddDocumentTask.h>
#include <U2Core/MultiTask.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectModel.h>

namespace U2 {

    void DelayedAddDocumentAndOpenViewTask::sl_onDocumentAvailable(Document *d) {
        assert(d != NULL);
        QList<Task*> taskList;

        // open a new project first if required
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            QList<GUrl> emptyList;
            taskList.prepend(  AppContext::getProjectLoader()->openProjectTask(emptyList, false) );
        }
        taskList.append(new LoadUnloadedDocumentAndOpenViewTask(d));
        taskList.append(new AddDocumentTask(d));
        addSubTask(new MultiTask(tr("Load and open the document"), taskList));
        AppContext::getTaskScheduler()->registerTopLevelTask(this);
    }
} // namespace