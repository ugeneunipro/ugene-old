# include (U2View.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2View
include( ../../ugene_lib_common.pri )

QT += svg webkit
equals(QT_MAJOR_VERSION, 5): QT -= webkit
equals(QT_MAJOR_VERSION, 5): QT += widgets webkitwidgets
DEFINES+= QT_FATAL_ASSERT BUILDING_U2VIEW_DLL
LIBS += -L../../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Remote -lU2Lang -lU2Gui

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        DESTDIR=../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Remote -lU2Lang -lU2Gui
        LIBS += -L../../_debug -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Remoted -lU2Langd -lU2Guid
    }

    CONFIG(release, debug|release) {
        DESTDIR=../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

