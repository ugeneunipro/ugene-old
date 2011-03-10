include (dbi_sqlite.pri)

# Input
HEADERS += src/SQLiteAssemblyDbi.h \
           src/SQLiteDbi.h \
           src/SQLiteDbiPlugin.h \
           src/SQLiteMsaDbi.h \
           src/SQLiteObjectDbi.h \
           src/SQLiteSequenceDbi.h
SOURCES += src/SQLiteAssemblyDbi.cpp \
           src/SQLiteDbi.cpp \
           src/SQLiteDbiPlugin.cpp \
           src/SQLiteMsaDbi.cpp \
           src/SQLiteObjectDbi.cpp \
           src/SQLiteSequenceDbi.cpp
