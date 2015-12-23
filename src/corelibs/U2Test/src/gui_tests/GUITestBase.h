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
#include <U2Test/UGUITest.h>

namespace U2 {

#define TESTNAME(className) #className
#define SUITENAME(className) QString(GUI_TEST_SUITE)

#define DIALOG_FILLER_DECLARATION(className, DialogName) \
    class className : public Filler { \
    public: \
        className(HI::GUITestOpStatus& os):Filler(os, DialogName){} \
        virtual void run(); \
    }

#define DIALOG_FILLER_DEFFINITION(className) \
    void className::run()

#define GUI_TEST_CLASS_DECLARATION(className) \
    class className : public UGUITest { \
    public: \
        className () : UGUITest(TESTNAME(className), SUITENAME(className)){} \
    protected: \
        virtual void run(HI::GUITestOpStatus &os); \
    };

#define GUI_TEST_CLASS_DECLARATION_SET_TIMEOUT(className, timeout) \
    class className : public UGUITest { \
    public: \
        className () : UGUITest(TESTNAME(className), SUITENAME(className), timeout){} \
    protected: \
        virtual void run(HI::GUITestOpStatus &os); \
    };

#define GUI_TEST_CLASS_DEFINITION(className) \
    void className::run(HI::GUITestOpStatus &os)

typedef QMap<QString, HI::GUITest*> GUITestMap;
typedef QList<HI::GUITest*> GUITests;

class U2TEST_EXPORT GUITestBase {
public:
    enum TestType {Normal, PreAdditional, PostAdditionalChecks, PostAdditionalActions} type;

    virtual ~GUITestBase();

    bool registerTest(HI::GUITest *test, TestType testType = Normal);
    HI::GUITest *getTest(const QString &suite, const QString &name, TestType testType = Normal);
    HI::GUITest *takeTest(const QString &suite, const QString &name, TestType testType = Normal); // removes item from GUITestBase

    GUITests getTests(TestType testType = Normal);
    GUITests takeTests(TestType testType = Normal); // removes items from GUITestBase

    GUITests getTestsWithoutRemoving(TestType testType = Normal);

    HI::GUITest *findTest(const QString &name, TestType testType = Normal);

    static const QString unnamedTestsPrefix;

private:
    GUITestMap tests;
    GUITestMap preAdditional;
    GUITestMap postAdditionalChecks;
    GUITestMap postAdditionalActions;
     // GUI checks additional to the launched checks

    GUITestMap &getMap(TestType testType);

    QString getNextTestName(TestType testType);

    bool isNewTest(HI::GUITest *test, TestType testType);
    void addTest(HI::GUITest *test, TestType testType);

    QString nameUnnamedTest(HI::GUITest* test, TestType testType);
};

}

#endif
