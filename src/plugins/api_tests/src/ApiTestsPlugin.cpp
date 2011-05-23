#include "ApiTestsPlugin.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#include <gtest/gtest.h>

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
    APITestBase* base = AppContext::getAPITestBase();
    if (base) {
        base->setRunner(new APITestRunnerImpl());
    }
}

// launcher
//////////////////////////////////////////////////////////////////////////

void APITestRunnerImpl::launchTests(const QStringList& suiteUrls) {
    QStringList testNames;
    parseTestNames(suiteUrls, testNames);

    if (testNames.isEmpty()) {
        return;
    }

    QString filter = "--gtest_filter=" + testNames.first();
    for (int i=1, n=testNames.size(); i<n; i++) {
        filter += ":";
        filter += testNames.at(i);
    }

    // copies filter string to bytearray
    QByteArray f;
    for(int i=0, n = filter.size(); i<n; i++) {
        char ch = filter.at(i).toAscii();
        f[i] = ch;
    }

    char *argv[] = { "", f.data() };
    int argc = 2;

    ::testing::InitGoogleTest(&argc, argv);
    if(0 != RUN_ALL_TESTS()) {
    }
}

static const char INCLUDE_SYM = '+';
static const char EXCLUDE_SYM = '-';

void APITestRunnerImpl::parseTestNames(const QStringList& urls, QStringList& result) {
    foreach(QString url, urls) {
        QFile file(url);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return;
        }

        QString testCase;

        QTextStream in(&file);
        QString line = in.readLine();
        while (!line.isNull()) {
            line = line.trimmed();
            if (line.isEmpty()) {
                line = in.readLine();
                continue;
            }
            if (!line.startsWith(INCLUDE_SYM) && !line.startsWith(EXCLUDE_SYM)) {
                testCase = line.append("/0.");
            } else if (line.startsWith(INCLUDE_SYM)) {
                line.remove(0,1);
                line = line.trimmed();
                if (!line.isEmpty()) {
                    line.insert(0, testCase);
                    result.append(line);
                }
            }
            line = in.readLine();
        }
    }
}

} // namespace U2
