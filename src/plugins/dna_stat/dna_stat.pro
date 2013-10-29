include (dna_stat.pri)

# Input
HEADERS += src/DistanceMatrixMSAProfileDialog.h \
           src/DNAStatMSAProfileDialog.h \
           src/DNAStatPlugin.h \
           src/DNAStatProfileTask.h \
           src/DNAStatsWindow.h
FORMS += src/ui/DistanceMatrixMSAProfileDialog.ui \
         src/ui/DNAStatMSAProfileDialog.ui
SOURCES += src/DistanceMatrixMSAProfileDialog.cpp \
           src/DNAStatMSAProfileDialog.cpp \
           src/DNAStatPlugin.cpp \
           src/DNAStatProfileTask.cpp \
           src/DNAStatsWindow.cpp
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
