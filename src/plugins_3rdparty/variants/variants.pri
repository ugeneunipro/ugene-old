# include (variants.pri)

PLUGIN_ID=variants
PLUGIN_NAME=CallVariants
PLUGIN_VENDOR=Unipro
CONFIG += warn_off
include( ../../ugene_plugin_common.pri )

DEFINES+="_FILE_OFFSET_BITS=64" _LARGEFILE64_SOURCE _USE_KNETFILE
win32 : DEFINES += _USE_MATH_DEFINES "inline=__inline" "__func__=__FUNCTION__" "R_OK=4" "atoll=_atoi64" "alloca=_alloca"

use_bundled_zlib() {
    INCLUDEPATH += ../../libs_3rdparty/zlib/src
    LIBS += -lzlib
}else{
    LIBS += -lz
}


win32 : LIBS += -lwsock32


INCLUDEPATH += src ../../include /usr/include
INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools 
INCLUDEPATH += ../../libs_3rdparty/samtools/src
INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/bcftools
win32 : INCLUDEPATH += ../../libs_3rdparty/samtools/src/samtools/win32

LIBS += -lsamtools

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        use_bundled_zlib() {
            LIBS -= -lzlib
            LIBS += -lzlibd
        }
        LIBS -= -lsamtools
        LIBS += -lsamtoolsd
    } 
}

win32-msvc2013 {
    DEFINES += _XKEYCHECK_H
}
