cmake_minimum_required(VERSION 3.4)
project(U2Test)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS -nn) # do not display notes.
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(Qt5 REQUIRED Core Gui Widgets Xml)

add_definitions(-DBUILDING_U2TEST_DLL)

include_directories(src)
include_directories(../../include)

file(GLOB SRCS src/*.cpp src/xmltest/*.cpp)
file(GLOB HDRS src/*.h src/xmltest/*.h)

add_library(U2Test SHARED ${HDRS} ${SRCS})

target_link_libraries(U2Test
        U2Core
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Xml)

