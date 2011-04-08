include (dbi_sqlite.pri)

# Input
HEADERS += src/SQLiteAssemblyDbi.h \
           src/SQLiteDbi.h \
           src/SQLiteDbiPlugin.h \
           src/SQLiteMsaDbi.h \
           src/SQLiteObjectDbi.h \
           src/SQLiteSequenceDbi.h \
           src/assembly/AssemblyPackAlgorithm.h \
           src/assembly/MultiTableAssemblyAdapter.h \
           src/assembly/RTreeAssemblyAdapter.h \
           src/assembly/SingleTableAssemblyAdapter.h
SOURCES += src/SQLiteAssemblyDbi.cpp \
           src/SQLiteDbi.cpp \
           src/SQLiteDbiPlugin.cpp \
           src/SQLiteMsaDbi.cpp \
           src/SQLiteObjectDbi.cpp \
           src/SQLiteSequenceDbi.cpp \
           src/assembly/AssemblyPackAlgorithm.cpp \
           src/assembly/MultiTableAssemblyAdapter.cpp \
           src/assembly/RTreeAssemblyAdapter.cpp \
           src/assembly/SingleTableAssemblyAdapter.cpp
