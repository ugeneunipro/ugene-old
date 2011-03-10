include (enzymes.pri)

# Input
HEADERS += src/CloningUtilTasks.h \
           src/ConstructMoleculeDialog.h \
           src/CreateFragmentDialog.h \
           src/DigestSequenceDialog.h \
           src/DNAFragment.h \
           src/EditFragmentDialog.h \
           src/EnzymeModel.h \
           src/EnzymesIO.h \
           src/EnzymesPlugin.h \
           src/EnzymesQuery.h \
           src/EnzymesTests.h \
           src/FindEnzymesAlgorithm.h \
           src/FindEnzymesDialog.h \
           src/FindEnzymesTask.h
FORMS += src/ui/ConstructMoleculeDialog.ui \
         src/ui/CreateFragmentDialog.ui \
         src/ui/DigestSequenceDialog.ui \
         src/ui/EditFragmentDialog.ui \
         src/ui/EnzymesSelectorDialog.ui \
         src/ui/EnzymesSelectorWidget.ui \
         src/ui/FindEnzymesDialog.ui
SOURCES += src/CloningUtilTasks.cpp \
           src/ConstructMoleculeDialog.cpp \
           src/CreateFragmentDialog.cpp \
           src/DigestSequenceDialog.cpp \
           src/DNAFragment.cpp \
           src/EditFragmentDialog.cpp \
           src/EnzymesIO.cpp \
           src/EnzymesPlugin.cpp \
           src/EnzymesQuery.cpp \
           src/EnzymesTests.cpp \
           src/FindEnzymesDialog.cpp \
           src/FindEnzymesTask.cpp
RESOURCES += enzymes.qrc
TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts
