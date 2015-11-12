#ifndef _U2_GUI_TEST_BASE_H_
#define _U2_GUI_TEST_BASE_H_

#include <U2Core/global.h>
#include <U2Core/U2IdTypes.h>
#include <U2Core/Task.h>
#include <U2Core/MultiTask.h>
#include <U2Core/GObject.h>
#include <U2Core/DNASequenceObject.h>
#include <U2Gui/MainWindow.h>
#include <U2View/ADVSingleSequenceWidget.h>

#include <QtGui>
#include "GUITest.h"

namespace U2 {

#define TESTNAME(className) #className
#define SUITENAME(className) QString(GUI_TEST_SUITE)

#define DIALOG_FILLER_DECLARATION(className, DialogName) \
    class className : public Filler { \
    public: \
        className(U2OpStatus& os):Filler(os, DialogName){} \
        virtual void run(); \
    }

#define DIALOG_FILLER_DEFFINITION(className) \
    void className::run()

#define GUI_TEST_CLASS_DECLARATION(className) \
    class className : public GUITest { \
    public: \
        className () : GUITest(TESTNAME(className), SUITENAME(className)){} \
    protected: \
        virtual void run(U2OpStatus &os); \
    };

#define GUI_TEST_CLASS_DECLARATION_SET_TIMEOUT(className, timeout) \
    class className : public GUITest { \
    public: \
        className () : GUITest(TESTNAME(className), SUITENAME(className), timeout){} \
    protected: \
        virtual void run(U2OpStatus &os); \
    };

#define GUI_TEST_CLASS_DEFINITION(className) \
    void className::run(U2OpStatus &os)

typedef QMap<QString, GUITest*> GUITestMap;

class U2TEST_EXPORT GUITestBase {
public:
    enum TestType {Normal, PreAdditional, PostAdditionalChecks, PostAdditionalActions} type;

    virtual ~GUITestBase();

    bool registerTest(GUITest *test, TestType testType = Normal);
    GUITest *getTest(const QString &suite, const QString &name, TestType testType = Normal); // removes item from GUITestBase

    GUITests getTests(TestType testType = Normal);

    static const QString unnamedTestsPrefix;

private:
    GUITestMap tests;
    GUITestMap preAdditional;
    GUITestMap postAdditionalChecks;
    GUITestMap postAdditionalActions;
     // GUI checks additional to the launched checks

    GUITest *findTest(const QString &name, TestType testType);

    GUITestMap &getMap(TestType testType);

    QString getNextTestName(TestType testType);

    bool isNewTest(GUITest *test, TestType testType);
    void addTest(GUITest *test, TestType testType);

    QString nameUnnamedTest(GUITest* test, TestType testType);
};

}

#endif
