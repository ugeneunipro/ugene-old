include (U2Designer.pri)

# Input
HEADERS += src/DelegateEditors.h \
           src/DesignerUtils.h \
           src/QDScheduler.h \
           src/WorkflowGUIUtils.h
SOURCES += src/DelegateEditors.cpp \
           src/DesignerUtils.cpp \
           src/QDScheduler.cpp \
           src/WorkflowGUIUtils.cpp

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
