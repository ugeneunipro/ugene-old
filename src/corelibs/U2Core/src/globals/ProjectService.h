#ifndef _U2_PROJECT_SERVICE_H_
#define _U2_PROJECT_SERVICE_H_

#include <U2Core/ProjectModel.h>
#include <U2Core/ServiceModel.h>
#include <U2Core/ServiceTypes.h>
#include <U2Core/Task.h>

#include <QtGui/QAction>

namespace U2 {

    enum SaveProjectTaskKind {
        SaveProjectTaskKind_SaveProjectOnly,
        SaveProjectTaskKind_SaveProjectAndDocuments,
        SaveProjectTaskKind_SaveProjectAndDocumentsAskEach
    };



    class U2CORE_EXPORT ProjectService : public Service {
    public:
        ProjectService(const QString& sname, const QString& sdesc) 
            : Service(Service_Project, sname, sdesc, QList<ServiceType>(), ServiceFlag_Singleton) {}

        virtual Project* getProject() const = 0;

        virtual Task* saveProjectTask(SaveProjectTaskKind kind) = 0;

        virtual Task* closeProjectTask() = 0;

        // hack to work around ambigous shortcuts
        virtual void enableSaveAction(bool e) = 0;
    };

}//namespace

#endif

