# include (U2Private.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2Private
include( ../../ugene_lib_common.pri )

QT += xml
DEFINES+= QT_FATAL_ASSERT BUILDING_U2PRIVATE_DLL
LIBS += -L../../_release -lU2Core -lU2Formats

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Formats
        LIBS += -L../../_debug -lU2Cored -lU2Formatsd
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

win32 {
    LIBS += Advapi32.lib
}
