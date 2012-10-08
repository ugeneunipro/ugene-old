include (U2Designer.pri)

# Input
HEADERS += src/DatasetsListWidget.h \
           src/DatasetsController.h \
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
           src/QDScheduler.h \
           src/UrlItem.h \
           src/WorkflowGUIUtils.h
FORMS += src/ui/AnnsActionDialog.ui \
         src/ui/DatasetsListWidget.ui \
         src/ui/DatasetWidget.ui \
         src/ui/DirectoryOptions.ui \
         src/ui/EditFloatMarkerWidget.ui \
         src/ui/EditMarkerGroupDialog.ui \
         src/ui/EditIntegerMarkerWidget.ui \
         src/ui/EditMarkerDialog.ui \
         src/ui/EditStringMarkerWidget.ui \
         src/ui/GrouperEditorWidget.ui \
         src/ui/MarkerEditorWidget.ui \
         src/ui/MsaActionDialog.ui \
         src/ui/NewGrouperSlotDialog.ui \
         src/ui/SequenceActionDialog.ui \
         src/ui/StringActionDialog.ui
SOURCES += src/DatasetsListWidget.cpp \
           src/DatasetsController.cpp \
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
           src/QDScheduler.cpp \
           src/UrlItem.cpp \
           src/WorkflowGUIUtils.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts

RESOURCES += U2Designer.qrc
