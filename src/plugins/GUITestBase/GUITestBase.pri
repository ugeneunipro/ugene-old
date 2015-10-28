#include (GUITestBase.pri)

PLUGIN_ID=GUITestBase
PLUGIN_NAME=GUI Test Base
PLUGIN_VENDOR=Unipro

QT += testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets

INCLUDEPATH += ../../corelibs/U2View/_tmp/

unix {
    !macx {
    LIBS += -lXtst
    }
    macx {
    QMAKE_LFLAGS += -framework ApplicationServices
    }
}

win32 {
    LIBS += User32.lib Gdi32.lib
}

macx {
    LIBS += -framework AppKit
}


include( ../../ugene_plugin_common.pri )
