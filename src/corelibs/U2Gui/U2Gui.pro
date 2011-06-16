include (U2Gui.pri)

# Input
HEADERS += src/AppSettingsGUI.h \
           src/MainWindow.h \
           src/Notification.h \
           src/NotificationWidget.h \
           src/ObjectViewModel.h \
           src/ObjectViewTasks.h \
           src/OpenViewTask.h \
           src/PluginViewer.h \
           src/ProjectParsing.h \
           src/ProjectView.h \
           src/UnloadDocumentTask.h \
           src/util_gui/AddExistingDocumentDialogController.h \
           src/util_gui/AddExistingDocumentDialogImpl.h \
           src/util_gui/AddNewDocumentDialogController.h \
           src/util_gui/AddNewDocumentDialogImpl.h \
           src/util_gui/AnnotationSettingsDialogController.h \
           src/util_gui/AnnotationSettingsDialogImpl.h \
           src/util_gui/BaseDocumentFormatConfigurators.h \
           src/util_gui/CopyDocumentDialogController.h \
           src/util_gui/CreateAnnotationDialog.h \
           src/util_gui/CreateAnnotationWidgetController.h \
           src/util_gui/CreateDocumentFromTextDialogController.h \
           src/util_gui/CreateObjectRelationDialogController.h \
           src/util_gui/DocumentFormatComboboxController.h \
           src/util_gui/DownloadRemoteFileDialog.h \
           src/util_gui/EditQualifierDialog.h \
           src/util_gui/EditSequenceDialogController.h \
           src/util_gui/ExportImageDialog.h \
           src/util_gui/GObjectComboBoxController.h \
           src/util_gui/GUIUtils.h \
           src/util_gui/ObjectViewTreeController.h \
           src/util_gui/ProjectDocumentComboBoxController.h \
           src/util_gui/ProjectTreeController.h \
           src/util_gui/ProjectTreeItemSelectorDialog.h \
           src/util_gui/ProjectTreeItemSelectorDialogImpl.h \
           src/util_gui/RemovePartFromSequenceDialogController.h \
           src/util_gui/SaveDocumentGroupController.h \
           src/util_gui/SeqPasterWidgetController.h \
           src/util_gui/logview/LogView.h
FORMS += src/util_gui/ui/AddExistingDocumentDialog.ui \
         src/util_gui/ui/AddNewDocumentDialog.ui \
         src/util_gui/ui/AnnotationSettingsDialog.ui \
         src/util_gui/ui/CopyDocumentDialog.ui \
         src/util_gui/ui/CreateAnnotationWidget.ui \
         src/util_gui/ui/CreateDocumentFromTextDialog.ui \
         src/util_gui/ui/CreateObjectRelationDialog.ui \
         src/util_gui/ui/DownloadRemoteFileDialog.ui \
         src/util_gui/ui/EditQualifierDialog.ui \
         src/util_gui/ui/EditSequenceDialog.ui \
         src/util_gui/ui/ExportImageDialog.ui \
         src/util_gui/ui/FormatSettingsDialog.ui \
         src/util_gui/ui/MultipartDocFormatConfiguratorWidget.ui \
         src/util_gui/ui/ProjectTreeItemSelectorDialogBase.ui \
         src/util_gui/ui/RemovePartFromSequenceDialog.ui \
         src/util_gui/ui/SeqPasterWidget.ui
SOURCES += src/MainWindow.cpp \
           src/Notification.cpp \
           src/NotificationWidget.cpp \
           src/ObjectViewModel.cpp \
           src/ObjectViewTasks.cpp \
           src/OpenViewTask.cpp \
           src/ProjectParsing.cpp \
           src/UnloadDocumentTask.cpp \
           src/util_gui/AddExistingDocumentDialogImpl.cpp \
           src/util_gui/AddNewDocumentDialogImpl.cpp \
           src/util_gui/AnnotationSettingsDialogImpl.cpp \
           src/util_gui/BaseDocumentFormatConfigurators.cpp \
           src/util_gui/CopyDocumentDialogController.cpp \
           src/util_gui/CreateAnnotationDialog.cpp \
           src/util_gui/CreateAnnotationWidgetController.cpp \
           src/util_gui/CreateDocumentFromTextDialogController.cpp \
           src/util_gui/CreateObjectRelationDialogController.cpp \
           src/util_gui/DocumentFormatComboboxController.cpp \
           src/util_gui/DownloadRemoteFileDialog.cpp \
           src/util_gui/EditQualifierDialog.cpp \
           src/util_gui/EditSequenceDialogController.cpp \
           src/util_gui/ExportImageDialog.cpp \
           src/util_gui/GObjectComboBoxController.cpp \
           src/util_gui/GUIUtils.cpp \
           src/util_gui/ObjectViewTreeController.cpp \
           src/util_gui/ProjectDocumentComboBoxController.cpp \
           src/util_gui/ProjectTreeController.cpp \
           src/util_gui/ProjectTreeItemSelectorDialog.cpp \
           src/util_gui/RemovePartFromSequenceDialogController.cpp \
           src/util_gui/SaveDocumentGroupController.cpp \
           src/util_gui/SeqPasterWidgetController.cpp \
           src/util_gui/logview/LogView.cpp
RESOURCES += U2Gui.qrc
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
