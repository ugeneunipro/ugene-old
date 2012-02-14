include (U2Test.pri)

# Input
HEADERS += src/GTest.h \
           src/GTestFrameworkComponents.h \
           src/TestRunnerTask.h \
           src/TestRunnerSettings.h \
           src/xmltest/XMLTestFormat.h \
           src/xmltest/XMLTestUtils.h \
           src/gui_tests/GUITest.h \
           src/gui_tests/GUITestTask.h \
           src/gui_tests/GUITestService.h \
           src/gui_tests/GUITestLauncher.h \
           src/gui_tests/GUITestBase.h
SOURCES += src/GTest.cpp \
           src/GTestFrameworkComponents.cpp \
           src/TestRunnerTask.cpp \
           src/xmltest/XMLTestFormat.cpp \
           src/xmltest/XMLTestUtils.cpp \
           src/gui_tests/GUITest.cpp \
           src/gui_tests/GUITestTask.cpp \
           src/gui_tests/GUITestService.cpp \
           src/gui_tests/GUITestLauncher.cpp \
           src/gui_tests/GUITestBase.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
