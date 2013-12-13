include(../ugene_globals.pri)
UGENE_RELATIVE_DESTDIR = ''

QT += xml network webkit
TEMPLATE = app
CONFIG +=qt thread debug_and_release
DEFINES+= QT_DLL QT_FATAL_ASSERT
INCLUDEPATH += src _tmp

!debug_and_release|build_pass {

    CONFIG(debug, debug|release) {
        TARGET = ugenem
        DEFINES+=_DEBUG
        CONFIG +=console
        DESTDIR=../_debug
        MOC_DIR=_tmp/moc/debug
        OBJECTS_DIR=_tmp/obj/debug
    }

    CONFIG(release, debug|release) {
        TARGET = ugenem
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

    RC_FILE = ugenem.rc
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

unix_not_mac() : LIBS += -lX11

HEADERS += src/SendReportDialog.h \
           src/Utils.h

FORMS += src/ui/SendReportDialog.ui

SOURCES += src/main.cpp \
           src/SendReportDialog.cpp \
           src/Utils.cpp

RESOURCES += ugenem.qrc

