#ifndef _U2_PROJECT_SERVICE_IMPL_H_
#define _U2_PROJECT_SERVICE_IMPL_H_

#include <U2Core/ProjectService.h>
#include <U2Core/Task.h>

#include <QtGui/QAction>

namespace U2 {

class ProjectServiceImpl : public ProjectService {
    Q_OBJECT

    friend class ProjectServiceEnableTask;
	friend class ProjectServiceDisableTask;
public:
	ProjectServiceImpl(Project* pr); 
	~ProjectServiceImpl();

	virtual Project* getProject() const {return pr;}

	virtual Task* saveProjectTask(SaveProjectTaskKind k);

	virtual Task* closeProjectTask();

    virtual void enableSaveAction(bool e);

protected:
	virtual Task* createServiceEnablingTask();
	virtual Task* createServiceDisablingTask();

private slots:
	void sl_save();
    void sl_saveAs();
	void sl_closeProject();
	void sl_exportProject();
	
private:
	QAction* saveAction;
    QAction* saveAsAction;
	QAction* closeProjectAction;
	QAction* exportProjectAction;

	Project* pr;
};


//////////////////////////////////////////////////////////////////////////
/// Tasks
class ProjectServiceEnableTask : public Task {
    Q_OBJECT
public:
	ProjectServiceEnableTask(ProjectServiceImpl* psi);
	virtual ReportResult report();
private:
	ProjectServiceImpl* psi;
};


class ProjectServiceDisableTask : public Task {
    Q_OBJECT
public:
	ProjectServiceDisableTask(ProjectServiceImpl* psi);
	virtual ReportResult report();
private:
	ProjectServiceImpl* psi;
};

}//namespace

#endif

