#ifndef _U2_UGUI_TEST_H_
#define _U2_UGUI_TEST_H_

#include <U2Core/global.h>

#include <QtCore/QTimer>
#include <GTGlobals.h>
#include <core/GUITestOpStatus.h>
#include <core/GUITest.h>

namespace U2 {
using namespace HI;

class U2TEST_EXPORT UGUITest: public GUITest {
    Q_OBJECT
public:
    UGUITest(const QString &_name = "", const QString &_suite = "", int timeout = 240000) : GUITest(_name, _suite, timeout) {}
    virtual ~UGUITest(){}

    static const QString testDir;
    static const QString dataDir;
    static const QString screenshotDir;
    static const QString sandBoxDir;

};

typedef QList<UGUITest*> UGUITests;

}

#endif
