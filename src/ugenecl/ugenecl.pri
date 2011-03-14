# include (ugenecl.pri)

include( ../ugene_globals.pri )
UGENE_RELATIVE_DESTDIR = ''

QT += xml network script webkit
TEMPLATE = app
CONFIG +=qt dll thread debug_and_release console
DEFINES+= QT_DLL QT_FATAL_ASSERT
INCLUDEPATH += src _tmp ../include ../corelibs/U2Private/src

LIBS += -L../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2Test -lU2Remote -lU2Misc -lU2Lang -lU2Private -lugene_db

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = ugenecld
        DEFINES+=_DEBUG
        DESTDIR=../_debug
        MOC_DIR=_tmp/moc/debug
        OBJECTS_DIR=_tmp/obj/debug
        LIBS -= -L../_release -lU2Core -lU2Algorithm -lU2Formats -lU2Gui -lU2Test -lU2Remote -lU2Misc -lU2Lang -lU2Private -lugene_db
        LIBS += -L../_debug -lU2Cored -lU2Algorithmd -lU2Formatsd -lU2Guid -lU2Testd -lU2Remoted -lU2Miscd -lU2Langd -lU2Privated -lugene_dbd
    }

    CONFIG(release, debug|release) {
        TARGET = ugenecl
        DESTDIR=../_release
        DEFINES+=NDEBUG
        MOC_DIR=_tmp/moc/release
        OBJECTS_DIR=_tmp/obj/release   
    }

}

UI_DIR=_tmp/ui
RCC_DIR=_tmp/rcc

win32 {
    LIBS += -luser32     # to import CharToOemA with nmake build

    QMAKE_CXXFLAGS_WARN_ON = -W3
    QMAKE_CFLAGS_WARN_ON = -W3
    RC_FILE = ugenecl.rc
}

macx {
    RC_FILE = images/ugenecl_mac.icns
}



unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}
