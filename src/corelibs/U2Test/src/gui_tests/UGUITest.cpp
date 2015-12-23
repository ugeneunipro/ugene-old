#include "UGUITest.h"
#include "core/GUITestOpStatus.h"

#include <QApplication>
#include <QDate>
#include <QDesktopWidget>
#include <QDir>
#include <QScreen>

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

const QString UGUITest::testDir = getTestDir();
const QString UGUITest::dataDir = getDataDir();
const QString UGUITest::sandBoxDir = testDir + "_common_data/scenarios/sandbox/";
const QString UGUITest::screenshotDir = getScreenshotDir();

} // namespace
