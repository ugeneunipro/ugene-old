# include (hmm3.pri)

PLUGIN_ID=hmm3
PLUGIN_NAME=HMM3
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

win32 {
    QMAKE_CXXFLAGS+=/wd4244 /wd4305
    DEFINES += _CRT_SECURE_NO_WARNINGS
}


#adding SSE2 gcc compiler flag if building on SSE2 capable CPU
use_sse2() {
    !win32 {
        QMAKE_CXXFLAGS += -msse2
        QMAKE_CFLAGS_DEBUG += -msse2
        QMAKE_CFLAGS_RELEASE += -msse2
    }
}
