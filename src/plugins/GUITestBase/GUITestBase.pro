include (GUITestBase.pri)

# Input
HEADERS += src/GUITestBasePlugin.h \
           src/GUIInitialChecks.h \
           src/GUIToolbarTests.h \
           src/QtUtils.h \
           src/GUIDialogUtils.h \
           src/ToolTipUtils.h \
           src/GTToolbar.h \
           src/AppUtils.h \
           src/DocumentUtils.h \
           src/LogUtils.h \
           src/ProjectUtils.h \
           src/ProjectTreeViewUtils.h \
           src/tests/common_scenarios/project/Project.h \
                src/api/GTMouseDriver.h \
                src/api/GTKeyboardDriver.h \
                src/api/GTComboBox.h \
                src/api/GTLineEdit.h \
                src/api/GTWidget.h \
                src/api/GTSystem.h \
                src/api/GTMouse.h \
                src/api/GTMenu.h \
                src/api/GTMenuBar.h \
                src/api/GTFileDialog.h \
				

SOURCES += src/GUITestBasePlugin.cpp \
           src/GUIInitialChecks.cpp \
           src/GUIToolbarTests.cpp \
           src/QtUtils.cpp \
           src/GUIDialogUtils.cpp \
           src/ToolTipUtils.cpp \
           src/GTToolbar.cpp \
           src/AppUtils.cpp \
           src/DocumentUtils.cpp \
           src/LogUtils.cpp \
           src/ProjectUtils.cpp \
           src/ProjectTreeViewUtils.cpp \
           src/tests/common_scenarios/project/Project.cpp \
                src/api/GTMouseDriver.cpp \
                src/api/GTKeyboardDriver.cpp \
                src/api/GTComboBox.cpp \
                src/api/GTLineEdit.cpp \
                src/api/GTWidget.cpp \
                src/api/GTSystem.cpp \
                src/api/GTMouse.cpp \
                src/api/GTMenu.cpp \
                src/api/GTMenuBar.cpp \
                src/api/GTKeyboardDriverWindows.cpp \
                src/api/GTKeyboardDriverMac.cpp \
                src/api/GTKeyboardDriverLinux.cpp \
                src/api/GTMouseDriverWindows.cpp \
                src/api/GTMouseDriverLinux.cpp \
                src/api/GTMouseDriverMac.cpp \
                src/api/GTFileDialog.cpp \
				

