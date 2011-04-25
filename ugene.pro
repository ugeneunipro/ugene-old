include( src/ugene_globals.pri )

TEMPLATE = subdirs 
CONFIG += ordered debug_and_release
use_bundled_zlib() {
    SUBDIRS += src/libs_3rdparty/zlib
}

SUBDIRS += \
          src/libs_3rdparty/qscore \
          src/libs_3rdparty/sqlite3 \
          src/corelibs/U2Core \
          src/corelibs/U2Test \
          src/corelibs/U2Algorithm \
          src/corelibs/U2Formats \
          src/corelibs/U2Misc \
          src/corelibs/U2Lang \
          src/corelibs/U2Private \
          src/corelibs/U2Gui \
          src/corelibs/U2Remote \
          src/corelibs/U2View \
          src/corelibs/U2Designer \
          src/ugeneui \
          src/ugenecl \
          src/ugenem \
          src/plugins_3rdparty/ball \
          src/plugins_3rdparty/bowtie \
          src/plugins_3rdparty/sitecon \
          src/plugins_3rdparty/umuscle \
          src/plugins_3rdparty/hmm2 \
          src/plugins_3rdparty/gor4 \
          src/plugins_3rdparty/psipred \
          src/plugins_3rdparty/phylip \
          src/plugins_3rdparty/kalign \
          src/plugins_3rdparty/ptools \
          src/plugins/assembly_browser \
          src/plugins/biostruct3d_view \
          src/plugins/chroma_view \
          src/plugins/circular_view \
          src/plugins/annotator \
          src/plugins/dbi_sqlite \
          src/plugins/dbi_file \
          src/plugins/dbi_bam \
          src/plugins/dna_export \
          src/plugins/dna_stat \
          src/plugins/dna_graphpack \
          src/plugins/orf_marker \
          src/plugins/workflow_designer \
          src/plugins/repeat_finder \
          src/plugins/test_runner \
          src/plugins/perf_monitor \
          src/plugins/smith_waterman \
          src/plugins_3rdparty/primer3 \
          src/plugins/enzymes \
          src/plugins/remote_blast \
          src/plugins/genome_aligner \
          src/plugins/weight_matrix \
          src/plugins/dotplot \
          src/plugins/query_designer \
          src/plugins/external_tool_support \
          src/plugins/remote_service \
	  src/plugins/expert_discovery \
          src/plugins/CoreTests

use_cuda() {
    SUBDIRS += src/plugins/cuda_support
}

use_opencl() {
    SUBDIRS += src/plugins/opencl_support
}

use_sse2() {
    SUBDIRS += src/plugins_3rdparty/hmm3
}

exclude_list_enabled() {
    SUBDIRS -= src/plugins/dbi_file
    SUBDIRS -= src/plugins/dbi_sqlite
    SUBDIRS -= src/plugins/dbi_bam
    SUBDIRS -= src/plugins/assembly_browser
    SUBDIRS -= src/plugins/CoreTests
    SUBDIRS -= src/plugins/test_runner
    SUBDIRS -= src/plugins/perf_monitor
}


#create target directories
win32 {
    system( if not exist src\\_debug mkdir src\\_debug )
    system( if not exist src\\_release mkdir src\\_release )    
} else {
    system( cd src && [ -d _debug ] || mkdir _debug )
    system( cd src && [ -d _release ] || mkdir _release )
    system( cp ./installer/_common_data/ugene src/_release/ugene )
    system( cp ./installer/_common_data/ugened src/_debug/ugened )
}


#prepare translations
UGENE_TRANSL_IDX   = 0          1          2
UGENE_TRANSL_FILES = russian.ts english.ts czech.ts
UGENE_TRANSL_TAG   = ru         en         cs

UGENE_TRANSL_DIR   = transl
UGENE_TRANSL_QM_TARGET_DIR = src/_debug src/_release

#detecting lrelease binary
win32 : UGENE_DEV_NULL = nul
unix : UGENE_DEV_NULL = /dev/null

UGENE_LRELEASE =
UGENE_LUPDATE = 
system(lrelease -version > $$UGENE_DEV_NULL 2> $$UGENE_DEV_NULL) {
    UGENE_LRELEASE = lrelease
    UGENE_LUPDATE = lupdate
} else : system(lrelease-qt4 -version > $$UGENE_DEV_NULL 2> $$UGENE_DEV_NULL) {
    UGENE_LRELEASE = lrelease-qt4
    UGENE_LUPDATE = lupdate-qt4
}

#foreach 'language'
for( i, UGENE_TRANSL_IDX ) {
    UGENE_TRANSLATIONS = 

    curTranslFile = $$member( UGENE_TRANSL_FILES, $$i )
    curTranslTag  = $$member( UGENE_TRANSL_TAG, $$i )

    #foreach project directory
    for( prj_dir, SUBDIRS ) {
        #look for file and add it to translation list if it exists
        translFile = $$prj_dir/$$UGENE_TRANSL_DIR/$$curTranslFile   # 'project/transl/english.ts' etc.
        exists( $$translFile ) {
            UGENE_TRANSLATIONS += $$translFile
#            system( $$UGENE_LUPDATE $$translFile ) FIXME
        }
    }
    for( targetDir, UGENE_TRANSL_QM_TARGET_DIR ) {
        targetQmFile = $$targetDir/transl_$$curTranslTag            # 'transl_en.qm' etc.
        targetQmFile = $$join( targetQmFile, , , .qm )              # special workaround for adding suffix started with '.'

        !isEmpty(UGENE_LRELEASE) {
            message( Generating traslations for language: $$curTranslTag )
            system( $$UGENE_LRELEASE $$UGENE_TRANSLATIONS -qm $$targetQmFile > $$UGENE_DEV_NULL ) 
        } else {
            message( Cannot generate translations: no lrelease binary found )
        }
    }
}


unix {
    system( chmod a+x ./src/gen_bin_script.cmd && ./src/gen_bin_script.cmd $$UGENE_INSTALL_DIR ugene > ugene; chmod a+x ugene )
    binscript.files += ugene
    binscript.path = $$UGENE_INSTALL_BINDIR

# to copy ugene executable to /usr/lib/ugene directory
    ugene_starter.files = ./src/_release/ugene
    ugene_starter.path = $$UGENE_INSTALL_DIR

    transl.files = ./src/_release/transl_en.qm ./src/_release/transl_ru.qm
    transl.path = $$UGENE_INSTALL_DIR
    
    plugins.files = ./src/_release/plugins/*
    plugins.path = $$UGENE_INSTALL_DIR/plugins

    scripts.files += scripts/*
    scripts.path = $$UGENE_INSTALL_DIR/scripts

    data.files += data/*
    data.path = $$UGENE_INSTALL_DATA

    desktop.files += installer/_common_data/ugene.desktop
    desktop.path = $$UGENE_INSTALL_DESKTOP

    icons.files += installer/_common_data/ugene.png installer/_common_data/ugene.xpm
    icons.path = $$UGENE_INSTALL_PIXMAPS

    manual.files += installer/_common_data/ugene.1.gz
    manual.path = $$UGENE_INSTALL_MAN

    INSTALLS += binscript ugene_starter transl plugins scripts data desktop icons manual
}
