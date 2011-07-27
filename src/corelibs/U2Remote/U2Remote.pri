# include (U2Remote.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2Remote
include( ../../ugene_lib_common.pri )

QT += network
DEFINES+= QT_FATAL_ASSERT BUILDING_U2REMOTE_DLL
INCLUDEPATH += ../U2Private/src

LIBS += -L../../_release -lU2Core -lU2Lang -lU2Gui -lU2Private

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Lang -lU2Gui -lU2Private
        LIBS += -L../../_debug -lU2Cored -lU2Langd -lU2Guid -lU2Privated
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

