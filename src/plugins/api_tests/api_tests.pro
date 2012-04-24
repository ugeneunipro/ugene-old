include (api_tests.pri)

# Input
HEADERS += \
    src/ApiTestsPlugin.h \
    src/unittest.h \
    src/core/dbi/DbiTest.h \
    src/core/dbi/assembly/AssemblyDbiTestUtil.h \
    src/core/dbi/assembly/AssemblyDbiUnitTests.h \
    src/core/dbi/sequence/SequenceDbiUnitTests.h \
    src/core/dbi/attribute/AttributeDbiUnitTests.h \
    src/core/dbi/features/FeaturesTableObjectUnitTest.h \
    src/core/format/fastq/FastqUnitTests.h \
    src/core/format/genbank/LocationParserUnitTests.h \
    src/UnitTestSuite.h 
SOURCES += \
    src/ApiTestsPlugin.cpp \
    src/core/dbi/DbiTest.cpp \
    src/core/dbi/assembly/AssemblyDbiTestUtil.cpp \
    src/core/dbi/assembly/AssemblyDbiUnitTests.cpp \
    src/core/dbi/sequence/SequenceDbiUnitTests.cpp \
    src/core/dbi/attribute/AttributeDbiUnitTests.cpp \
    src/core/dbi/features/FeaturesTableObjectUnitTest.cpp \
    src/core/format/fastq/FastqUnitTests.cpp \
    src/core/format/genbank/LocationParserUnitTests.cpp \
    src/UnitTestSuite.cpp 
