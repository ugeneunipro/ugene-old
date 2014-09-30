# include (api_tests.pri)

PLUGIN_ID=api_tests
PLUGIN_NAME=Tests for UGENE 2.0 public API
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
}

LIBS += -lsamtools -L../../_release -lU2Script

# Force re-linking when lib changes
unix:POST_TARGETDEPS += ../../_release/libsamtools.a
# Same options which samtools is built with
DEFINES+="_FILE_OFFSET_BITS=64" _LARGEFILE64_SOURCE _USE_KNETFILE
INCLUDEPATH += ../../libs_3rdparty/samtools/src ../../libs_3rdparty/samtools/src/samtools
win32:INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/win32
win32:LIBS += -lws2_32
win32:DEFINES += _USE_MATH_DEFINES "inline=__inline" "__func__=__FUNCTION__" "R_OK=4" "atoll=_atoi64" "alloca=_alloca"

win32-msvc2013 {
    DEFINES += NOMINMAX _XKEYCHECK_H
    LIBS += -L../../_release -lzlib
}

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        LIBS -= -lsamtools -L../../_release -lU2Script
        LIBS += -lsamtoolsd -L../../_debug -lU2Scriptd

        win32-msvc2013 {
            LIBS -= -L../../_release -lzlib
            LIBS += -L../../_debug -lzlibd
        }

        unix:POST_TARGETDEPS -= ../../_release/libsamtools.a
        unix:POST_TARGETDEPS += ../../_debug/libsamtoolsd.a
    }
}
