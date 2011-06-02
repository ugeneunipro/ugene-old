include (api_tests.pri)

# Input
HEADERS += \
    src/ApiTestsPlugin.h \
    src/ApiTest.h \
    src/core/dbi/DbiTest.h \
    src/core/dbi/assembly/AssemblyDbiTest.h
SOURCES += \
    src/ApiTestsPlugin.cpp \
    src/ApiTest.cpp \
    src/core/dbi/assembly/AssemblyDbiTest.cpp
