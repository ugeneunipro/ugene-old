include (dna_graphpack.pri)

# Input
HEADERS += src/BaseContentGraph.h src/DNAGraphPackPlugin.h src/DeviationGraph.h src/EntropyAlgorithm.h src/KarlinSignatureDifferenceGraph.h src/CumulativeSkew.h
SOURCES += src/BaseContentGraph.cpp src/DNAGraphPackPlugin.cpp src/DeviationGraph.cpp src/EntropyAlgorithm.cpp src/KarlinSignatureDifferenceGraph.cpp src/CumulativeSkew.cpp
RESOURCES += dna_graphpack.qrc
TRANSLATIONS += transl/english.ts transl/russian.ts
