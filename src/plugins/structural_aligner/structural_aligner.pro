include (structural_aligner.pri)

# Input
HEADERS +=  src/StructuralAlignerPlugin.h \
            src/StructuralAlignerPtools.h \
            # ptools includes
            src/ptools/atom.h \
            src/ptools/atomselection.h \
            src/ptools/basetypes.h \
            src/ptools/coord3d.h \
            src/ptools/coordsarray.h \
            src/ptools/derivify.h \
            src/ptools/geometry.h \
            src/ptools/pdbio.h \
            src/ptools/rigidbody.h \
            src/ptools/rmsd.h \
            src/ptools/screw.h \
            src/ptools/superpose.h

SOURCES +=  src/StructuralAlignerPlugin.cpp \
            src/StructuralAlignerPtools.cpp \
            # ptools sources
            src/ptools/atom.cpp \
            src/ptools/atomselection.cpp \
            src/ptools/basetypes.cpp \
            src/ptools/coord3d.cpp \
            src/ptools/coordsarray.cpp \
            src/ptools/geometry.cpp \
            src/ptools/pdbio.cpp \
            src/ptools/rigidbody.cpp \
            src/ptools/rmsd.cpp \
            src/ptools/superpose.cpp
