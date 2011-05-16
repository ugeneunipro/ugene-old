include (dbi_bam.pri)

# Input
HEADERS += \
    src/BAMDbiPlugin.h \
    src/BAMFormat.h \
    src/CigarValidator.h \
    src/Exception.h \
    src/IOException.h \
    src/InvalidFormatException.h \
    src/Header.h \
    src/Alignment.h \
    src/VirtualOffset.h \
    src/BgzfReader.h \
    src/BgzfWriter.h \
    src/Reader.h \
    src/Writer.h \
    src/Index.h \
    src/BaiReader.h \
    src/BaiWriter.h \
    src/Dbi.h \
    src/ConvertToSQLiteTask.h \
    src/ConvertToSQLiteDialog.h \
    src/LoadBamInfoTask.h
SOURCES += \
    src/BAMDbiPlugin.cpp \
    src/BAMFormat.cpp \
    src/CigarValidator.cpp \
    src/Exception.cpp \
    src/IOException.cpp \
    src/InvalidFormatException.cpp \
    src/Header.cpp \
    src/Alignment.cpp \
    src/VirtualOffset.cpp \
    src/BgzfReader.cpp \
    src/BgzfWriter.cpp \
    src/Reader.cpp \
    src/Writer.cpp \
    src/Index.cpp \
    src/BaiReader.cpp \
    src/BaiWriter.cpp \
    src/Dbi.cpp \
    src/ConvertToSQLiteTask.cpp \
    src/ConvertToSQLiteDialog.cpp \
    src/LoadBamInfoTask.cpp
FORMS += \
    src/ConvertToSQLiteDialog.ui

TRANSLATIONS += transl/czech.ts transl/english.ts transl/russian.ts