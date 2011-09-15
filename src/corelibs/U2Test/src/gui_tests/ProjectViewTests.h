#ifndef _PROJECT_VIEW_TESTS_H_
#define _PROJECT_VIEW_TESTS_H_

#include "GUITests.h"

#include <QtCore/QString>

namespace U2 {

class ProjectViewTests: public GUITest {
public:
    ProjectViewTests(const QString &name): GUITest(name) {}
    void openFile(const QString &path);
    void addObjectToView(const QString &objectName);
    void openDocumentInView(const QString &documentName);

protected:
    static const QString projectViewName;
};

class TaskViewTest: public GUITest {
	Q_OBJECT
public:
    TaskViewTest(const QString &name): GUITest(name) {}
    QString getTaskProgress(const QString &taskName);
    QString getTaskState(const QString &taskName);
    void cancelTask(const QString &taskName);

protected:
    static const QString taskViewWidgetName;
};



}

#endif