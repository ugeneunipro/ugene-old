#ifndef _U2_SHUTDOWN_TASK_H_
#define _U2_SHUTDOWN_TASK_H_

#include <U2Core/Task.h>

namespace U2 {

class MainWindowImpl;

class ShutdownTask : public Task {
    Q_OBJECT
public:
	ShutdownTask(MainWindowImpl* mw);
	
	void prepare();

	ReportResult report();

protected:
	virtual QList<Task*> onSubTaskFinished(Task* subTask);

private:
	MainWindowImpl* mw;

};


}//namespace

#endif
