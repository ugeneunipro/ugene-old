cmake_minimum_required(VERSION 3.4)
project(U2Formats)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.

find_package(Qt5 REQUIRED Core Gui Widgets)

add_definitions(-DBUILDING_U2FORMATS_DLL)

include_directories(src)
include_directories(../../include)
include_directories(../../libs_3rdparty/samtools/src)
include_directories(../../libs_3rdparty/samtools/src/samtools)

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS src/*.h)
qt5_add_resources(RCC_SRCS src/U2Formats.qrc)

add_library(U2Formats SHARED ${HDRS} ${SRCS})

target_link_libraries(U2Formats
        U2Core U2Algorithm
        samtools ugenedb
        Qt5::Core Qt5::Gui Qt5::Widgets)

