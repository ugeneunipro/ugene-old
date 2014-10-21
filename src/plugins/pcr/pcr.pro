include (pcr.pri)

HEADERS += src/AddPrimerDialog.h \
           src/PcrPlugin.h \
           src/Primer.h \
           src/PrimerLibrary.h \
           src/PrimerLibraryMdiWindow.h \
           src/PrimerLibraryWidget.h

FORMS += src/ui/AddPrimerDialog.ui \
         src/ui/PrimerLibraryWidget.ui

SOURCES += src/AddPrimerDialog.cpp \
           src/PcrPlugin.cpp \
           src/Primer.cpp \
           src/PrimerLibrary.cpp \
           src/PrimerLibraryMdiWindow.cpp \
           src/PrimerLibraryWidget.cpp

RESOURCES += pcr.qrc

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
