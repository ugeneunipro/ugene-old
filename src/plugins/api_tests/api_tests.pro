include (api_tests.pri)

# Input
HEADERS += \
    src/ApiTestsPlugin.h \
    src/unittest.h \
    src/core/datatype/msa/MAlignmentUnitTests.h \
    src/core/datatype/msa/MAlignmentRowUnitTests.h \
    src/core/dbi/DbiTest.h \
    src/core/dbi/assembly/AssemblyDbiTestUtil.h \
    src/core/dbi/assembly/AssemblyDbiUnitTests.h \
    src/core/dbi/sequence/SequenceDbiUnitTests.h \
    src/core/dbi/attribute/AttributeDbiUnitTests.h \
    src/core/dbi/features/FeaturesTableObjectUnitTest.h \
    src/core/dbi/msa/MsaDbiUnitTests.h \
    src/core/format/fastq/FastqUnitTests.h \
    src/core/format/genbank/LocationParserUnitTests.h \
    src/core/format/sqlite_msa_dbi/MsaDbiSQLiteSpecificUnitTests.h \
    src/core/format/sqlite_object_dbi/SQLiteObjectDbiUnitTests.h \
    src/core/gobjects/MAlignmentObjectUnitTests.h \
    src/core/util/MAlignmentImporterExporterUnitTests.h \
    src/UnitTestSuite.h \  
    src/core/util/MsaDbiUtilsUnitTests.h \
    src/core/format/sqlite_mod_dbi/ModDbiSQLiteSpecificUnitTests.h
SOURCES += \
    src/ApiTestsPlugin.cpp \
    src/core/datatype/msa/MAlignmentUnitTests.cpp \
    src/core/datatype/msa/MAlignmentRowUnitTests.cpp \
    src/core/dbi/DbiTest.cpp \
    src/core/dbi/assembly/AssemblyDbiTestUtil.cpp \
    src/core/dbi/assembly/AssemblyDbiUnitTests.cpp \
    src/core/dbi/sequence/SequenceDbiUnitTests.cpp \
    src/core/dbi/attribute/AttributeDbiUnitTests.cpp \
    src/core/dbi/features/FeaturesTableObjectUnitTest.cpp \
    src/core/dbi/msa/MsaDbiUnitTests.cpp \
    src/core/format/fastq/FastqUnitTests.cpp \
    src/core/format/genbank/LocationParserUnitTests.cpp \
    src/core/format/sqlite_msa_dbi/MsaDbiSQLiteSpecificUnitTests.cpp \
    src/core/format/sqlite_object_dbi/SQLiteObjectDbiUnitTests.cpp \
    src/core/gobjects/MAlignmentObjectUnitTests.cpp \
    src/core/util/MAlignmentImporterExporterUnitTests.cpp \
    src/UnitTestSuite.cpp \  
    src/core/util/MsaDbiUtilsUnitTests.cpp \
    src/core/format/sqlite_mod_dbi/ModDbiSQLiteSpecificUnitTests.cpp






