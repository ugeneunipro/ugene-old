#include "GUITest.h"
#include "GUITestOpStatus.h"
#if (QT_VERSION < 0x050000) //Qt 5
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>
#include <QtGui/QPixmap>
#else
#include <QtGui/QScreen>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDesktopWidget>
#endif
#include "QDir"

namespace U2 {

QString getTestDir(){
    bool ok;
    int i = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER").toInt(&ok);
#ifdef Q_OS_MAC
    if ( ok && i>1){
        return QString("../../../../../../test%1/").arg(i-1);
    }else{
        return QString("../../../../../../test/");
    }
#else
    if ( ok && i>1){
        return QString("../../test%1/").arg(i-1);
    }else{
        return QString("../../test/");
    }
#endif
}

QString getDataDir(){
    bool ok;
    int i = qgetenv("UGENE_GUI_TEST_SUITE_NUMBER").toInt(&ok);
#ifdef Q_OS_MAC
    if ( ok && i>1 ){
        return QString("data%1/").arg(i-1);
    }else{
        return QString("data/");
    }
#else
    if ( ok && i>1){
        return QString("../../data%1/").arg(i-1);
    }else{
        return QString("../../data/");
    }
#endif
}

QString getScreenshotDir(){
    QString result;
#ifdef Q_OS_MAC
    result = "../../../../../../screenshotFol/";
#else
    QString guiTestOutputDirectory = qgetenv("GUI_TESTING_OUTPUT");
    if(guiTestOutputDirectory.isEmpty()){
        result = QDir::homePath() + "/gui_testing_output/" +
                QDate::currentDate().toString("dd.MM.yyyy") + "/screenshots/";
    }else{
        result = guiTestOutputDirectory + "/gui_testing_output/" +
                QDate::currentDate().toString("dd.MM.yyyy") + "/screenshots/";
    }
#endif
    return result;
}

const QString GUITest::testDir = getTestDir();
const QString GUITest::dataDir = getDataDir();
const QString GUITest::sandBoxDir = testDir + "_common_data/scenarios/sandbox/";
const QString GUITest::screenshotDir = getScreenshotDir();

void GUITest::sl_fail(){
#if (QT_VERSION < 0x050000) // deprecated method
    QPixmap originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
#else
    QPixmap originalPixmap = QGuiApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId());
#endif
    originalPixmap.save(GUITest::screenshotDir + name + ".jpg");
    uiLog.error("GUItest timed out");
    uiLog.trace("\nGT_DEBUG_MESSAGE !!!FIRST FAIL");
    GUITestOpStatus os;// = new GUITestOpStatus();
    os.setError("time out");
}



} // namespace
