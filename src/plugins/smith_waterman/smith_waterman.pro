include (smith_waterman.pri)

# Input
HEADERS += src/PairAlignSequences.h \
           src/SmithWatermanAlgorithm.h \
           src/SmithWatermanAlgorithmSSE2.h \
           src/SWAlgorithmPlugin.h \
           src/SWAlgorithmTask.h \
           src/SmithWatermanAlgorithmCUDA.h \
           src/SmithWatermanAlgorithmOPENCL.h \
           src/sw_cuda_cpp.h \
           src/SWTaskFactory.h \
           src/SmithWatermanTests.h \
           src/SWQuery.h

SOURCES += src/PairAlignSequences.cpp \
           src/SmithWatermanAlgorithm.cpp \
           src/SmithWatermanAlgorithmSSE2.cpp \
           src/SWAlgorithmPlugin.cpp \
           src/SWAlgorithmTask.cpp \
           src/SmithWatermanAlgorithmCUDA.cpp \
           src/SmithWatermanAlgorithmOPENCL.cpp \
           src/sw_cuda_cpp.cpp \
           src/SWTaskFactory.cpp \
           src/SmithWatermanTests.cpp \
           src/SWQuery.cpp

RESOURCES += smith_waterman.qrc

TRANSLATIONS += transl/english.ts transl/russian.ts
