# include (U2Designer.pri)

MODULE_ID=U2Designer
include( ../../ugene_lib_common.pri )

UGENE_RELATIVE_DESTDIR = ''

QT += svg webkit
DEFINES+= QT_FATAL_ASSERT BUILDING_U2DESIGNER_DLL

LIBS += -L../../_release -lU2Core -lU2Lang -lU2Gui

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Lang -lU2Gui
        LIBS += -L../../_debug -lU2Cored -lU2Langd -lU2Guid
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

