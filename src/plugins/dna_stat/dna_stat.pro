include (dna_stat.pri)

# Input
HEADERS +=      src/DNAStatPlugin.h \
                src/DNAStatMSAProfileDialog.h \
                src/DistanceMatrixMSAProfileDialog.h 
FORMS +=        src/ui/DNAStatMSAProfileDialog.ui \
                src/ui/DistanceMatrixMSAProfileDialog.ui 
SOURCES +=      src/DNAStatPlugin.cpp \
                src/DNAStatMSAProfileDialog.cpp \
                src/DistanceMatrixMSAProfileDialog.cpp 
TRANSLATIONS += transl/english.ts \
                transl/russian.ts
