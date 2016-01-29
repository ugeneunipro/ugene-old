#include (GUITestBase.pri)

PLUGIN_ID=GUITestBase
PLUGIN_NAME=GUI Test Base
PLUGIN_VENDOR=Unipro

QT += testlib
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets

INCLUDEPATH += ../../corelibs/U2View/_tmp/ ../../libs_3rdparty/QSpec/src
LIBS +=-L../../_release -L../../libs_3rdparty/humimit/ -lhumimit

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lhumimit
        LIBS += -L../../_debug -lhumimitd
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

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
