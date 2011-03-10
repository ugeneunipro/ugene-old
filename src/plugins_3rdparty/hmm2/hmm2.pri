# include (hmm2.pri)

PLUGIN_ID=hmm2
PLUGIN_NAME=HMM2
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

win32 {
    QMAKE_CXXFLAGS+=/wd4244 /wd4305
}


unix {
    contains( UGENE_CELL, 1 ) {
#       message( "building for Cell BE..." )
        INCLUDEPATH += $(CELL_TOP)/usr/include
        DEFINES += ALTIVEC USE_SDK30 UGENE_CELL
        LIBS += -m32 -lspe2 -lsync src/u_spu/hmmercell_spu.a -lm
        QMAKE_LIBDIR += $(CELL_TOP)/usr/lib
        QMAKE_CXXFLAGS += -maltivec
    

        QMAKE_CLEAN += src/u_spu/*.d src/u_spu/*.o src/u_spu/*.a src/u_spu/hmmercell_spu
        target.depends = spe
        spe.target = src/u_spu/hmmercell_spu.a
        spe.commands = cd src/u_spu && make -f makefile USE_SDK30=1

        QMAKE_EXTRA_TARGETS = spe
        PRE_TARGETDEPS = src/u_spu/hmmercell_spu.a
    }
}

#adding SSE2 gcc compiler flag if building on SSE2 capable CPU
use_sse2() {
    !win32 {
        QMAKE_CXXFLAGS += -msse2
        QMAKE_CFLAGS_DEBUG += -msse2
        QMAKE_CFLAGS_RELEASE += -msse2
    }
    DEFINES += HMMER_BUILD_WITH_SSE2
}

