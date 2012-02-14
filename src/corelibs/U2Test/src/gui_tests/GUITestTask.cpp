#include "GUITestTask.h"
#include "GUITestBase.h"
#include <U2Core/AppContext.h>

#define TIMEOUT 60000
#define GUITESTING_REPORT_PREFIX "GUITesting"

namespace U2 {

const QString GUITestTask::taskName = "GUI Test";
const QString GUITestTask::successResult = "Success";

GUITestTask::GUITestTask(GUITest* task) : Task(taskName, TaskFlags_FOSCOE) {

	Q_ASSERT(task);
	if (task) {
		tests.append(task);
	}
}

GUITestTask::~GUITestTask() {

	qDeleteAll(tests);
}

void GUITestTask::prepare() {

	Q_ASSERT(tests.size());
    if (!tests.size()) {
		setError("Empty test");
    }

	addChecks();
}

void GUITestTask::addChecks() {

	GUITestBase* tb = AppContext::getGUITestBase();
	Q_ASSERT(tb);

	GUITests additionalChecks = tb->getTests(GUITestBase::ADDITIONAL);

	additionalChecks.append(tests);
	tests = additionalChecks;
}

void GUITestTask::run() {

	foreach (GUITest *test, tests) {
		launchTest(test);

		if (hasError()) {
			return;
		}
	}
}

void GUITestTask::launchTest(GUITest* test) {

	Q_ASSERT(test);

	TaskStateInfo os;
	if (test) {
		test->launch(os);
	}

	setError(os.getError());
}

Task::ReportResult GUITestTask::report() {

	QString testResult = successResult;
    if (hasError()) {
		testResult = getError();
	}

	writeTestResult(testResult);

	exit(0);
}

void GUITestTask::writeTestResult(const QString& result) const {

	printf("%s\n", (QString(GUITESTING_REPORT_PREFIX) + ":" + result).toUtf8().data());
}

}
