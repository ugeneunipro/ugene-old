#ifndef _U2_GUI_TEST_BASE_H_
#define _U2_GUI_TEST_BASE_H_

#include "GUITests.h"

#include <U2Core/global.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Core/GObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/MainWindow.h>
#include <U2View/ADVSingleSequenceWidget.h>

#include <QtGui>


namespace U2 {


class U2TEST_EXPORT GUITestBase {
public:
    ~GUITestBase() {
        foreach(GUITest* t, tests.values()) {
            delete t;
        }
    }
    bool registerTest(GUITest *t);
    GUITest *findTestByName(const QString &name);
    QList<GUITest*> getTests();

private:
    QMap<QString, GUITest*> tests;
};



}

#endif
