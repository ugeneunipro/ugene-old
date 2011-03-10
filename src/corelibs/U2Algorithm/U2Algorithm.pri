# include (U2Algorithm.pri)

MODULE_ID=U2Algorithm
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''
DEFINES+= QT_FATAL_ASSERT BUILDING_U2ALGORITHM_DLL

LIBS += -L../../_release -lU2Core

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core
        LIBS += -L../../_debug -lU2Cored 
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
