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
           src/PrimerLibrarySelector.h \
           src/PrimerLibraryTable.h \
           src/PrimerLibraryWidget.h \
           src/PrimerLineEdit.h \
           src/PrimerStatistics.h \
           src/PrimersDetailsDialog.h

FORMS += src/ui/AddPrimerDialog.ui \
         src/ui/InSilicoPcrOptionPanelWidget.ui \
         src/ui/PrimerGroupBox.ui \
         src/ui/PrimerLibrarySelector.ui \
         src/ui/PrimerLibraryWidget.ui \
         src/ui/PrimersDetailsDialog.ui

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
           src/PrimerLibrarySelector.cpp \
           src/PrimerLibraryTable.cpp \
           src/PrimerLibraryWidget.cpp \
           src/PrimerLineEdit.cpp \
           src/PrimerStatistics.cpp \
           src/PrimersDetailsDialog.cpp

RESOURCES += pcr.qrc

TRANSLATIONS += transl/czech.ts \
                transl/english.ts \
                transl/russian.ts
