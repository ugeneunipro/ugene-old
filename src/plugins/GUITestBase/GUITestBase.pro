include (GUITestBase.pri)

# Input
HEADERS +=  src/GUITestBasePlugin.h \
            src/GTUtilsApp.h \
            src/GTUtilsDialog.h \
            src/GTUtilsDocument.h \
            src/GTUtilsLog.h \
            src/GTUtilsMdi.h \
            src/GTUtilsProject.h \
            src/GTUtilsProjectTreeView.h \
            src/GTUtilsAnnotationsTreeView.h \
            src/GTUtilsToolTip.h \
            src/GTUtilsTaskTreeView.h \
            src/tests/GUIInitialChecks.h \
            src/tests/common_scenarios/project/GTTestsProject.h \
            src/tests/common_scenarios/project/relations/GTTestsProjectRelations.h \
            src/tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.h \
            src/tests/common_scenarios/project/multiple_docs/GTTestsProjectMultipleDocs.h \
            src/tests/common_scenarios/project/anonymous_project/GTTestsProjectAnonymousProject.h \
            src/tests/common_scenarios/project/user_locking/GTTestsProjectUserLocking.h \
            src/api/GTComboBox.h \
            src/api/GTFileDialog.h \
            src/api/GTFile.h \
            src/api/GTGlobals.h \
            src/api/GTKeyboardDriver.h \
            src/api/GTLineEdit.h \
            src/api/GTMenu.h \
            src/api/GTMenuBar.h \
            src/api/GTMouse.h \
            src/api/GTMouseDriver.h \
            src/api/GTScrollBar.h \
            src/api/GTSystem.h \
            src/api/GTTreeWidget.h \
            src/api/GTToolbar.h \
            src/api/GTWidget.h \
            src/api/GTSequenceReadingModeDialog.h \
            src/api/GTSequenceReadingModeDialogUtils.h \
            src/tests/common_scenarios/project/sequence_exporting/from_project_view/GTTestsFromProjectView.h \
    src/GTSequenceViewUtils.h

SOURCES +=  src/GUITestBasePlugin.cpp \
            src/GTUtilsApp.cpp \
            src/GTUtilsDialog.cpp \
            src/GTUtilsDocument.cpp \
            src/GTUtilsLog.cpp \
            src/GTUtilsMdi.cpp \
            src/GTUtilsProject.cpp \
            src/GTUtilsProjectTreeView.cpp \
            src/GTUtilsAnnotationsTreeView.cpp \
            src/GTUtilsToolTip.cpp \
            src/GTUtilsTaskTreeView.cpp \
            src/tests/GUIInitialChecks.cpp \
            src/tests/common_scenarios/project/GTTestsProject.cpp \
            src/tests/common_scenarios/project/relations/GTTestsProjectRelations.cpp \
            src/tests/common_scenarios/project/sequence_exporting/GTTestsProjectSequenceExporting.cpp \
            src/tests/common_scenarios/project/user_locking/GTTestsProjectUserLocking.cpp \
            src/tests/common_scenarios/project/anonymous_project/GTTestsProjectAnonymousProject.cpp \
            src/tests/common_scenarios/project/multiple_docs/GTTestsProjectMultipleDocs.cpp \
            src/api/GTComboBox.cpp \
            src/api/GTFileDialog.cpp \
            src/api/GTFile.cpp \
            src/api/GTGlobals.cpp \
            src/api/GTKeyboardDriver.cpp \
            src/api/GTKeyboardDriverLinux.cpp \
            src/api/GTKeyboardDriverMac.cpp \
            src/api/GTKeyboardDriverWindows.cpp \
            src/api/GTLineEdit.cpp \
            src/api/GTMenu.cpp \
            src/api/GTMenuBar.cpp \
            src/api/GTMouse.cpp \
            src/api/GTMouseDriver.cpp \
            src/api/GTMouseDriverLinux.cpp \
            src/api/GTMouseDriverMac.cpp \
            src/api/GTMouseDriverWindows.cpp \
            src/api/GTScrollBar.cpp \
            src/api/GTSystem.cpp \
            src/api/GTToolbar.cpp \
            src/api/GTTreeWidget.cpp \
            src/api/GTWidget.cpp \
            src/api/GTSequenceReadingModeDialog.cpp \
            src/api/GTSequenceReadingModeDialogUtils.cpp \
            src/tests/common_scenarios/project/sequence_exporting/from_project_view/GTTestsFromProjectView.cpp \
    src/GTSequenceViewUtils.cpp




