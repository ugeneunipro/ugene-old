#include "UGUITestBase.h"

namespace U2 {

const QString UGUITestBase::unnamedTestsPrefix = "test";

UGUITestBase::~UGUITestBase() {

    qDeleteAll(tests);
    qDeleteAll(preAdditional);
    qDeleteAll(postAdditionalActions);
    qDeleteAll(postAdditionalChecks);
}

bool UGUITestBase::registerTest(HI::GUITest *test, TestType testType) {

    Q_ASSERT(test);

    test->setName(nameUnnamedTest(test, testType));

    if (isNewTest(test, testType)) {
        addTest(test, testType);
        return true;
    }

    return false;
}

QString UGUITestBase::nameUnnamedTest(HI::GUITest* test, TestType testType) {

    Q_ASSERT(test);
    if (!test) {
        return "";
    }

    QString testName = test->getName();
    if (testName.isEmpty()) {
        testName = getNextTestName(testType);
    }

    return testName;
}

bool UGUITestBase::isNewTest(HI::GUITest *test, TestType testType) {

    return test && !findTest(test->getFullName(), testType);
}

void UGUITestBase::addTest(HI::GUITest *test, TestType testType) {

    if (test) {
        getMap(testType).insert(test->getFullName(), test);
    }
}

QString UGUITestBase::getNextTestName(TestType testType) {

    int testsCount = getMap(testType).size();
    return unnamedTestsPrefix + QString::number(testsCount);
}

HI::GUITest *UGUITestBase::findTest(const QString &name, TestType testType) {
    GUITestMap map = getMap(testType);
    return map.value(name);
}

HI::GUITest *UGUITestBase::getTest(const QString &suite, const QString &name, TestType testType) {

    return getMap(testType).value(suite + ":" + name);
}

HI::GUITest *UGUITestBase::takeTest(const QString &suite, const QString &name, TestType testType) {

    return getMap(testType).take(suite + ":" + name);
}

GUITestMap& UGUITestBase::getMap(TestType testType) {

    switch(testType) {
    case PreAdditional:
        return preAdditional;
    case PostAdditionalChecks:
        return postAdditionalChecks;
    case PostAdditionalActions:
        return postAdditionalActions;

    default:
    case Normal: return tests;
    }
}

GUITests UGUITestBase::getTests(TestType testType) {

    GUITests testList = getMap(testType).values();

    return testList;
}

GUITests UGUITestBase::takeTests(TestType testType) {

    GUITests testList = getMap(testType).values();
    getMap(testType).clear();

    return testList;
}


} // namespace
