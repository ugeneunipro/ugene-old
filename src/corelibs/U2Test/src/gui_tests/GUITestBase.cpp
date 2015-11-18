#include "GUITestBase.h"

namespace U2 {

const QString GUITestBase::unnamedTestsPrefix = "test";

GUITestBase::~GUITestBase() {

    qDeleteAll(tests);
    qDeleteAll(preAdditional);
    qDeleteAll(postAdditionalActions);
    qDeleteAll(postAdditionalChecks);
}

bool GUITestBase::registerTest(GUITest *test, TestType testType) {

    Q_ASSERT(test);

    test->setName(nameUnnamedTest(test, testType));

    if (isNewTest(test, testType)) {
        addTest(test, testType);
        return true;
    }

    return false;
}

QString GUITestBase::nameUnnamedTest(GUITest* test, TestType testType) {

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

bool GUITestBase::isNewTest(GUITest *test, TestType testType) {

    return test && !findTest(test->getFullName(), testType);
}

void GUITestBase::addTest(GUITest *test, TestType testType) {

    if (test) {
        getMap(testType).insert(test->getFullName(), test);
    }
}

QString GUITestBase::getNextTestName(TestType testType) {

    int testsCount = getMap(testType).size();
    return unnamedTestsPrefix + QString::number(testsCount);
}

GUITest *GUITestBase::findTest(const QString &name, TestType testType) {
    GUITestMap map = getMap(testType);
    return map.value(name);
}

GUITest *GUITestBase::getTest(const QString &suite, const QString &name, TestType testType) {

    return getMap(testType).value(suite + ":" + name);
}

GUITest *GUITestBase::takeTest(const QString &suite, const QString &name, TestType testType) {

    return getMap(testType).take(suite + ":" + name);
}

GUITestMap& GUITestBase::getMap(TestType testType) {

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

GUITests GUITestBase::getTests(TestType testType) {

    GUITests testList = getMap(testType).values();

    return testList;
}

GUITests GUITestBase::takeTests(TestType testType) {

    GUITests testList = getMap(testType).values();
    getMap(testType).clear();

    return testList;
}


} // namespace
