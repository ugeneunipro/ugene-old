include (api_tests.pri)

# Input
HEADERS += \
    src/ApiTestsPlugin.h \
    src/ApiTest.h \
    src/core/dbi/DbiTest.h \
    src/core/dbi/assembly/AssemblyDbiTest.h \
    src/core/dbi/assembly/AssemblyDbiTestUtil.h \
    src/core/dbi/sequence/SequenceDbiTest.h \
    src/core/dbi/attribute/AttributeDbiTest.h \
    src/core/dbi/ConvertToDbTask.h
SOURCES += \
    src/ApiTestsPlugin.cpp \
    src/core/dbi/DbiTest.cpp \
    src/ApiTest.cpp \
    src/core/dbi/assembly/AssemblyDbiTest.cpp \
    src/core/dbi/assembly/AssemblyDbiTestUtil.cpp \
    src/core/dbi/sequence/SequenceDbiTest.cpp \
    src/core/dbi/attribute/AttributeDbiTest.cpp \
    src/core/dbi/ConvertToDbTask.cpp
