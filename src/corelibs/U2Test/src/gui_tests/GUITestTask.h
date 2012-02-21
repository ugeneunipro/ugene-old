#ifndef _U2_GUI_TEST_TASK_H_
#define _U2_GUI_TEST_TASK_H_

#include <U2Core/Task.h>
#include <QtCore/QList>
#include "GUITest.h"

namespace U2 {

class GUITest;

class GUITestTask: public Task {
    Q_OBJECT
public:
    GUITestTask(GUITest* task);
    virtual ~GUITestTask();

    virtual void prepare();
    virtual void run();
    virtual ReportResult report();

    static const QString taskName;
    static const QString successResult;
private:
    GUITests tests;

    void writeTestResult(const QString &result) const;
    void launchTest(GUITest* test);

    void addChecks();
};

} // namespace

#endif
