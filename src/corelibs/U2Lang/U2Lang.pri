# include (U2Lang.pri)

MODULE_ID=U2Lang
include( ../../ugene_lib_common.pri )
UGENE_RELATIVE_DESTDIR = ''

QT += xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
DEFINES+= QT_FATAL_ASSERT BUILDING_U2LANG_DLL

LIBS += -L../../_release -lU2Core

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS += -L../../_debug -lU2Cored
        LIBS -= -L../../_release -lU2Core
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
