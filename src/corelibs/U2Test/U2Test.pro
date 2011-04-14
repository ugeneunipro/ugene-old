include (U2Test.pri)

# Input
HEADERS += src/GTest.h \
           src/GTestFrameworkComponents.h \
           src/TestRunnerTask.h \
           src/xmltest/XMLTestFormat.h \
           src/xmltest/XMLTestUtils.h \
		   src/gui_tests/GUITests.h \
		   src/gui_tests/GUITestService.h \
		   src/gui_tests/ProjectViewTests.h \
		   src/gui_tests/GUITestLauncher.h \
		   src/gui_tests/GUITestBase.h 
SOURCES += src/GTest.cpp \
           src/GTestFrameworkComponents.cpp \
           src/TestRunnerTask.cpp \
           src/xmltest/XMLTestFormat.cpp \
           src/xmltest/XMLTestUtils.cpp \
		   src/gui_tests/GUITests.cpp \
		   src/gui_tests/GUITestService.cpp \
		   src/gui_tests/ProjectViewTests.cpp \
		   src/gui_tests/GUITestLauncher.cpp \
		   src/gui_tests/GUITestBase.cpp

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
