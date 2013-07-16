#include "GUITest.h"

namespace U2 {

#ifdef Q_OS_MAC
const QString GUITest::testDir = "../../../../../../test/";
#else
const QString GUITest::testDir = "../../test/";
#endif
const QString GUITest::dataDir = "../../data/";

} // namespace
