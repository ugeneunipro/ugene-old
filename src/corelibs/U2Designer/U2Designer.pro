include (U2Designer.pri)

# Input
HEADERS += src/DelegateEditors.h \
           src/DesignerUtils.h \
           src/EditMarkerGroupDialog.h \
           src/MarkerEditor.h \
           src/MarkerEditorWidget.h \
           src/QDScheduler.h \
           src/WorkflowGUIUtils.h
FORMS += src/ui/EditFloatMarkerWidget.ui \
         src/ui/EditMarkerGroupDialog.ui \
         src/ui/EditIntegerMarkerWidget.ui \
         src/ui/EditMarkerDialog.ui \
         src/ui/EditStringMarkerWidget.ui \
         src/ui/MarkerEditorWidget.ui
SOURCES += src/DelegateEditors.cpp \
           src/DesignerUtils.cpp \
           src/EditMarkerGroupDialog.cpp \
           src/MarkerEditor.cpp \
           src/MarkerEditorWidget.cpp \
           src/QDScheduler.cpp \
           src/WorkflowGUIUtils.cpp

TRANSLATIONS += transl/chinese.ts \
                transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
