#include "GUITest.h"

namespace U2 {

#ifdef Q_OS_MAC
const QString GUITest::testDir = "../../../../../../test/";
const QString GUITest::dataDir = "data/";
#else
const QString GUITest::testDir = "../../test/";
const QString GUITest::dataDir = "../../data/";
#endif


} // namespace
