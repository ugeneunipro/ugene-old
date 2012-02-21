#include "GUITest.h"

#include <QtGui/QApplication>

namespace U2 {

const QString GUITest::testDir = "../../test/";
const QString GUITest::dataDir = "../../data/";

void GUITest::launch(U2OpStatus &os) {

    QThread *t = this->thread();
    moveToThread(QApplication::instance()->thread());

    execute(os);
}

} // namespace
