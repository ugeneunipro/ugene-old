# include (U2Core.pri)

MODULE_ID=U2Core
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

QT += network xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

DEFINES+= QT_FATAL_ASSERT BUILDING_U2CORE_DLL
use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
    LIBS += -lzlib
} else {
    LIBS += -lz
}

LIBS += -L../../_release
LIBS += -lugenedb

INCLUDEPATH += ../../libs_3rdparty/sqlite3/src

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS += -L../../_debug
        LIBS -= -L../../_release
        use_bundled_zlib() {
            LIBS += -lzlibd
            LIBS -= -lzlib
        }
        LIBS -= -lugenedb
        LIBS += -lugenedbd
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

# Special compiler flags for windows configuration
win32 {
    LIBS += Psapi.lib User32.lib
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
    LIBS += -lprocps
}

HEADERS += ../../include/U2Core/U2*.h
