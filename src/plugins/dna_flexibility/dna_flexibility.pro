include (dna_flexibility.pri)

# Input
HEADERS += src/DNAFlexDialog.h \
           src/DNAFlexPlugin.h \
           src/DNAFlexTask.h \
           src/FindHighFlexRegions.h \
           src/FindHighFlexRegionsAlgorithm.h \
           src/HighFlexSettings.h \
           src/HighFlexResult.h

FORMS += src/ui/DNAFlexDialog.ui

SOURCES += src/DNAFlexDialog.cpp \
           src/DNAFlexPlugin.cpp \
           src/DNAFlexTask.cpp \
           src/FindHighFlexRegions.cpp \
           src/FindHighFlexRegionsAlgorithm.cpp \
           src/HighFlexSettings.cpp

RESOURCES += dna_flexibility.qrc

TRANSLATIONS += transl/english.ts transl/russian.ts
