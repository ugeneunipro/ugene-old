/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2012 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

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
