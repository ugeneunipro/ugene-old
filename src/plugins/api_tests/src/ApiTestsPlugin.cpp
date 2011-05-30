#include "ApiTestsPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>
#include <U2Gui/MainWindow.h>

#include <gtest/gtest.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>
#include <U2Test/TestRunnerTask.h>
#include <U2Test/TestRunnerSettings.h>

#include <QtGui/QMenu>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QCoreApplication>


namespace U2 {

extern "C" Q_DECL_EXPORT U2::Plugin *U2_PLUGIN_INIT_FUNC()
{
    return new ApiTestsPlugin();
}

ApiTestsPlugin::ApiTestsPlugin() :
Plugin("UGENE 2.0 API tests", "Tests for UGENE 2.0 public API") {
    GTestFormatRegistry* tfr = AppContext::getTestFramework()->getTestFormatRegistry();
    XMLTestFormat *xmlTestFormat = qobject_cast<XMLTestFormat*>(tfr->findFormat("XML"));
    assert(xmlTestFormat!=NULL);
    XMLTestFactory* f = GTest_APITest::createFactory();
    bool res = xmlTestFormat->registerTestFactory(f);
    assert(res); Q_UNUSED(res);
}

void GTest_APITest::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);
    tcase = el.attribute("case");
    if (tcase.isEmpty()) {
        failMissingValue(tr("Test case is not set"));
    }
    
    QString testList = el.text();
    QTextStream stream(&testList);
    QString line = stream.readLine();
    while (!line.isNull()) {
        line = line.trimmed();
        if (line.startsWith('+')) {
            line.remove(0,1);
            line = line.trimmed();
            included.append(line);
        } else if (line.startsWith('-')) {
            line.remove(0,1);
            line = line.trimmed();
            excluded.append(line);
        }
        line = stream.readLine();
    }
}

void GTest_APITest::prepare() {
    TestRunnerTask* trTask = qobject_cast<TestRunnerTask*>(getParentTask());
    assert(trTask);
    GTestState* ts = trTask->getStateByTestMap().value(this);
    assert(ts);
    GTestSuite* suite = ts->getTestRef()->getSuite();
    QString dataDir = QFileInfo(suite->getURL()).absoluteDir().absolutePath() + "/_common_data";
    AppContext::getAppSettings()->getTestRunnerSettings()->setVar(tcase, dataDir);
}

void GTest_APITest::run() {
    if (included.isEmpty()) {
        return;
    }
    QString filter = QString("--gtest_filter=*/%1.%2/*").arg(tcase).arg(included.first());
    for (int i=1; i<included.size(); i++) {
        const QString& testName = included.at(i);
        filter += QString(":*/%1.%2/*").arg(tcase).arg(testName);
    }

    if (!excluded.isEmpty()) {
        filter += QString("-*/%1.%2/*").arg(tcase).arg(excluded.first());
        for(int i=1; i<excluded.size(); i++) {
            const QString& ex = excluded.at(i);
            filter += QString(":*/%1.%2/*").arg(tcase).arg(ex);
        }
    }
    
    QByteArray ba = filter.toLocal8Bit();
    char *argv[] = { "", ba.data() };
    int argc = 2;
    ::testing::InitGoogleTest(&argc, argv);
    if(0 != RUN_ALL_TESTS()) {
        stateInfo.setError("An error occurred.");
    }
}

void GTest_APITest::cleanup() {
    AppContext::getAppSettings()->getTestRunnerSettings()->removeVar(tcase);
}

} // namespace U2
