include (U2Designer.pri)

# Input
HEADERS += src/DatasetsController.h \
           src/DatasetsListWidget.h \
           src/DatasetWidget.h \
           src/DelegateEditors.h \
           src/DesignerUtils.h \
           src/DirectoryItem.h \
           src/EditMarkerGroupDialog.h \
           src/FileItem.h \
           src/GrouperEditor.h \
           src/GrouperEditorWidget.h \
           src/MarkerEditor.h \
           src/MarkerEditorWidget.h \
           src/NewGrouperSlotDialog.h \
           src/PropertyWidget.h \
           src/QDScheduler.h \
           src/UrlItem.h \
           src/WorkflowGUIUtils.h \
           _tmp/ui/ui_DatasetsWidget.h \
           _tmp/ui/ui_DirectoryItem.h \
           _tmp/ui/ui_FileItem.h \
           _tmp/ui/ui_InputDataWidget.h \
           _tmp/ui/ui_ItemBox.h \
           src/support/URLLineEdit.h \
           src/wizard/PropertyWizardController.h \
           src/wizard/WizardController.h
FORMS += src/ui/AnnsActionDialog.ui \
         src/ui/DatasetsListWidget.ui \
         src/ui/DatasetWidget.ui \
         src/ui/DirectoryOptions.ui \
         src/ui/EditFloatMarkerWidget.ui \
         src/ui/EditIntegerMarkerWidget.ui \
         src/ui/EditMarkerDialog.ui \
         src/ui/EditMarkerGroupDialog.ui \
         src/ui/EditStringMarkerWidget.ui \
         src/ui/GrouperEditorWidget.ui \
         src/ui/MarkerEditorWidget.ui \
         src/ui/MsaActionDialog.ui \
         src/ui/NewGrouperSlotDialog.ui \
         src/ui/SequenceActionDialog.ui \
         src/ui/StringActionDialog.ui
SOURCES += src/DatasetsController.cpp \
           src/DatasetsListWidget.cpp \
           src/DatasetWidget.cpp \
           src/DelegateEditors.cpp \
           src/DesignerUtils.cpp \
           src/DirectoryItem.cpp \
           src/EditMarkerGroupDialog.cpp \
           src/FileItem.cpp \
           src/GrouperEditor.cpp \
           src/GrouperEditorWidget.cpp \
           src/MarkerEditor.cpp \
           src/MarkerEditorWidget.cpp \
           src/NewGrouperSlotDialog.cpp \
           src/PropertyWidget.cpp \
           src/QDScheduler.cpp \
           src/UrlItem.cpp \
           src/WorkflowGUIUtils.cpp \
           src/support/URLLineEdit.cpp \
           src/wizard/PropertyWizardController.cpp \
           src/wizard/WizardController.cpp
RESOURCES += U2Designer.qrc
TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
