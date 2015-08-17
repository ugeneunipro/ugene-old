# include (U2Private.pri)

UGENE_RELATIVE_DESTDIR = ''
MODULE_ID=U2Private
include( ../../ugene_lib_common.pri )

QT += xml
DEFINES += QT_FATAL_ASSERT BUILDING_U2PRIVATE_DLL
LIBS += -L../../_release -lU2Core -lU2Formats -lbreakpad
INCLUDEPATH += ../../libs_3rdparty/breakpad/src

!debug_and_release|build_pass {
    CONFIG(debug, debug|release) {
        DESTDIR = ../../_debug
        LIBS -= -L../../_release -lU2Core -lU2Formats -lbreakpad
        LIBS += -L../../_debug -lU2Cored -lU2Formatsd -lbreakpadd
    }

    CONFIG(release, debug|release) {
        DESTDIR = ../../_release
    }
}

unix {
    target.path = $$UGENE_INSTALL_DIR/$$UGENE_RELATIVE_DESTDIR
    INSTALLS += target
}

freebsd {
    LIBS += -lexecinfo
}

win32 {
    LIBS += Advapi32.lib

    CONFIG(x64) {
        ASM += src/crash_handler/StackRollbackX64.asm
        masm.name = MASM compiler
        masm.input = ASM
        masm.output = ${QMAKE_FILE_BASE}.obj
        masm.commands = ml64 /Fo ${QMAKE_FILE_OUT} /c ${QMAKE_FILE_IN}
        QMAKE_EXTRA_COMPILERS += masm
    }
}

win32-msvc2013 {
    DEFINES += NOMINMAX
}

macx {
    LIBS += -framework Foundation -framework IOKit
}
