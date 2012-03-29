include (U2Designer.pri)

# Input
HEADERS += src/DelegateEditors.h \
           src/DesignerUtils.h \
           src/EditMarkerGroupDialog.h \
           src/GrouperEditor.h \
           src/GrouperEditorWidget.h \
           src/MarkerEditor.h \
           src/MarkerEditorWidget.h \
           src/NewGrouperSlotDialog.h \
           src/QDScheduler.h \
           src/WorkflowGUIUtils.h
FORMS += src/ui/AnnsActionDialog.ui \
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
SOURCES += src/DelegateEditors.cpp \
           src/DesignerUtils.cpp \
           src/EditMarkerGroupDialog.cpp \
           src/GrouperEditor.cpp \
           src/GrouperEditorWidget.cpp \
           src/MarkerEditor.cpp \
           src/MarkerEditorWidget.cpp \
           src/NewGrouperSlotDialog.cpp \
           src/QDScheduler.cpp \
           src/WorkflowGUIUtils.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
