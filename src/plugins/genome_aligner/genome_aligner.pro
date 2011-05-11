include (genome_aligner.pri)

# Input
HEADERS += src/BuildSArraySettingsWidget.h \
           src/GenomeAlignerCMDLineTask.h \
           src/GenomeAlignerFindTask.h \
           src/GenomeAlignerIndex.h \
           src/GenomeAlignerIndexPart.h \
           src/GenomeAlignerIndexTask.h \
           src/GenomeAlignerIO.h \
           src/GenomeAlignerPlugin.h \
           src/GenomeAlignerSearchQuery.h \
           src/GenomeAlignerSettingsWidget.h \
           src/GenomeAlignerTask.h \
           src/GenomeAlignerWorker.h \
           src/GenomeAlignerWriteTask.h \
           src/SuffixSearchCUDA.h
FORMS += src/ui/BuildSuffixArraySettings.ui src/ui/GenomeAlignerSettings.ui
SOURCES += src/BuildSArraySettingsWidget.cpp \
           src/GenomeAlignerCMDLineTask.cpp \
           src/GenomeAlignerFindTask.cpp \
           src/GenomeAlignerIndex.cpp \
           src/GenomeAlignerIndexPart.cpp \
           src/GenomeAlignerIndexTask.cpp \
           src/GenomeAlignerIO.cpp \
           src/GenomeAlignerPlugin.cpp \
           src/GenomeAlignerSearchQuery.cpp \
           src/GenomeAlignerSettingsWidget.cpp \
           src/GenomeAlignerTask.cpp \
           src/GenomeAlignerWorker.cpp \
           src/GenomeAlignerWriteTask.cpp \
           src/SuffixSearchCUDA.cpp
TRANSLATIONS += transl/english.ts transl/russian.ts
