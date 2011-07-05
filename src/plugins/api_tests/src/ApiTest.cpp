#include "ApiTest.h"

#include <gtest/gtest.h>

#include <U2Core/AppContext.h>
#include <U2Core/AppSettings.h>

#include <U2Test/TestRunnerSettings.h>


namespace U2 {

class GTestFilterBuilder {
public:
    bool addTestCase(const QString& testCase) {
        if (testCase.isEmpty() || filter.keys().contains(testCase)) {
            return false;
        }
        filter[testCase] = QStringList();
        return true;
    }

    bool addTest(const QString& testCase, const QString& test) {
        if (!filter.keys().contains(testCase)) {
            return false;
        }
        filter[testCase].append(test); 
        return true;
    }

    QString buildFilterString() const {
        QString filterString;
        QMapIterator<QString, QStringList> iter(filter);

        while(iter.hasNext()) {
            iter.next();
            const QStringList& tests = iter.value();
            if (tests.isEmpty()) {
                continue;
            }
            
            int i = 0;
            if (filterString.isEmpty()) {
                filterString = QString("--gtest_filter=*/%1.%2/*").arg(iter.key()).arg(tests.first());
                i = 1;
            }

            for (; i<tests.size(); i++) {
                const QString& test = tests.at(i);
                filterString += QString(":*/%1.%2/*").arg(iter.key()).arg(test);
            }
        }
        return filterString;
    }

private:
    QMap<QString, QStringList> filter;
};

void GTest_APITest::init(XMLTestFormat *tf, const QDomElement& el) {
    Q_UNUSED(tf);

    GTestFilterBuilder builder;

    QString testList = el.text();
    QTextStream stream(&testList);
    QString line = stream.readLine();
    QString testCase;
    while (!line.isNull()) {
        line = line.trimmed();
        if (line.startsWith('+')) {
            line.remove(0,1);
            line = line.trimmed();
            builder.addTest(testCase, line);
        } else if (!line.startsWith('-') && !line.startsWith('#')) {
            testCase = line;
            builder.addTestCase(line);
        }
        line = stream.readLine();
    }

    filter = builder.buildFilterString();
}

void GTest_APITest::prepare() {
    QString dataDir = env->getVar("COMMON_DATA_DIR");
    AppContext::getAppSettings()->getTestRunnerSettings()->setVar("COMMON_DATA_DIR", dataDir);
}

void GTest_APITest::run() {
    if (filter.isEmpty()) {
        return;
    }

    QByteArray ba = filter.toLocal8Bit();
    QByteArray empty;
    char *argv[] = { empty.data(), ba.data() };
    int argc = 2;
    ::testing::InitGoogleTest(&argc, argv);
    int res = RUN_ALL_TESTS();
    if(0 != res) {
        stateInfo.setError(tr("Gtest has exited with code %1.").arg(res));
    }
}

void GTest_APITest::cleanup() {
    AppContext::getAppSettings()->getTestRunnerSettings()->removeVar("COMMON_DATA_DIR");
}

} //namespace
