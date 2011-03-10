include (qscore.pri)

# Input
HEADERS += src/QScoreAdapter.h \
           src/qscore/msa.h \
           src/qscore/qscore.h \
           src/qscore/qscore_context.h \
           src/qscore/sab_ids.h \
           src/qscore/sab_ref2.h \
           src/qscore/seq.h
SOURCES += src/QScoreAdapter.cpp \
           src/qscore/qscore_clineshift.cpp \
           src/qscore/qscore_comparemap.cpp \
           src/qscore/qscore_comparemsa.cpp \
           src/qscore/qscore_comparepair.cpp \
           src/qscore/qscore_context.cpp \
           src/qscore/qscore_fasta.cpp \
           src/qscore/qscore_fastq.cpp \
           src/qscore/qscore_gapscore.cpp \
           src/qscore/qscore_gapscore2.cpp \
           src/qscore/qscore_msa_qscore.cpp \
           src/qscore/qscore_options.cpp \
           src/qscore/qscore_perseq.cpp \
           src/qscore/qscore_qscore.cpp \
           src/qscore/qscore_sab.cpp \
           src/qscore/qscore_seq.cpp \
           src/qscore/qscore_sumpairs.cpp \
           src/qscore/qscore_tc.cpp \
           src/qscore/qscore_utils.cpp
