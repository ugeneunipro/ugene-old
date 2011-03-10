# include (U2Formats.pri)

MODULE_ID=U2Formats
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

DEFINES+= QT_FATAL_ASSERT BUILDING_U2FORMATS_DLL

LIBS += -L../../_release -lU2Core -lU2Algorithm

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Algorithm
        LIBS += -L../../_debug -lU2Cored -lU2Algorithmd
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

