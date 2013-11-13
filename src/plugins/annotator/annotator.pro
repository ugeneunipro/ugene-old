include (annotator.pri)

# Input
HEADERS += src/AnnotatorPlugin.h \
           src/AnnotatorTests.h \
           src/CollocationsDialogController.h \
           src/CollocationsSearchAlgorithm.h \
           src/CollocationWorker.h\
           src/GeneByGeneReportWorker.h\
           src/GeneByGeneReportTask.h \
    src/CustomPatternAnnotationTask.h
FORMS += src/ui/FindAnnotationCollocationsDialog.ui
SOURCES += src/AnnotatorPlugin.cpp \
           src/AnnotatorTests.cpp \
           src/CollocationsDialogController.cpp \
           src/CollocationsSearchAlgorithm.cpp \
           src/CollocationWorker.cpp\
           src/GeneByGeneReportWorker.cpp\
           src/GeneByGeneReportTask.cpp \
    src/CustomPatternAnnotationTask.cpp
RESOURCES += annotator.qrc
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
