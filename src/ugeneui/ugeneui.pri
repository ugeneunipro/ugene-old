# include (ugeneui.pri)

include( ../ugene_globals.pri )
UGENE_RELATIVE_DESTDIR = ''

QT += xml network script webkit
TEMPLATE = app
CONFIG +=qt dll thread debug_and_release
DEFINES+= QT_DLL QT_FATAL_ASSERT
INCLUDEPATH += src _tmp ../include ../corelibs/U2Private/src

LIBS += -L../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2View -lU2Test -lU2Remote -lU2Misc -lU2Lang -lU2Private -lsqlite3

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = ugeneuid
        DEFINES+=_DEBUG
        CONFIG +=console
        DESTDIR=../_debug
        MOC_DIR=_tmp/moc/debug
        OBJECTS_DIR=_tmp/obj/debug
        LIBS -= -L../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2View -lU2Test -lU2Remote -lU2Misc -lU2Lang -lU2Private -lsqlite3
        LIBS += -L../_debug -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Guid -lU2Viewd -lU2Testd -lU2Remoted -lU2Miscd -lU2Langd -lU2Privated -lsqlite3d
    }

    CONFIG(release, debug|release) {
        TARGET = ugeneui
        DESTDIR=../_release
        DEFINES+=NDEBUG
        MOC_DIR=_tmp/moc/release
        OBJECTS_DIR=_tmp/obj/release

        FORCE_CONSOLE = $$(UGENE_BUILD_WITH_CONSOLE)
        !isEmpty( FORCE_CONSOLE ) : CONFIG +=console
    }

}

UI_DIR=_tmp/ui
RCC_DIR=_tmp/rcc

win32 {

    LIBS += -luser32     # to import CharToOemA with nmake build

    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON = -W3
    RC_FILE = ugeneui.rc
}

macx {
    RC_FILE = images/ugeneui.icns
}



unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

# Prepare version info for NSIS installer
win32 {
    !debug_and_release|build_pass {
        CONFIG(release, debug|release) {
            NSIS_LINE = "!define ProductVersion "$${UGENE_VERSION}"
            NSIS_FILE = $${DESTDIR}/version.nsis

            NSIS_LINE = $$replace(NSIS_LINE, "\\.","_")
            NSIS_LINE = $$replace(NSIS_LINE, "\\-","_")

            win32: system (echo $${NSIS_LINE} > $${NSIS_FILE})
            else:  system (echo \"$${NSIS_LINE}\" > $${NSIS_FILE})
        }
    }
}
