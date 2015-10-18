include (U2Test.pri)

# Input
HEADERS += src/GTest.h \
           src/GTestFrameworkComponents.h \
           src/TestRunnerSettings.h \
           src/TestRunnerTask.h \
           src/gui_tests/CustomScenario.h \
           src/gui_tests/GUITest.h \
           src/gui_tests/GUITestBase.h \
           src/gui_tests/GUITestLauncher.h \
           src/gui_tests/GUITestOpStatus.h \
           src/gui_tests/GUITestService.h \
           src/gui_tests/GUITestTeamcityLogger.h \
           src/gui_tests/GUITestThread.h \
           src/gui_tests/GUITestWindow.h \
           src/gui_tests/MainThreadRunnable.h \
           src/gui_tests/MainThreadTimer.h \
           src/xmltest/XMLTestFormat.h \
           src/xmltest/XMLTestUtils.h

SOURCES += src/GTest.cpp \
           src/GTestFrameworkComponents.cpp \
           src/TestRunnerTask.cpp \
           src/gui_tests/CustomScenario.cpp \
           src/gui_tests/GUITest.cpp \
           src/gui_tests/GUITestBase.cpp \
           src/gui_tests/GUITestLauncher.cpp \
           src/gui_tests/GUITestService.cpp \
           src/gui_tests/GUITestTeamcityLogger.cpp \
           src/gui_tests/GUITestThread.cpp \
           src/gui_tests/GUITestWindow.cpp \
           src/gui_tests/MainThreadRunnable.cpp \
           src/gui_tests/MainThreadTimer.cpp \
           src/xmltest/XMLTestFormat.cpp \
           src/xmltest/XMLTestUtils.cpp

TRANSLATIONS += transl/english.ts \
                transl/russian.ts

FORMS += GUITestingWindow.ui
