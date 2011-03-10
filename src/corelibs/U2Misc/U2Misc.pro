include (U2Misc.pri)

# Input
HEADERS += src/AuthenticationDialog.h \
           src/DialogUtils.h \
           src/GlassView.h \
           src/GraphUtils.h \
           src/GScrollBar.h \
           src/HBar.h \
           src/PositionSelector.h \
           src/RangeSelector.h \
           src/ScaleBar.h \
           src/ScriptEditorDialog.h \
           src/TextEditorDialog.h \
           src/TreeWidgetUtils.h
FORMS += src/ui/AuthentificationDialog.ui \
         src/ui/ScriptEditorDialog.ui \
         src/ui/TextEditorDialog.ui
SOURCES += src/AuthenticationDialog.cpp \
           src/DialogUtils.cpp \
           src/GlassView.cpp \
           src/GraphUtils.cpp \
           src/GScrollBar.cpp \
           src/HBar.cpp \
           src/PositionSelector.cpp \
           src/RangeSelector.cpp \
           src/ScaleBar.cpp \
           src/ScriptEditorDialog.cpp \
           src/TextEditorDialog.cpp \
           src/TreeWidgetUtils.cpp

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
