# include (U2Formats.pri)

MODULE_ID=U2Formats
include( ../../ugene_lib_common.pri )

use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
}

UGENE_RELATIVE_DESTDIR = ''

DEFINES+= QT_FATAL_ASSERT BUILDING_U2FORMATS_DLL

LIBS += -L../../_release -lU2Core -lU2Algorithm
LIBS += -lugenedb -lsamtools

win32-msvc2013 {
    DEFINES += NOMINMAX _XKEYCHECK_H
    LIBS += -lzlib
}

QT += sql

# Force re-linking when lib changes
unix:POST_TARGETDEPS += ../../_release/libsamtools.a
# Same options which samtools is built with
DEFINES+="_FILE_OFFSET_BITS=64" _LARGEFILE64_SOURCE _USE_KNETFILE
INCLUDEPATH += ../../libs_3rdparty/samtools/src ../../libs_3rdparty/samtools/src/samtools
win32:INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/win32
win32:LIBS += -lws2_32
win32:DEFINES += _USE_MATH_DEFINES "inline=__inline" "__func__=__FUNCTION__" "R_OK=4" "atoll=_atoi64" "alloca=_alloca"

INCLUDEPATH += ../../libs_3rdparty/sqlite3/src

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Algorithm -lugenedb -lsamtools
        LIBS += -L../../_debug -lU2Cored -lU2Algorithmd -lugenedbd -lsamtoolsd

        win32-msvc2013 {
            LIBS -= -lzlib
            LIBS += -lzlibd
        }

        unix:POST_TARGETDEPS -= ../../_release/libsamtools.a
        unix:POST_TARGETDEPS += ../../_debug/libsamtoolsd.a
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
