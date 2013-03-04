include (ugene_version.pri)

UGENE_GLOBALS_DEFINED=1

DEFINES+=UGENE_VERSION=$${UGENE_VERSION}
DEFINES+=UGENE_VER_MAJOR=$${UGENE_VER_MAJOR}
DEFINES+=UGENE_VER_MINOR=$${UGENE_VER_MINOR}
DEFINES+=UGENE_VER_PATCH=$${UGENE_VER_PATCH}

#win32 : CONFIG -= flat  #group the files within the source/header group depending on the directory they reside in file system
win32 : QMAKE_CXXFLAGS += /MP # use parallel build with nmake
win32 : DEFINES+= _WINDOWS

win32 : QMAKE_CFLAGS_RELEASE = -O2 -Oy- -MD
win32 : QMAKE_CXXFLAGS_RELEASE = -O2 -Oy- -MD
win32 : QMAKE_LFLAGS_RELEASE = /INCREMENTAL:NO /MAP /MAPINFO:EXPORTS

macx : QMAKE_CXXFLAGS += -Wno-attributes # Ignore "'weak_import' attribute ignored" warning coming from OpenCL headers

isEmpty( INSTALL_PREFIX )  : INSTALL_PREFIX  = /usr

isEmpty( INSTALL_BINDIR )  : INSTALL_BINDIR  = $$INSTALL_PREFIX/bin
isEmpty( INSTALL_LIBDIR )  {
    INSTALL_LIBDIR  = $$INSTALL_PREFIX/lib
}

isEmpty( INSTALL_MANDIR )  : INSTALL_MANDIR  = $$INSTALL_PREFIX/share/man
isEmpty( INSTALL_DATADIR ) : INSTALL_DATADIR = $$INSTALL_PREFIX/share

isEmpty( UGENE_INSTALL_DESKTOP ) : UGENE_INSTALL_DESKTOP = $$INSTALL_DATADIR/applications
isEmpty( UGENE_INSTALL_PIXMAPS ) : UGENE_INSTALL_PIXMAPS = $$INSTALL_DATADIR/pixmaps
isEmpty( UGENE_INSTALL_DATA )    : UGENE_INSTALL_DATA    = $$INSTALL_DATADIR/ugene/data
isEmpty( UGENE_INSTALL_DIR )     : UGENE_INSTALL_DIR     = $$INSTALL_LIBDIR/ugene
isEmpty( UGENE_INSTALL_BINDIR )  : UGENE_INSTALL_BINDIR  = $$INSTALL_BINDIR
isEmpty( UGENE_INSTALL_MAN )     : UGENE_INSTALL_MAN     = $$INSTALL_MANDIR/man1

CONFIG(x64) {
    DEFINES += UGENE_X86_64
    win32 : QMAKE_LFLAGS *= /MACHINE:X64
} else:CONFIG(ppc) {
    DEFINES += UGENE_PPC
} else {
    DEFINES += UGENE_X86
}

macx : DEFINES += RUN_WORKFLOW_IN_THREADS

# uncomment when building on Cell BE
# UGENE_CELL = 1

# Checking if processor is SSE2 capable.
# On Windows UGENE relies on run-time check.
#
# Needed for:
#  1) adding -msse2 compilation flag if needed (currently uhmmer and smith_waterman2)
#  2) performing run-time check using cpuid instruction on intel proccessors.

isEmpty( UGENE_SSE2_DETECTED ) {
    UGENE_SSE2_DETECTED = 0

    !win32 : exists( /proc/cpuinfo ) {
        system( grep sse2 /proc/cpuinfo > /dev/null ) {
            UGENE_SSE2_DETECTED = 1
        }
    }
    macx {
        !ppc{
            system(/usr/sbin/system_profiler SPHardwareDataType | grep Processor | grep Intel > /dev/null) {
               UGENE_SSE2_DETECTED = 1
            } 
        }
    }
}

defineTest( use_sse2 ) {
    win32 : return (true)
    contains( UGENE_SSE2_DETECTED, 1 ) : return (true)
    return (false)
}

# CUDA environment
UGENE_NVCC         = nvcc
UGENE_CUDA_LIB_DIR = $$(CUDA_LIB_PATH)
UGENE_CUDA_INC_DIR = $$(CUDA_INC_PATH)

# CUDA detection tools
isEmpty(UGENE_CUDA_DETECTED) : UGENE_CUDA_DETECTED = 0
defineTest( use_cuda ) {
    contains( UGENE_CUDA_DETECTED, 1) : return (true)
    return (false)
}

# OPENCL detection tools
isEmpty(UGENE_OPENCL_DETECTED) : UGENE_OPENCL_DETECTED = 1
defineTest( use_opencl ) {
    contains( UGENE_OPENCL_DETECTED, 1) : return (true)
    return (false)
}

# establishing binary-independet data directory for *nix installation
unix {
    DEFINES *= UGENE_DATA_DIR=\\\"$$UGENE_INSTALL_DATA\\\"
}

# new conditional function for case 'unix but not macx'
defineTest( unix_not_mac ) {
    unix : !macx {
        return (true)
    }
    return (false)
}


#By default, UGENE uses bundled zlib (libs_3rdparty/zlib) on windows and mac os.
#On any OS it can be forced using the following variable

# UGENE_USE_BUNDLED_ZLIB = 1

defineTest( use_bundled_zlib ) {
    contains( UGENE_USE_BUNDLED_ZLIB, 1 ) : return (true)
    contains( UGENE_USE_BUNDLED_ZLIB, 0 ) : return (false)
    unix_not_mac() : return (false)
    return (true)
}

#Variable enabling exclude list for ugene modules
#UGENE_EXCLUDE_LIST_ENABLED = 1
defineTest( exclude_list_enabled ) {
    contains( UGENE_EXCLUDE_LIST_ENABLED, 1 ) : return (true)
    return (false)
}
#Variable enabling exclude list for ugene non-free modules
defineTest( without_non_free ) {
    contains( UGENE_WITHOUT_NON_FREE, 1 ) : return (true)
    return (false)
}