include (GUITestBase.pri)

# Input
HEADERS += src/GUITestBasePlugin.h \
           src/GUIInitialChecks.h \
           src/GUIOpenFileTests.h \
           src/GUISaveFileTests.h \
           src/QtUtils.h \
           src/GUIDialogUtils.h \
           src/ToolTipUtils.h \
           src/AppUtils.h \
           src/ProjectUtils.h

SOURCES += src/GUITestBasePlugin.cpp \
           src/GUIInitialChecks.cpp \
           src/GUIOpenFileTests.cpp \
           src/GUISaveFileTests.cpp \
           src/QtUtils.cpp \
           src/GUIDialogUtils.cpp \
           src/ToolTipUtils.cpp \
           src/AppUtils.cpp \
           src/ProjectUtils.cpp
