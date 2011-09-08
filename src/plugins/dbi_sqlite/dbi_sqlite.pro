include (dbi_sqlite.pri)

# Input
HEADERS += src/SQLiteAnnotationDbi.h \
           src/SQLiteAssemblyDbi.h \
           src/SQLiteAttributeDbi.h \
           src/SQLiteDbi.h \
           src/SQLiteDbiPlugin.h \
           src/SQLiteFeatureDbi.h \
           src/SQLiteMsaDbi.h \
           src/SQLiteObjectDbi.h \
           src/SQLiteSequenceDbi.h \
           src/SQLiteVariantDbi.h \
           src/assembly/AssemblyPackAlgorithm.h \
           src/assembly/MultiTableAssemblyAdapter.h \
           src/assembly/RTreeAssemblyAdapter.h \
           src/assembly/SingleTableAssemblyAdapter.h
SOURCES += src/SQLiteAnnotationDbi.cpp \
           src/SQLiteAssemblyDbi.cpp \
           src/SQLiteAttributeDbi.cpp \
           src/SQLiteDbi.cpp \
           src/SQLiteDbiPlugin.cpp \
           src/SQLiteFeatureDbi.cpp \
           src/SQLiteMsaDbi.cpp \
           src/SQLiteObjectDbi.cpp \
           src/SQLiteSequenceDbi.cpp \
           src/SQLiteVariantDbi.cpp \
           src/assembly/AssemblyPackAlgorithm.cpp \
           src/assembly/MultiTableAssemblyAdapter.cpp \
           src/assembly/RTreeAssemblyAdapter.cpp \
           src/assembly/SingleTableAssemblyAdapter.cpp
