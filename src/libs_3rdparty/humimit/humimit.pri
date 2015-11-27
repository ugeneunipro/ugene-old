# include (humimit.pri)

include( ../../ugene_globals.pri )
UGENE_RELATIVE_DESTDIR = ''

TARGET = humimit
TEMPLATE = lib
CONFIG += thread debug_and_release warn_off qt dll
INCLUDEPATH += src _tmp ../../include
LIBS += -L../../_release
QT += testlib 

greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets 

DEFINES += BUILDING_HUMIMIT_DLL
DEFINES += QT_DLL

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = humimitd
        DEFINES += _DEBUG
        CONFIG +=console
        DESTDIR=../../_debug/
        OBJECTS_DIR=_tmp/obj/debug
        LIBS -= -L../../_release
        LIBS += -L../../_debug
    }

    CONFIG(release, debug|release) {
        TARGET = humimit
        DEFINES+=NDEBUG
        DESTDIR=../../_release/
        OBJECTS_DIR=_tmp/obj/release
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


win32 {
    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON = -W3

    QMAKE_MSVC_PROJECT_NAME=lib_3rd_humimit
}


unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

