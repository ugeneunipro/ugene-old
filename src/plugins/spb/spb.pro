include (spb.pri)

# Input
HEADERS += src/ComparingAlgorithm.h \
           src/DereplicateSequencesWorker.h \
           src/DereplicationTask.h \
           src/DistanceReportWorker.h \
           src/FilterSequencesWorker.h \
           src/FullIndexComparer.h \
           src/RandomDereplicationTask.h \
           src/SpbPlugin.h \
    src/RandomFilterWorker.h
SOURCES += src/ComparingAlgorithm.cpp \
           src/DereplicateSequencesWorker.cpp \
           src/DereplicationTask.cpp \
           src/DistanceReportWorker.cpp \
           src/FilterSequencesWorker.cpp \
           src/FullIndexComparer.cpp \
           src/RandomDereplicationTask.cpp \
           src/SpbPlugin.cpp \
    src/RandomFilterWorker.cpp
