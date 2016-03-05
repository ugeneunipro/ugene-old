include (U2Test.pri)

# Input
HEADERS += src/GTest.h \
           src/GTestFrameworkComponents.h \
           src/TestRunnerSettings.h \
           src/TestRunnerTask.h \
           src/gui_tests/UGUITestBase.h \
           src/gui_tests/GUITestLauncher.h \
           src/gui_tests/GUITestService.h \
           src/gui_tests/GUITestTeamcityLogger.h \
           src/gui_tests/GUITestThread.h \
           src/gui_tests/GUITestWindow.h \
           src/xmltest/XMLTestFormat.h \
           src/xmltest/XMLTestUtils.h \
           src/gui_tests/UGUITest.h

SOURCES += src/GTest.cpp \
           src/GTestFrameworkComponents.cpp \
           src/TestRunnerTask.cpp \
           src/gui_tests/UGUITestBase.cpp \
           src/gui_tests/GUITestLauncher.cpp \
           src/gui_tests/GUITestService.cpp \
           src/gui_tests/GUITestTeamcityLogger.cpp \
           src/gui_tests/GUITestThread.cpp \
           src/gui_tests/GUITestWindow.cpp \
           src/xmltest/XMLTestFormat.cpp \
           src/xmltest/XMLTestUtils.cpp \
           src/gui_tests/UGUITest.cpp

TRANSLATIONS += transl/english.ts \
                transl/russian.ts

FORMS += src/gui_tests/GUITestingWindow.ui

if(exclude_list_enabled()|!exists( ../../libs_3rdparty/QSpec/QSpec.pro )) {

HEADERS -= src/gui_tests/UGUITest.h \
           src/gui_tests/UGUITestBase.h \
           src/gui_tests/GUITestLauncher.h \
           src/gui_tests/GUITestService.h \
           src/gui_tests/GUITestTeamcityLogger.h \
           src/gui_tests/GUITestThread.h \
           src/gui_tests/GUITestWindow.h

SOURCES -= src/gui_tests/UGUITest.cpp \
           src/gui_tests/UGUITestBase.cpp \
           src/gui_tests/GUITestLauncher.cpp \
           src/gui_tests/GUITestService.cpp \
           src/gui_tests/GUITestTeamcityLogger.cpp \
           src/gui_tests/GUITestThread.cpp \
           src/gui_tests/GUITestWindow.cpp
}
