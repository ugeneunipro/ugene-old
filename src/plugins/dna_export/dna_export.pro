include (dna_export.pri) 

# Input
HEADERS += src/CSVColumnConfiguration.h \
           src/CSVColumnConfigurationDialog.h \
           src/DNAExportPlugin.h \
           src/DNAExportPluginTests.h \
           src/DNASequenceGenerator.h \
           src/DNASequenceGeneratorDialog.h \
           src/ExportAlignmentViewItems.h \
           src/ExportAnnotations2CSVTask.h \
           src/ExportAnnotationsDialog.h \
           src/ExportChromatogramDialog.h \
           src/ExportMSA2MSADialog.h \
           src/ExportMSA2SequencesDialog.h \
           src/ExportProjectViewItems.h \
           src/ExportQualityScoresTask.h \
           src/ExportSequences2MSADialog.h \
           src/ExportSequencesDialog.h \
           src/ExportSequenceTask.h \
           src/ExportSequenceViewItems.h \
           src/ExportTasks.h \
           src/ExportUtils.h \
           src/GenerateDNAWorker.h \
           src/GetSequenceByIdDialog.h \
           src/ImportAnnotationsFromCSVDialog.h \
           src/ImportAnnotationsFromCSVTask.h \
           src/ImportQualityScoresTask.h \
           src/ImportQualityScoresWorker.h \
           src/WriteAnnotationsWorker.h \
    src/ExportQualityScoresWorker.h
FORMS += src/ui/BaseContentDialog.ui \
         src/ui/CSVColumnConfigurationDialog.ui \
         src/ui/DNASequenceGeneratorDialog.ui \
         src/ui/ExportAnnotationsDialog.ui \
         src/ui/ExportChromatogramDialog.ui \
         src/ui/ExportMSA2MSADialog.ui \
         src/ui/ExportMSA2SequencesDialog.ui \
         src/ui/ExportSequences2MSADialog.ui \
         src/ui/ExportSequencesDialog.ui \
         src/ui/GetSequenceByIdDialog.ui \
         src/ui/ImportAnnotationsFromCSVDialog.ui
SOURCES += src/CSVColumnConfigurationDialog.cpp \
           src/DNAExportPlugin.cpp \
           src/DNAExportPluginTests.cpp \
           src/DNASequenceGenerator.cpp \
           src/DNASequenceGeneratorDialog.cpp \
           src/ExportAlignmentViewItems.cpp \
           src/ExportAnnotations2CSVTask.cpp \
           src/ExportAnnotationsDialog.cpp \
           src/ExportChromatogramDialog.cpp \
           src/ExportMSA2MSADialog.cpp \
           src/ExportMSA2SequencesDialog.cpp \
           src/ExportProjectViewItems.cpp \
           src/ExportQualityScoresTask.cpp \
           src/ExportSequences2MSADialog.cpp \
           src/ExportSequencesDialog.cpp \
           src/ExportSequenceTask.cpp \
           src/ExportSequenceViewItems.cpp \
           src/ExportTasks.cpp \
           src/ExportUtils.cpp \
           src/GenerateDNAWorker.cpp \
           src/GetSequenceByIdDialog.cpp \
           src/ImportAnnotationsFromCSVDialog.cpp \
           src/ImportAnnotationsFromCSVTask.cpp \
           src/ImportQualityScoresTask.cpp \
           src/ImportQualityScoresWorker.cpp \
           src/WriteAnnotationsWorker.cpp \
    src/ExportQualityScoresWorker.cpp
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
