include (phylip.pri)

# Input
HEADERS += src/dist.h \
           src/DistanceMatrix.h \
           src/DistMatrixModelWidget.h \
           src/dnadist.h \
           src/neighbor.h \
           src/NeighborJoinAdapter.h \
           src/phylip.h \
           src/PhylipPlugin.h \
           src/PhylipPluginTests.h \
           src/protdist.h \
           src/seq.h \
	   src/seqboot.h \
	   src/SeqBootAdapter.h \
	   src/SeqBootModelWidget.h \
	   src/cons.h
FORMS += src/ui/DistMatrixModel.ui \
         src/ui/SeqBootModel.ui
SOURCES += src/dist.cpp \
           src/DistanceMatrix.cpp \
           src/DistMatrixModelWidget.cpp \
           src/dnadist.cpp \
           src/neighbor.cpp \
           src/NeighborJoinAdapter.cpp \
           src/phylip.cpp \
           src/PhylipPlugin.cpp \
           src/PhylipPluginTests.cpp \
           src/protdist.cpp \
           src/seq.cpp \
	   src/seqboot.cpp \
	   src/SeqBootAdapter.cpp \
	   src/SeqBootModelWidget.cpp \
	   src/cons.cpp 
TRANSLATIONS += transl/english.ts transl/russian.ts
