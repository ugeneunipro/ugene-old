# include (gtest.pri)
include( ../../ugene_globals.pri )
UGENE_RELATIVE_DESTDIR = ''

TARGET = gtest
TEMPLATE = lib
CONFIG +=thread debug_and_release
INCLUDEPATH += src
LIBS += -L../../_release

DEFINES+=GTEST_CREATE_SHARED_LIBRARY=1
#DEFINES+=GTEST_LINKED_AS_SHARED_LIBRARY=1

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = gtestd
        DEFINES+=_DEBUG
        CONFIG +=console
        DESTDIR=../../_debug/
        OBJECTS_DIR=_tmp/obj/debug
        LIBS -= -L../../_release 
        LIBS += -L../../_debug
    }

    CONFIG(release, debug|release) {
        TARGET = gtest
        DEFINES+=NDEBUG
        DESTDIR=../../_release/
        OBJECTS_DIR=_tmp/obj/release
    }
}


win32 {

    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON = -W3

    QMAKE_MSVC_PROJECT_NAME=lib_3rd_gtest
}


unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}