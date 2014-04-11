#include "GUITest.h"

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

#ifdef Q_OS_MAC
const QString GUITest::screenshotDir = "../../../../../../screenshotFol/";
#else
const QString GUITest::screenshotDir = "../../screenshotFol/";
#endif

const QString GUITest::testDir = getTestDir();
const QString GUITest::dataDir = getDataDir();
const QString GUITest::sandBoxDir = testDir + "_common_data/scenarios/sandbox/";



} // namespace
