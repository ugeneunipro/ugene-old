include (snp_effect.pri)

# Input
HEADERS += src/SNPEffectPlugin.h \
           src/SNPReportWriter.h \
           src/snp_toolbox/SNPToolboxWorker.h \
           src/snp_toolbox/DamageEffectEvaluator.h \
           src/snp_toolbox/SNPToolboxTask.h
           
SOURCES += src/SNPEffectPlugin.cpp \
           src/SNPReportWriter.cpp \
           src/snp_toolbox/SNPToolboxWorker.cpp \
           src/snp_toolbox/DamageEffectEvaluator.cpp \
           src/snp_toolbox/SNPToolboxTask.cpp
           
