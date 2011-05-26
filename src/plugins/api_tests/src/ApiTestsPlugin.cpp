#include "ApiTestsPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#include <gtest/gtest.h>
#include <U2Test/GTest.h>
#include <U2Test/GTestFrameworkComponents.h>

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
    tcase = el.attribute("case");
    if (tcase.isEmpty()) {
        failMissingValue(tr("Test case is not set"));
    }

    AppContext::getAPITestEnvRegistry()->setEnvironment(tcase, env);
    
    QString ex = el.attribute("excluded");
    excluded = ex.split(QRegExp(",\\s*"));
}

void GTest_APITest::run() {
    QString filter = QString("--gtest_filter=*/%1.*").arg(tcase);
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

Task::ReportResult GTest_APITest::report() {
    AppContext::getAPITestEnvRegistry()->removeEnvironment(tcase);
    return ReportResult_Finished;
}

} // namespace U2
