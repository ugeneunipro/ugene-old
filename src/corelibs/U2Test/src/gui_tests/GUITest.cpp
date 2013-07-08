#include "GUITest.h"
#include <QtCore/QDir>

namespace U2 {
const QString GUITest::getTrunkDir(){
    QDir current = QDir::current();
    while(current.dirName()!="trunk")
        current.cdUp();
    QString s = current.path();
    return s;
}

const QString GUITest::testDir = GUITest::getTrunkDir() + "/test/";
const QString GUITest::dataDir = GUITest::getTrunkDir() + "/data/";

} // namespace
