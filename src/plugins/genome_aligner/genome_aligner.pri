# include (genome_aligner.pri)

PLUGIN_ID=genome_aligner
PLUGIN_NAME=Genome aligner
PLUGIN_VENDOR=Unipro

include( ../../ugene_plugin_common.pri )

use_opencl(){
    DEFINES += OPENCL_SUPPORT
}

#adding CUDA specific parameters
use_cuda() {
    
    #manually convert INCLUDEPATH:
    GA_CUDA_INCLUDEPATH =
    for(path, INCLUDEPATH) {
        GA_CUDA_INCLUDEPATH += -I$${path}
    }
    for(path, QMAKE_INCDIR) {
        GA_CUDA_INCLUDEPATH += -I$${path}
    }
    
    LIBS += -L$$UGENE_CUDA_LIB_DIR -lcudart
    INCLUDEPATH += $$UGENE_CUDA_INC_DIR
    
    GA_NVCC_FLAGS = 

#TODO: win 64?
    !win32  {
        GA_NVCC_FLAGS += -Xcompiler -fPIC
    }
    
    GA_CUDA_LIBS += -lcudart
    GA_CUDA_FILES += src/suffix_search.cu

    nvzz.output = $$OBJECTS_DIR/${QMAKE_FILE_BASE}$$QMAKE_EXT_OBJ
    nvzz.commands = $$UGENE_NVCC $$GA_NVCC_FLAGS -c -I\"$$UGENE_CUDA_INC_DIR\" $$GA_CUDA_INCLUDEPATH -I$$QMAKE_INCDIR_QT \
                    -L\"$$UGENE_CUDA_LIB_DIR\" $$GA_CUDA_LIBS \
                    -o ${QMAKE_FILE_OUT} \
                    ${QMAKE_FILE_NAME}
    nvzz.depend_command = 
    nvzz.input = GA_CUDA_FILES

    QMAKE_EXTRA_COMPILERS += nvzz

    DEFINES += GA_BUILD_WITH_CUDA
}

