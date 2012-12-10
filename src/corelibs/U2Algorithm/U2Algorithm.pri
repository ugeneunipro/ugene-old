# include (U2Algorithm.pri)

MODULE_ID=U2Algorithm
include( ../../ugene_lib_common.pri )

use_opencl(){
    DEFINES += OPENCL_SUPPORT
}

UGENE_RELATIVE_DESTDIR = ''
DEFINES+= QT_FATAL_ASSERT BUILDING_U2ALGORITHM_DLL

LIBS += -L../../_release -lU2Core -lU2Formats -lsamtools
use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
    LIBS += -lzlib
} else {
    LIBS += -lz
}

# Force re-linking when lib changes
unix:POST_TARGETDEPS += ../../_release/libsamtools.a
# Same options which samtools is built with
DEFINES+="_FILE_OFFSET_BITS=64" _LARGEFILE64_SOURCE _USE_KNETFILE
INCLUDEPATH += ../../libs_3rdparty/samtools/src ../../libs_3rdparty/samtools/src/samtools
win32:INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/win32
win32:LIBS+=-lws2_32

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Formats -lsamtools
        LIBS += -L../../_debug -lU2Cored -lU2Formatsd -lsamtoolsd

        unix:POST_TARGETDEPS -= ../../_release/libsamtools.a
        unix:POST_TARGETDEPS += ../../_debug/libsamtoolsd.a

        use_bundled_zlib() {
            LIBS += -lzlibd
            LIBS -= -lzlib
        }
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
