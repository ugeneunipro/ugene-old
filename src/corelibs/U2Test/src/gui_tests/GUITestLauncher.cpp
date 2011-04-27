#include "GUITestLauncher.h"
#include "GUITestBase.h"

#include <U2Core/AppContext.h>
#include <U2Gui/MainWindow.h>

#include <QtCore/QMap>

#define TIMEOUT 60000

namespace U2 {

void LaunchTestTask::run() {
    try {
        t->launch();
    }
    catch(const TestException &ex) {
        t->setError(ex.msg);
    }
}

void LaunchTestTask::prepare() {
    if(!t) {
        printf(tr("GUITesting:Empty test").toUtf8().data());
        //coreLog.info(tr("GUITesting:Empty test"));
        exit(0);
    }
}

Task::ReportResult LaunchTestTask::report() {
    if(t->isSuccessful()) {
        printf(tr("GUITesting:Success").toUtf8().data());
        //coreLog.info(tr("GUITesting:Success"));
    } else {
        printf(QString(tr("GUITesting:") + t->getError()).toUtf8().data());
        //coreLog.info(QString(tr("GUITesting:") + t->getError()).toUtf8().data());
    }
    exit(0);
}

void TestLauncher::run() {
    tests = AppContext::getGUITestBase()->getTests();
    if(tests.isEmpty()) {
        setError(tr("No tests to run"));
        return;
    }
    QString path = QCoreApplication::applicationFilePath();
    int finishedTest = 0;
    foreach(GUITest* t, tests) {
        if(isCanceled()) {
            return;
        }
        QProcess process;
        process.start(path, QStringList() << QString("--gui-test") + "=" + t->getName());
        if(!process.waitForStarted()) {
            results[t->getName()] = tr("Can't start instance of UGENE");
            continue;
        }
        if(!process.waitForFinished(TIMEOUT)) {
            process.kill();
            results[t->getName()] = tr("Exit by timeout");
        } else if(process.exitStatus() == QProcess::CrashExit) {
            results[t->getName()] = tr("UGENE crashed");
        } else {
            QString msg = "";
            QByteArray output = process.readAllStandardOutput();
            QTextStream stream(&output, QIODevice::ReadOnly);
            while(!stream.atEnd() && msg.isEmpty()) {
                QString str = stream.readLine();
                if(str.contains("GUITesting")) {
                    msg = str.split(":").last();
                }
            }
            results[t->getName()] = msg;
            finishedTest++;
            stateInfo.progress = finishedTest*100/tests.size();
            emit si_progressChanged();
        }
    }
}

QString TestLauncher::generateReport() const{
    QString res;
    res += "<table width=\"100%\">";
    res += QString("<tr><th>%1</th><th>%2</th></tr>").arg(tr("Test name")).arg(tr("Status"));

    QMap<QString, QString>::const_iterator i;
    for (i = results.begin(); i != results.end(); ++i) {
        QString color = "green";
        if(!i.value().contains("Success")) {
            color = "red";
        }
        res += QString("<tr><th><font color='%3'>%1</font></th><th><font color='%3'>%2</font></th></tr>").arg(i.key()).arg(i.value()).arg(color);
    }
    res+="</table>";
    return res;
}


}
