include (pcr.pri)

HEADERS += src/AddPrimerDialog.h \
           src/ExtractProductTask.h \
           src/InSilicoPcrOPWidgetFactory.h \
           src/InSilicoPcrOptionPanelWidget.h \
           src/InSilicoPcrProductsTable.h \
           src/InSilicoPcrTask.h \
           src/PcrPlugin.h \
           src/Primer.h \
           src/PrimerGroupBox.h \
           src/PrimerLibrary.h \
           src/PrimerLibraryMdiWindow.h \
           src/PrimerLibraryWidget.h \
           src/PrimerStatistics.h

FORMS += src/ui/AddPrimerDialog.ui \
         src/ui/InSilicoPcrOptionPanelWidget.ui \
         src/ui/PrimerGroupBox.ui \
         src/ui/PrimerLibraryWidget.ui

SOURCES += src/AddPrimerDialog.cpp \
           src/ExtractProductTask.cpp \
           src/InSilicoPcrOPWidgetFactory.cpp \
           src/InSilicoPcrOptionPanelWidget.cpp \
           src/InSilicoPcrProductsTable.cpp \
           src/InSilicoPcrTask.cpp \
           src/PcrPlugin.cpp \
           src/Primer.cpp \
           src/PrimerGroupBox.cpp \
           src/PrimerLibrary.cpp \
           src/PrimerLibraryMdiWindow.cpp \
           src/PrimerLibraryWidget.cpp \
           src/PrimerStatistics.cpp

RESOURCES += pcr.qrc

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
