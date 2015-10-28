include (humimit.pri)

# Input
HEADERS += \
            src/GTGlobals.h \
#           src/base_dialogs/ColorDialogFiller.h \
#           src/base_dialogs/DefaultDialogFiller.h \
#           src/base_dialogs/FontDialogFiller.h \
#           src/base_dialogs/GTFileDialog.h \
#           src/base_dialogs/GTUtilsEscClicker.h \
#           src/base_dialogs/MessageBoxFiller.h \
#           src/core/global.h \
#           src/core/CustomScenario.h \
#           src/core/Log.h \
#           src/core/U2OpStatus.h \
#           src/core/U2SafePoints.h \
           src/drivers/GTKeyboardDriver.h \
           src/drivers/GTMouseDriver.h
#           src/qt_primitives/GTAction.h \
#           src/qt_primitives/GTCheckBox.h \
#           src/qt_primitives/GTComboBox.h \
#           src/qt_primitives/GTDoubleSpinBox.h \
#           src/qt_primitives/GTGroupBox.h \
#           src/qt_primitives/GTLineEdit.h \
#           src/qt_primitives/GTListWidget.h \
#           src/qt_primitives/GTMenu.h \
#           src/qt_primitives/GTMenuBar.h \
#           src/qt_primitives/GTPlainTextEdit.h \
#           src/qt_primitives/GTRadioButton.h \
#           src/qt_primitives/GTScrollBar.h \
#           src/qt_primitives/GTSlider.h \
#           src/qt_primitives/GTSpinBox.h \
#           src/qt_primitives/GTTabBar.h \
#           src/qt_primitives/GTTableView.h \
#           src/qt_primitives/GTTabWidget.h \
#           src/qt_primitives/GTTextEdit.h \
#           src/qt_primitives/GTToolbar.h \
#           src/qt_primitives/GTTreeView.h \
#           src/qt_primitives/GTTreeWidget.h \
#           src/qt_primitives/GTWebView.h \
#           src/qt_primitives/GTWidget.h \
#           src/qt_primitives/PopupChooser.h \
#           src/system/GTClipboard.h \
#           src/system/GTFile.h \
#           src/utils/GTKeyboardUtils.h \
#           src/utils/GTMouse.h \
#           src/utils/GTUtilsApp.h \
#           src/utils/GTUtilsDialog.h \
#           src/utils/GTUtilsMdi.h \
#           src/utils/GTUtilsToolTip.h
SOURCES += \
           src/GTGlobals.cpp \
#           src/base_dialogs/ColorDialogFiller.cpp \
#           src/base_dialogs/DefaultDialogFiller.cpp \
#           src/base_dialogs/FontDialogFiller.cpp \
#           src/base_dialogs/GTFileDialog.cpp \
#           src/base_dialogs/GTUtilsEscClicker.cpp \
#           src/base_dialogs/MessageBoxFiller.cpp \
#           src/core/CustomScenario.cpp \
#           src/core/Log.cpp \
           src/drivers/GTKeyboardDriver.cpp \
           src/drivers/GTKeyboardDriverLinux.cpp \
           src/drivers/GTKeyboardDriverMac.cpp \
           src/drivers/GTKeyboardDriverWindows.cpp \
           src/drivers/GTMouseDriver.cpp \
           src/drivers/GTMouseDriverLinux.cpp \
           src/drivers/GTMouseDriverMac.cpp \
           src/drivers/GTMouseDriverWindows.cpp
#           src/qt_primitives/GTAction.cpp \
#           src/qt_primitives/GTCheckBox.cpp \
#           src/qt_primitives/GTComboBox.cpp \
#           src/qt_primitives/GTDoubleSpinBox.cpp \
#           src/qt_primitives/GTGroupBox.cpp \
#           src/qt_primitives/GTLineEdit.cpp \
#           src/qt_primitives/GTListWidget.cpp \
#           src/qt_primitives/GTMenu.cpp \
#           src/qt_primitives/GTMenuBar.cpp \
#           src/qt_primitives/GTPlainTextEdit.cpp \
#           src/qt_primitives/GTRadioButton.cpp \
#           src/qt_primitives/GTScrollBar.cpp \
#           src/qt_primitives/GTSlider.cpp \
#           src/qt_primitives/GTSpinBox.cpp \
#           src/qt_primitives/GTTabBar.cpp \
#           src/qt_primitives/GTTableView.cpp \
#           src/qt_primitives/GTTabWidget.cpp \
#           src/qt_primitives/GTTextEdit.cpp \
#           src/qt_primitives/GTToolbar.cpp \
#           src/qt_primitives/GTTreeView.cpp \
#           src/qt_primitives/GTTreeWidget.cpp \
#           src/qt_primitives/GTWebView.cpp \
#           src/qt_primitives/GTWidget.cpp \
#           src/qt_primitives/PopupChooser.cpp \
#           src/system/GTClipboard.cpp \
#           src/system/GTFile.cpp \
#           src/utils/GTKeyboardUtils.cpp \
#           src/utils/GTMouse.cpp \
#           src/utils/GTUtilsApp.cpp \
#           src/utils/GTUtilsDialog.cpp \
#           src/utils/GTUtilsMdi.cpp \
#           src/utils/GTUtilsToolTip.cpp
