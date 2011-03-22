include (genome_aligner.pri)

# Input
HEADERS += src/BuildSArraySettingsWidget.h \
           src/GenomeAlignerFindTask.h \
           src/GenomeAlignerIndex.h \
           src/GenomeAlignerIndexTask.h \
           src/GenomeAlignerIO.h \
           src/GenomeAlignerPlugin.h \
           src/GenomeAlignerSearchQuery.h \
           src/GenomeAlignerSettingsWidget.h \
           src/GenomeAlignerTask.h \
           src/GenomeAlignerWorker.h
FORMS += src/ui/GenomeAlignerSettings.ui \
         src/ui/BuildSuffixArraySettings.ui
SOURCES += src/BuildSArraySettingsWidget.cpp \
           src/GenomeAlignerFindTask.cpp \
           src/GenomeAlignerIndex.cpp \
           src/GenomeAlignerIndexTask.cpp \
           src/GenomeAlignerIO.cpp \
           src/GenomeAlignerPlugin.cpp \
           src/GenomeAlignerSearchQuery.cpp \
           src/GenomeAlignerSettingsWidget.cpp \
           src/GenomeAlignerTask.cpp \
           src/GenomeAlignerWorker.cpp
