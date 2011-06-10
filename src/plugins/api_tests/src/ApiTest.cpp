#include "ApiTest.h"

#include <gtest/gtest.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>

#include <U2Test/TestRunnerSettings.h>


namespace U2 {

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
    QString dataDir = env->getVar("COMMON_DATA_DIR");
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
    int res = RUN_ALL_TESTS();
    if(0 != res) {
        stateInfo.setError(tr("Gtest has exited with code %1.").arg(res));
    }
}

void GTest_APITest::cleanup() {
    AppContext::getAppSettings()->getTestRunnerSettings()->removeVar(tcase);
}

} //namespace
