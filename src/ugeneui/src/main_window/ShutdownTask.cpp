#include "ShutdownTask.h"
#include "MainWindowImpl.h"

#include <U2Core/Log.h>
#include <U2Core/ServiceModel.h>
#include <U2Core/PluginModel.h>
#include <U2Core/AppContext.h>
#include <U2Core/ProjectService.h>
#include <U2Core/Settings.h>
#include <project_support/ProjectLoaderImpl.h>

#include <QtGui/QMessageBox>

namespace U2 {

ShutdownTask::ShutdownTask(MainWindowImpl* _mw) : Task(tr("shutdown_task_name"), TaskFlags(TaskFlag_NoRun)), mw(_mw) 
{
}

static bool isReadyToBeDisabled(Service* s, ServiceRegistry* sr) {
	ServiceType st = s->getType();
	int nServicesOfTheSameType = sr->findServices(st).size();
	assert(nServicesOfTheSameType >= 1);
	foreach(Service* child, sr->getServices()) {
		if (!child->getParentServiceTypes().contains(st) || !child->isEnabled()) {
			continue;
		}
		if (nServicesOfTheSameType == 1) {
			return false;
		}
	}
	return true;
}

static Service* findServiceToDisable(ServiceRegistry* sr) {
	int nEnabled = 0;
	foreach(Service* s, sr->getServices()) {
		nEnabled+= s->isEnabled() ? 1 : 0;
		if (s->isEnabled() && isReadyToBeDisabled(s, sr)) {
			return s;
		}
	}
	assert(nEnabled == 0);
	return NULL;
}

static bool closeViews() {
    MWMDIManager* wm = AppContext::getMainWindow()->getMDIManager();
    MWMDIWindow* w = NULL;
    // close windows one by one, asking active window first
    // straightforward foreach() cycle appears not flexible enough, 
    // as interdependent windows may close each other (happened with TestRunner and TestReporter)
    while ((w = wm->getActiveWindow())) {
        if (!wm->closeMDIWindow(w)) {
            return false;
        }
    }
    while (!wm->getWindows().isEmpty()) {
        if (!wm->closeMDIWindow(wm->getWindows().first())) {
            return false;
        }
    }
    return true;
}

class CloseWindowsTask : public Task {
public:
    CloseWindowsTask() : Task(U2::ShutdownTask::tr("Close windows"), TaskFlags(TaskFlag_NoRun)) {}
    void prepare() {
        Project* proj = AppContext::getProject();
        if (proj == NULL) {
            return;
        }
        if ( proj->isTreeItemModified() ) {
            addSubTask(AppContext::getProjectService()->saveProjectTask(SaveProjectTaskKind_SaveProjectAndDocumentsAskEach));
        }
    }

    QList<Task*> onSubTaskFinished(Task* subTask) {
        if (subTask->isCanceled()) {
            setError("Shutdown is canceled by user");
            return QList<Task*>();
        }
        coreLog.trace("Closing views");
        if (!closeViews()) {
            getTopLevelParentTask()->cancel();
        }
        return QList<Task*>();
    }

    ReportResult report() {
        // wait for saving/closing tasks if any
        foreach(Task* t, AppContext::getTaskScheduler()->getTopLevelTasks()) {
            if (t != getTopLevelParentTask() && !t->isFinished()) {
                return ReportResult_CallMeAgain;
            }
        }
        return ReportResult_Finished;
    }
};


class CancelAllTask : public Task {
public:
    CancelAllTask() : Task(U2::ShutdownTask::tr("Cancel active tasks"), TaskFlag_NoRun) {}
    void prepare() {
        // cancel all tasks but ShutdownTask
        QList<Task*> activeTopTasks = AppContext::getTaskScheduler()->getTopLevelTasks();
        activeTopTasks.removeOne(getTopLevelParentTask());
        foreach(Task* t, activeTopTasks) {
            coreLog.trace(QString("Canceling: %1").arg(t->getTaskName()));
            t->cancel();
        }
    }

    ReportResult report() {
        foreach(Task* t, AppContext::getTaskScheduler()->getTopLevelTasks()) {
            if (t->isCanceled() && !t->isFinished()) {
                return ReportResult_CallMeAgain;
            }
        }
        return ReportResult_Finished;
    }
};

// This function prepends empty string to RecentProjects in UGENE SETTINGS in order 
// to prevent project auto loading on next UGENE launch
static void cancelProjectAutoLoad() {
    QStringList recentFiles = AppContext::getSettings()->getValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME).toStringList();
    QString emptyUrl;
    recentFiles.prepend(emptyUrl);
    AppContext::getSettings()->setValue(SETTINGS_DIR + RECENT_PROJECTS_SETTINGS_NAME, recentFiles);
}


void ShutdownTask::prepare() {
    coreLog.info(tr("Starting shutdown process..."));

    Project* currProject = AppContext::getProject();
    if (currProject == NULL) {
        cancelProjectAutoLoad();
    }
    
    Task* ct = new CloseWindowsTask();
    addSubTask(ct);
    
    QList<Task*> activeTopTasks = AppContext::getTaskScheduler()->getTopLevelTasks();
    activeTopTasks.removeOne(this);
    if (!activeTopTasks.isEmpty()) {
        QStringList sl;
        foreach(Task* t, activeTopTasks) {
            sl.append(t->getTaskName());
        }

        QMessageBox::StandardButton ret = QMessageBox::question(mw->getQMainWindow(), 
            tr("Shutdown confirmation"), 
            tr("There are active tasks. Stop them now?")+QString("\n\n - %1").arg(sl.join("\n - ")), 
            QMessageBox::Ok|QMessageBox::Cancel);
        if (ret != QMessageBox::Ok) {
            cancel();
            return;
        }
        // To be run before closing any views
        // otherwise it may cancel tasks produced by closing windows (e.g. SaveWorkflowTask)
        ct->addSubTask(new CancelAllTask());
    }
}

QList<Task*> ShutdownTask::onSubTaskFinished(Task* subTask) {
    QList<Task*> res;
    
    if (isCanceled() || subTask->hasErrors()) {
        mw->setShutDownInProcess(false);
        return res; //stop shutdown process
    }

	ServiceRegistry* sr = AppContext::getServiceRegistry();
	Service* s = findServiceToDisable(sr);
	if (s!=NULL) {
		res.append(sr->disableServiceTask(s));
	}
	return res;
}

Task::ReportResult ShutdownTask::report() {
    if (propagateSubtaskError() || hasErrors() || isCanceled()) {
        setErrorNotificationSuppression(true);
        return Task::ReportResult_Finished;
    }

#ifdef _DEBUG
	const QList<Service*>& services = AppContext::getServiceRegistry()->getServices();
	foreach(Service* s, services) {
		assert(s->isDisabled());
	}
#endif
    mw->close();
    QCoreApplication::exit(0);
	return Task::ReportResult_Finished;
}



}//namespace
