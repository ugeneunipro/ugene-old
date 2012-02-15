include (../../ugene_globals.pri)


PLUGIN_ID=smith_waterman
PLUGIN_NAME=Smith-Waterman
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

INCLUDEPATH += ../../corelibs/U2View/_tmp

#adding SSE2 gcc compiler flag if building on SSE2 capable CPU

use_sse2() {
    !win32 {
        QMAKE_CXXFLAGS += -msse2
        QMAKE_CFLAGS_DEBUG += -msse2
        QMAKE_CFLAGS_RELEASE += -msse2
    }
    DEFINES += SW2_BUILD_WITH_SSE2
}

#adding CUDA specific parameters
use_cuda() {

    LIBS += -L$$UGENE_CUDA_LIB_DIR -lcudart
    INCLUDEPATH += $$UGENE_CUDA_INC_DIR

    SW2_NVCC_FLAGS =

#TODO: win 64?
    !win32  {
        SW2_NVCC_FLAGS += -Xcompiler -fPIC
    }
    
    SW2_CUDA_LIBS += -lcudart
    SW2_CUDA_FILES += src/sw_cuda.cu

#workaround for nmake bug: nvcc failes with an internal error when launched from Makefile by nvcc
    win32 {
        SW2_CUDA_NULL_REDIRECT = > nul
    } else {
        SW2_CUDA_NULL_REDIRECT = 
    }
#manually convert INCLUDEPATH:
    SW2_CUDA_INCLUDEPATH =
    for(path, INCLUDEPATH) {
        SW2_CUDA_INCLUDEPATH += -I$${path}
    }
    for(path, QMAKE_INCDIR) {
        SW2_CUDA_INCLUDEPATH += -I$${path}
    }
#    message( $$SW2_CUDA_INCLUDEPATH )

    nvzz.output = $$OBJECTS_DIR/${QMAKE_FILE_BASE}$$QMAKE_EXT_OBJ
    nvzz.commands = $$UGENE_NVCC $$SW2_NVCC_FLAGS -c -I$$UGENE_CUDA_INC_DIR $$SW2_CUDA_INCLUDEPATH -I$$QMAKE_INCDIR_QT \
                    -L$$UGENE_CUDA_LIB_DIR $$SW2_CUDA_LIBS \
                    -o ${QMAKE_FILE_OUT} \
                    ${QMAKE_FILE_NAME} \
                    $$SW2_CUDA_NULL_REDIRECT
    nvzz.depend_command = 
    nvzz.input = SW2_CUDA_FILES

    QMAKE_EXTRA_COMPILERS += nvzz

    DEFINES += SW2_BUILD_WITH_CUDA
}

#adding OPENCL specific parameters
use_opencl() {
    DEFINES += SW2_BUILD_WITH_OPENCL
    DEFINES += OPENCL_SUPPORT
}
