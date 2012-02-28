include (GUITestBase.pri)

# Input
HEADERS += src/GUITestBasePlugin.h \
           src/GUIInitialChecks.h \
           src/GUIOpenFileTests.h \
           src/GUISaveFileTests.h \
           src/GUIExportProjectTests.h \
	   src/GUIToolbarTests.h \
           src/QtUtils.h \
           src/GUIDialogUtils.h \
           src/ToolTipUtils.h \
	   src/ToolbarUtils.h \
           src/AppUtils.h \
           src/DocumentUtils.h \
           src/LogUtils.h \
           src/ProjectUtils.h \
           src/ProjectTreeViewUtils.h

SOURCES += src/GUITestBasePlugin.cpp \
           src/GUIInitialChecks.cpp \
           src/GUIOpenFileTests.cpp \
           src/GUISaveFileTests.cpp \
           src/GUIExportProjectTests.cpp \
	   src/GUIToolbarTests.cpp \
           src/QtUtils.cpp \
           src/GUIDialogUtils.cpp \
           src/ToolTipUtils.cpp \
	   src/ToolbarUtils.cpp \
           src/AppUtils.cpp \
           src/DocumentUtils.cpp \
           src/LogUtils.cpp \
           src/ProjectUtils.cpp \
           src/ProjectTreeViewUtils.cpp
